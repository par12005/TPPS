File Manipulation
=================

File Manipulation in TPPS is a complex topic, because TPPS needs to provide file uploads that can be parsed easily, but also need to be very flexible. TPPS achieves this by allowing users to define the data types of each of the columns in a file upload. For example, for the plant accession file, users need to define columns that describe the plant identifier and location of that plant identifier. Creating the file upload field as well as the fields to define column types and show a preview of the data inside the file requires a lot of code, so some parts of it are found in the normal form build files, some parts are found in hooks in the tpps.module file, and some parts of it are found in helper functions in the tpps.module file. Here we will attempt to cover all aspects of the file manipulation process.

For reference, we will be using the Plant Accession file field as our example through the entire process.

Define Drupal Form Element
--------------------------

The first part of creating a dynamic TPPS file upload field is specifying it in the ``$form`` array just as you would for any other Drupal Form API field::

    $form['tree-accession']['file'] = array(
      '#type' => 'managed_file',
      '#title' => t("Plant Accession File: *<br>$file_description"),
      '#upload_location' => "$file_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('txt csv xlsx'),
      ),
      '#states' => ($species_number > 1) ? (array(
        'visible' => array(
          ':input[name="tree-accession[check]"]' => array('checked' => FALSE),
        ),
      )) : NULL,
      '#field_prefix' => '<span style="width: 100%;display: block;text-align: right;padding-right: 2%;">Allowed file extensions: txt csv xlsx</span>',
      '#suffix' => '<style>figure {}</style>',
    );

The type of Drupal Form API field we will be building on is the ``managed_file`` field. This is the closest contributed field type to what TPPS needs. If the field definition is left like this, then it will operate the same as any other managed file field. This is important because there are a few file uploads within the TPPS form that do not require the user to define their column data, so we want to preserve the same managed file functionality for those fields.

Now we will add attributes to the file field which will include some additional information about how this specific dynamic file upload field should behave::

    $form['tree-accession']['file']['empty'] = array(
      '#default_value' => isset($values['tree-accession']['file']['empty']) ? $values['tree-accession']['file']['empty'] : 'NA',
    );

    $form['tree-accession']['file']['columns'] = array(
      '#description' => 'Please define which columns hold the required data: Plant Identifier and Location. If your plants are located based on a population group, you can provide the population group column and a mapping of population group to location below.',
    );

    $column_options = array(
      '0' => 'N/A',
      '1' => 'Plant Identifier',
      '2' => 'Country',
      '3' => 'State',
      '4' => 'Latitude',
      '5' => 'Longitude',
      '8' => 'County',
      '9' => 'District',
      '12' => 'Population Group',
    );

    ...

    $form['tree-accession']['file']['columns-options'] = array(
      '#type' => 'hidden',
      '#value' => $column_options,
    );

    $form['tree-accession']['file']['no-header'] = array();

Now, in addition to our plain managed file field, we have defined some subfields: an incomplete field named ``empty`` with a default value, an incomplete field named ``columns`` with a description, a hidden field named ``columns-options`` with an associative array of the data types we would like the user to choose from when defining their data, and an empty field named ``no-header``. These fields are important to the next section, where some hook functions and helper functions will interpret them and use them to create the column dropdowns.

TPPS Element Info Alter
-----------------------

The hook function named ``tpps_element_info_alter`` makes changes to form elements of certain types. If you would like to learn more about the hook ``hook_element_info_alter()``, you can visit `this link`_. Right now we are only interested in the changes it makes to ``managed_file`` element types::

    function tpps_element_info_alter(&$type) {
      ...

      $type['managed_file']['#process'][] = 'tpps_managed_file_process';
    }

The function appends the name of a new callback function to the end of the managed file ``#process`` attribute. This means that when the form is processed, the function named ``tpps_managed_file_process`` will be called after each of the managed file's other process functions.

TPPS Managed File Process
-------------------------

