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
  $file_description .= 'Please find an example of an accession file below.'
    . '<figure><img src="/' . TPPS_IMAGES_PATH . 'accession_example.png">'
    . '<figcaption>Example Accession File</figcaption></figure>';

  $check = tpps_get_ajax_value($form_state, ['tree-accession', 'check'], NULL);

  for ($i = 1; $i <= $species_number; $i++) {
    $name = $form_state['saved_values'][TPPS_PAGE_1]['organism']["$i"]['name'];

    $column_options = [
      '0' => 'N/A',
      TPPS_COLUMN_PLANT_IDENTIFIER => t('Plant Identifier'),
      '2' => t('Country'),
      '3' => t('State'),
      TPPS_COLUMN_LATITUDE => t('Latitude'),
      TPPS_COLUMN_LONGITUDE => t('Longitude'),
      '8' => t('County'),
      '9' => t('District'),
      TPPS_COLUMN_POPULATION_GROUP => t('Population Group'),
      '13' => t('Clone Number'),
    ];

    $title = t('@name Accession File: *', ['@name' => $name])
      . '<br />' .$file_description;
    if ($species_number > 1 and !$check) {
      $title = t('Plant Accession File: *') . "<br>$file_description";
      $column_options['6'] = t('Genus');
      $column_options['7'] = t('Species');
      $column_options['10'] = t('Genus + Species');
    }

    if ($form_state['saved_values'][TPPS_PAGE_2]['study_type'] != '1') {
      $column_options['11'] = t('Source Plant Identifier');
    }

    $form['tree-accession']["species-$i"] = [
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#states' => ($i > 1) ? [
        'visible' => [
          ':input[name="tree-accession[check]"]' => ['checked' => TRUE],
        ],
      ] : NULL,
    ];

    // @TODO Move styles to CSS files.
    $form['tree-accession']["species-$i"]['file'] = array(
      '#type' => 'managed_file',
      '#title' => $title,
      '#upload_location' => $file_upload_location,
      '#upload_validators' => [
        'file_validate_extensions' => ['txt csv'],
      ],
      '#field_prefix' => '<span style="width: 100%;display: block;text-align: right;padding-right: 2%;">Allowed file extensions: txt csv</span>',
      '#suffix' => '<style>figure {}</style>',
      'empty' => [
        '#default_value' => $values['tree-accession']["species-$i"]['file']['empty'] ?? 'NA',
      ],
      'columns' => [
        '#description' => t('Please define which columns hold the required data: Plant Identifier and Location. If your plants are located based on a population group, you can provide the population group column and a mapping of population group to location below.'),
      ],
      'no-header' => [],
      'columns-options' => [
        '#type' => 'hidden',
        '#value' => $column_options,
      ],
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
      '#prefix' => "<div id=\"population-mapping-species-$i\">",
    );

    $cols = tpps_get_ajax_value($form_state, array(
      'tree-accession',
      "species-$i",
      'file',
      'columns',
    ), NULL, 'file');

    // [VS]
    // Previously $fid was an array which caused warnings on Page 3 submit.
    $fid = tpps_get_ajax_value(
      $form_state,
      ['tree-accession', "species-$i", 'file', 'fid'],
      NULL
    );
    // [/VS]
    // When $fid is NULL it cause warning message.
    $file = file_load(($fid ?? ''));
    if ($file && ($file->filesize ?? FALSE) && empty($skip)) {
      $wrapper_id = "{$fid}_map_wrapper";
      $button_id = "{$fid}_map_button";
      $form['tree-accession']["species-$i"]['coord-format']['#suffix']
        = "<div id=\"$wrapper_id\"></div>"
        . "<input id=\"$button_id\" type=\"button\" "
        . "value=\"Click here to view plants on map!\" "
        . "class=\"btn btn-primary form-button map-button\"></input>";
      $no_header = tpps_get_ajax_value(
        $form_state,
        ['tree-accession', "species-$i", 'file', 'no_header'],
        NULL,
        'file'
      );

      $id_col = $lat_col = $long_col = NULL;
      foreach ($cols as $key => $col) {
        if ($key[0] != '#') {
          // '1' => 'Plant Identifier',
          if (
            (is_array($col) and $col['#value'] == TPPS_COLUMN_PLANT_IDENTIFIER)
            || (!is_array($col) and $col == TPPS_COLUMN_PLANT_IDENTIFIER)
          ) {
            $id_col = $key;
          }
          if (
            (is_array($col) && $col['#value'] == TPPS_COLUMN_LATITUDE)
            || (!is_array($col) and $col == TPPS_COLUMN_LATITUDE)
          ) {
            $lat_col = $key;
          }
          if (
            (is_array($col) and $col['#value'] == TPPS_COLUMN_LONGITUDE)
            || (!is_array($col) and $col == TPPS_COLUMN_LONGITUDE)
          ) {
            $long_col = $key;
          }
        }
      }
      $form['#attached']['js'][] = [
        'type' => 'setting',
        'scope' => 'footer',
        'data' => [
          'tpps' => [
            'accession_files' => [
              $fid => [
                'no_header' => $no_header,
                'id_col' => $id_col,
                'lat_col' => $lat_col,
                'long_col' => $long_col,
                'fid' => $fid,
              ],
            ],
            'map_buttons' => [
              $fid => [
                'wrapper' => $wrapper_id,
                'button' => $button_id,
                'fid' => $fid,
              ],
            ],
          ],
        ],
      ];
    }
    $form['tree-accession']["species-$i"]['pop-group'] = array(
      '#type' => 'hidden',
      '#title' => 'Population group mapping',
      '#suffix' => '</div>',
      '#tree' => TRUE,
    );

    $pop_group_show = FALSE;
    $found_lat = FALSE;
    $found_lng = FALSE;

    if (!empty($cols)) {
      foreach ($cols as $col_name => $data) {

        if (empty($col_name) || $col_name[0] == '#') {
          continue;
        }
        $val = $data;
        $fid = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$i"]['file'] ?? NULL;
        if (!empty($data['#value'])) {
          $fid = $form_state['complete form']['tree-accession']["species-$i"]['file']['#value']['fid'];
          $val = $data['#value'];
        }
        switch ($val) {
          case TPPS_COLUMN_LATITUDE:
            $found_lat = TRUE;
            break;

          case TPPS_COLUMN_LONGITUDE:
            $found_lng = TRUE;
            break;

          case TPPS_COLUMN_POPULATION_GROUP:
            $pop_group_show = TRUE;
            $pop_col = $col_name;
            break;

          default:
            break;
        }
      }

      if ($pop_group_show && $file && ($file->filesize ?? FALSE)) {
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
            '#description' => t('The location for this population. This should be GPS coordinates if possbile, otherwise this can be the name of a location.'),
          );
        }
      }

      if ($found_lat and $found_lng) {
        unset($form['tree-accession']["species-$i"]['pop-group']['#suffix']);
        // [VS] #8669py308
        $form['tree-accession']["species-$i"]['location_accuracy'] = [
          '#type' => 'select',
          '#title' => t('Location accuracy: *'),
          '#default_value' => $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$i"]['location_accuracy'] ?? 'exact',
          '#options' => [
            'exact' => t('Exact'),
            'approximate' => t('Approximate'),
            'descriptive_place' => t('Descriptive Place'),
          ],
        ];
        $form['tree-accession']["species-$i"]['descriptive_place'] = [
          '#type' => 'select',
          '#title' => t('Descriptive place: *'),
          '#default_value' => $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$i"]['descriptive_place'] ?? 'street',
          '#options' => [
            'street' => t('Street'),
            'city' => t('City'),
            'county' => t('County'),
            'state/province' => t('State/province'),
            'country' => t('Country'),
          ],
          '#states' => [
            'visible' => [
              ":input[name=\"tree-accession[species-$i][location_accuracy]\"]" => [
                'value' => 'descriptive_place'
              ],
            ],
          ],
        ];
        $form['tree-accession']["species-$i"]['coord_precision'] = [
          '#type' => 'textfield',
          '#title' => t('Coordinates accuracy: *'),
          '#description' => t('The precision of the provided coordinates. '
            . 'For example, if a plant could be up to 10m awa from the '
            . 'provided coordinates, then the accuracy would be "10m".'),
          '#suffix' => '</div>',
          '#states' => [
            'visible' => [
              ":input[name=\"tree-accession[species-$i][location_accuracy]\"]" => [
                'value' => 'approximate'
              ],
            ],
          ],
        ];
        // [/VS] #8669py308
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
  tpps_add_buttons($form, 'page_3');
  return $form;
}
