<?php

/**
 * @file
 * Creates the Accession file form page and includes helper files.
 */

require_once 'page_3_ajax.php';
require_once 'page_3_helper.php';

/**
 * Creates the Accession file form page.
 *
 * This function creates a single accession file field if the user has indicated
 * that their study only includes one species. Otherwise, the user may choose to
 * either upload a single accession file with every species type, or to upload
 * multiple files sorted by species.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 */
function tpps_page_3_create_form(array &$form, array &$form_state) {

  if (isset($form_state['saved_values'][TPPS_PAGE_3])) {
    $values = $form_state['saved_values'][TPPS_PAGE_3];
  }
  else {
    $values = array();
  }

  if ($form_state['saved_values'][TPPS_PAGE_2]['study_type'] != 1) {
    tpps_study_location($form, $form_state);
  }

  $form['existing_trees'] = array(
    '#type' => 'checkbox',
    '#title' => t('These plants may have been studied in the past'),
    '#description' => t('If this box is checked, TPPS will try to find plants with matching ids around the same location as the ones you are providing. If it finds them successfully, it will mark them as the same plant in the database.'),
  );

  if (tpps_access('administer tpps module')) {
    $form['skip_validation'] = array(
      '#type' => 'checkbox',
      '#title' => t('Skip location validation (ignore location information)'),
    );
  }

  $form['tree-accession'] = array(
    '#type' => 'fieldset',
    '#title' => t('Plant Accession Information'),
    '#tree' => TRUE,
    '#prefix' => '<div id="tpps_accession">',
    '#suffix' => '</div>',
  );

  $species_number = $form_state['stats']['species_count'];

  if ($species_number > 1) {
    // Create the single/multiple file checkbox.
    $form['tree-accession']['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('I would like to upload a separate plant accession file for each species.'),
      '#ajax' => array(
        'wrapper' => 'tpps_accession',
        'callback' => 'tpps_accession_multi_file',
      ),
    );
  }

  $file_description = "Please upload a spreadsheet file containing plant population data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may omit that drop-down menu. Your file must contain columns with information about at least the Plant Identifier and the Location of the plant (either gps coordinates or country/state).";
  $file_upload_location = 'public://' . variable_get('tpps_accession_files_dir', 'tpps_accession');

  if ($form_state['saved_values'][TPPS_PAGE_2]['study_type'] == '4') {
    $file_description .= ' Location columns should describe the location of the source plant for the Common Garden.';
  }

  if ($species_number > 1) {
    $file_description .= " If you are uploading a single file with multiple species, your file must also specify the genus and species of each plant.";
  }

  $image_path = drupal_get_path('module', 'tpps') . '/images/';
  $file_description .= "Please find an example of an accession file below.<figure><img src=\"/{$image_path}accession_example.png\"><figcaption>Example Accession File</figcaption></figure>";

  $check = tpps_get_ajax_value($form_state, array('tree-accession', 'check'), NULL);

  for ($i = 1; $i <= $species_number; $i++) {
    $name = $form_state['saved_values'][TPPS_PAGE_1]['organism']["$i"];
    $id_name = implode("_", explode(" ", $name));

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
      '13' => 'Clone Number',
    );

    $title = t("@name Accession File: *", array('@name' => $name)) . "<br>$file_description";
    if ($species_number > 1 and !$check) {
      $title = t('Plant Accession File: *') . "<br>$file_description";
      $column_options['6'] = 'Genus';
      $column_options['7'] = 'Species';
      $column_options['10'] = 'Genus + Species';
    }

    if ($form_state['saved_values'][TPPS_PAGE_2]['study_type'] != '1') {
      $column_options['11'] = 'Source Plant Identifier';
    }

    $form['tree-accession']["species-$i"] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#states' => ($i > 1) ? array(
        'visible' => array(
          ':input[name="tree-accession[check]"]' => array('checked' => TRUE),
        ),
      ) : NULL,
    );

    $form['tree-accession']["species-$i"]['file'] = array(
      '#type' => 'managed_file',
      '#title' => $title,
      '#upload_location' => $file_upload_location,
      '#upload_validators' => array(
        'file_validate_extensions' => array('txt csv xlsx'),
      ),
      '#field_prefix' => '<span style="width: 100%;display: block;text-align: right;padding-right: 2%;">Allowed file extensions: txt csv xlsx</span>',
      '#suffix' => '<style>figure {}</style>',
      'empty' => array(
        '#default_value' => isset($values['tree-accession']["species-$i"]['file']['empty']) ? $values['tree-accession']["species-$i"]['file']['empty'] : 'NA',
      ),
      'columns' => array(
        '#description' => 'Please define which columns hold the required data: Plant Identifier and Location. If your plants are located based on a population group, you can provide the population group column and a mapping of population group to location below.',
      ),
      'no-header' => array(),
      'empty' => array(
        '#default_value' => isset($values['tree-accession']["species-$i"]['file']['empty']) ? $values['tree-accession']["species-$i"]['file']['empty'] : 'NA',
      ),
      'columns-options' => array(
        '#type' => 'hidden',
        '#value' => $column_options,
      ),
    );

    $form['tree-accession']["species-$i"]['coord-format'] = array(
      '#type' => 'select',
      '#title' => t('Coordinate Projection'),
      '#options' => array(
        'WGS 84',
        'NAD 83',
        'ETRS 89',
        'Other Coordinate Projection',
        'My file does not use coordinates for plant locations',
      ),
      '#states' => $form['tree-accession']["species-$i"]['#states'] ?? NULL,
    );

    $cols = tpps_get_ajax_value($form_state, array(
      'tree-accession',
      "species-$i",
      'file',
      'columns',
    ), NULL, 'file');

    $fid = tpps_get_ajax_value($form_state, array(
      'tree-accession',
      "species-$i",
      'file',
    ), NULL);
    if (!empty($fid)) {
      $wrapper_id = "{$fid}_map_wrapper";
      $button_id = "{$fid}_map_button";
      $form['tree-accession']["species-$i"]['coord-format']['#suffix'] = "<div id=\"$wrapper_id\"></div>"
      . "<input id=\"$button_id\" type=\"button\" value=\"Click here to view plants on map!\" class=\"btn btn-primary\"></input>";
      $no_header = tpps_get_ajax_value($form_state, array(
        'tree-accession',
        "species-$i",
        'file',
        'no_header',
      ), NULL, 'file');

      $id_col = $lat_col = $long_col = NULL;
      foreach ($cols as $key => $col) {
        if ($key[0] != '#') {
          if ((is_array($col) and $col['#value'] == '1') or (!is_array($col) and $col == '1')) {
            $id_col = $key;
          }
          if ((is_array($col) and $col['#value'] == '4') or (!is_array($col) and $col == '4')) {
            $lat_col = $key;
          }
          if ((is_array($col) and $col['#value'] == '5') or (!is_array($col) and $col == '5')) {
            $long_col = $key;
          }
        }
      }

      drupal_add_js(array(
        'tpps' => array(
          'accession_files' => array(
            $fid => array(
              'no_header' => $no_header,
              'id_col' => $id_col,
              'lat_col' => $lat_col,
              'long_col' => $long_col,
              'fid' => $fid,
            ),
          ),
        ),
      ), 'setting');

      drupal_add_js(array(
        'tpps' => array(
          'map_buttons' => array(
            $fid => array(
              'wrapper' => $wrapper_id,
              'button' => $button_id,
              'fid' => $fid,
            ),
          ),
        )
      ), 'setting');
    }

    $form['tree-accession']["species-$i"]['pop-group'] = array(
      '#type' => 'hidden',
      '#title' => 'Population group mapping',
      '#prefix' => "<div id=\"population-mapping-species-$i\">",
      '#suffix' => '</div>',
      '#tree' => TRUE,
    );

    $pop_group_show = FALSE;
    $found_lat = FALSE;
    $found_lng = FALSE;

    if (!empty($cols)) {
      foreach ($cols as $col_name => $data) {
        if ($col_name[0] == '#') {
          continue;
        }
        $val = $data;
        $fid = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$i"]['file'] ?? NULL;
        if (!empty($data['#value'])) {
          $fid = $form_state['complete form']['tree-accession']["species-$i"]['file']['#value']['fid'];
          $val = $data['#value'];
        }
        switch ($val) {
          case '4':
            $found_lat = TRUE;
            break;

          case '5':
            $found_lng = TRUE;
            break;

          case '12':
            $pop_group_show = TRUE;
            $pop_col = $col_name;
            break;

          default:
            break;
        }
      }

      if ($pop_group_show and !empty($fid) and ($file = file_load($fid))) {
        $form['tree-accession']["species-$i"]['pop-group']['#type'] = 'fieldset';
        $pop_groups = array();
        $options = array(
          'columns' => array(
            $pop_col,
          ),
          'pop_groups' => &$pop_groups,
        );
        tpps_file_iterator($fid, 'tpps_accession_pop_groups', $options);
        foreach ($pop_groups as $pop_group) {
          $form['tree-accession']["species-$i"]['pop-group'][$pop_group] = array(
            '#type' => 'textfield',
            '#title' => "Location for $name plants from group $pop_group:",
          );
        }
      }

      if ($found_lat and $found_lng) {
        unset($form['tree-accession']["species-$i"]['pop-group']['#suffix']);
        $form['tree-accession']["species-$i"]['exact_coords'] = array(
          '#type' => 'checkbox',
          '#title' => t('The provided GPS coordinates are exact'),
          '#default_value' => $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$i"]['exact_coords'] ?? TRUE,
        );

        $form['tree-accession']["species-$i"]['coord_precision'] = array(
          '#type' => 'textfield',
          '#title' => t('Coordinates accuracy:'),
          '#description' => t('The precision of the provided coordinates. For example, if a plant could be up to 10m awa from the provided coordinates, then the accuracy would be "10m".'),
          '#suffix' => '</div>',
          '#states' => array(
            'visible' => array(
              ":input[name=\"tree-accession[species-$i][exact_coords]\"]" => array('checked' => FALSE),
            ),
          ),
        );
      }
    }
  }

  $map_api_key = variable_get('tpps_maps_api_key', NULL);
  if (!empty($map_api_key)) {
    $form['tree-accession']['#suffix'] .= '
    <script src="https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/markerclusterer.js"></script>
    <script src="https://maps.googleapis.com/maps/api/js?key=' . $map_api_key . '&callback=initMap"
    async defer></script>
    <style>
      #map_wrapper {
      height: 450px;
      }
    </style>';
  }

  $form['Back'] = array(
    '#type' => 'submit',
    '#value' => t('Back'),
    '#prefix' => '<div class="input-description">* : Required Field</div>',
  );

  $form['Save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['Next'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );
}