The ``tpps_managed_file_process`` function is pretty complicated, so we'll be breaking it down into smaller parts and explaining them here. The first snippet ensures that no changes are made to the managed file field if it is not inside the main TPPS form::

    function tpps_managed_file_process(array $element, array &$form_state, array $form) {
      if ($form_state['build_info']['form_id'] !== 'tpps_main') {
        return $element;
      }

The next section checks whether the columns attribute of the file has been defined, then loads some information that will be needed later, and defines the no-header checkbox. The important variables that are defined here are ``$wrapper`` and ``$fid``. ``$wrapper`` is the id of the ``div`` element that wraps the file field. This is important for ajax functions. ``$fid`` is the id of the managed file that the user has uploaded. If the user has not uploaded a file yet, then ``$fid`` will be 0::

      if (isset($element['columns'])) {
        require_once 'ajax/tpps_ajax.php';
        $fid = $element['#value']['fid'];
        $wrapper = substr($element['#id'], 0, -7) . '-ajax-wrapper';

        $saved_value_parents = $no_header_parents = $element['#parents'];
        $no_header_parents[] = '#value';
        $no_header_parents[] = 'no-header';

        $no_header = drupal_array_get_nested_value($form_state['complete form'], $no_header_parents);
        $callback = isset($form_state['triggering_element']['#ajax']['callback']) ? $form_state['triggering_element']['#ajax']['callback'] : NULL;
        if (!$no_header and ($callback != 'tpps_no_header_callback')) {
          $end = array_pop($saved_value_parents);
          $saved_value_parents[] = $end . "-no-header";
          $no_header = drupal_array_get_nested_value($form_state['saved_values'][$form_state['stage']], $saved_value_parents);
        }

        $element['no-header'] = array(
          '#type' => 'checkbox',
          '#title' => 'My file has no header row',
          '#ajax' => array(
            'wrapper' => $wrapper,
            'callback' => 'tpps_no_header_callback',
          ),
          '#states' => isset($element['#states']) ? $element['#states'] : NULL,
          '#default_value' => $no_header ? $no_header : NULL,
        );

The next snippet will check if the user has selected a file. If they have, then TPPS attempts to load it. If it loads correctly, then TPPS starts to define the ``columns`` fieldset which will hold each of the column data type fields. TPPS will then stop usage of the file (so that it may be deleted if the user clicks the remove button) and load the file uri, location of the file, the content of the file, and the column data type options::

        if (!empty($fid) and ($file = file_load($fid))) {

          $saved_vals = $form_state['saved_values'][$form_state['stage']];
          $element['columns']['#type'] = 'fieldset';
          $element['columns']['#title'] = t('<div class="fieldset-title" style="font-size:.8em">Define Data</div>');
          $element['columns']['#collapsible'] = TRUE;

          $file_name = $file->uri;

          // Stop using the file so it can be deleted if the user clicks 'remove'.
          file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));

          $location = drupal_realpath("$file_name");
          $content = tpps_parse_xlsx($location, 3, !empty($no_header));

          $options = $element['columns-options']['#value'];

Now TPPS will define the actual column data type drop down fields::

          $first = TRUE;

          foreach ($content['headers'] as $key => $item) {
            $item_parents = $element['#parents'];
            array_pop($item_parents);
            $item_parents[] = end($element['#parents']) . '-columns';
            $item_parents[] = $key;

            $default = drupal_array_get_nested_value($saved_vals, $item_parents);
            // dpm($item_parents);
            $element['columns'][$key] = array(
              '#type' => 'select',
              '#title' => $item,
              '#options' => $options,
              '#default_value' => $default,
              '#prefix' => "<th>",
              '#suffix' => "</th>",
              '#attributes' => array(
                'data-toggle' => array('tooltip'),
                'data-placement' => array('top'),
                'title' => array("Select the type of data the '$item' column holds"),
              ),
            );

            if ($first) {
              $first = FALSE;
              $first_col = $key;
            }

            if (!empty($no_header)) {
              $element['columns'][$key]['#title'] = '';
              $element['columns'][$key]['#attributes']['title'] = array("Select the type of data column $item holds");
            }
          }

Finally, TPPS will display a preview of the data found in the file and populate the subfield named ``empty`` if it exists::

          $rows = $content;
          unset($rows['headers']);
          $headers = array();
          foreach ($content['headers'] as $col_name) {
            $headers[] = $col_name;
          }
          $vars = array(
            'header' => $headers,
            'rows' => $rows,
            'attributes' => array('class' => array('view')),
            'caption' => '',
            'colgroups' => NULL,
            'sticky' => FALSE,
            'empty' => ''
          );
          $table = theme_table($vars);
          preg_match('/\A(.*<thead[A-Z|a-z|"|\'|-|_|0-9]*>).*(<\/thead>.*<\/table>)/s', $table, $matches);
          
          $element['columns'][$first_col]['#prefix'] = "<div style=\"overflow-x:auto\">" . $matches[1] . "<tr>" . $element['columns'][$first_col]['#prefix'];
          $element['columns'][$key]['#suffix'] = "</tr>" . $matches[2] . "</div>";
        }
      }

      if (isset($element['empty'])) {

        $element['empty']['#type'] = 'textfield';
        $element['empty']['#title'] = t('File Upload empty field:');
        $element['empty']['#states'] = isset($element['#states']) ? $element['#states'] : NULL;
        $element['empty']['#description'] = 'By default, TPPS will treat cells with the value "NA" as empty. If you used a different empty value indicator, please provide it here.';
      }

      return $element;
    }

The returned element should now be a fully populated TPPS dynamic file upload field.

Values and Parsing
------------------

We will now discuss the format of TPPS Dynamic file values and the ways in which they are parsed and validated.


.. _this link: https://api.drupal.org/api/drupal/modules%21system%21system.api.php/function/hook_element_info_alter/7.x

