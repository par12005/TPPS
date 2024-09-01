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
    '#title' => t('PLANT ACCESSION INFORMATION'),
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

  $file_description = 'Please upload a spreadsheet file containing plant '
    . 'population data. When your file is uploaded, you will be shown a '
    . 'table with your column header names, several drop-downs, and '
    . 'the first few rows of your file. You will be asked to define the '
    . 'data type for each column, using the drop-downs provided to you. '
    . 'If a column data type does not fit any of the options in the '
    . 'drop-down menu, you may omit that drop-down menu. Your file must '
    . 'contain columns with information about at least the Plant Identifier '
    . 'and the Location of the plant (either gps coordinates or country/state).';

  if ($form_state['saved_values'][TPPS_PAGE_2]['study_type'] == '4') {
    $file_description .= ' Location columns should describe the location '
      . 'of the source plant for the Common Garden.';
  }

  if ($species_number > 1) {
    $file_description .= ' If you are uploading a single file with '
      . 'multiple species, your file must also specify the genus and '
      . 'species of each plant.';
  }
  $file_description .= ' Please find an example of an accession file below.'
    . '<figure style="text-align:center;">'
      . '<img src="/' . TPPS_IMAGES_PATH . 'accession_example.png">'
      . '<figcaption>Example Accession File</figcaption>'
      . '</figure>';

  $file_upload_location = 'public://'
    . variable_get('tpps_accession_files_dir', 'tpps_accession');

  $check = tpps_get_ajax_value($form_state, ['tree-accession', 'check'], NULL);

  for ($i = 1; $i <= $species_number; $i++) {
    $name = $form_state['saved_values'][TPPS_PAGE_1]['organism']["$i"]['name'];

    $column_options = [
      '0' => 'N/A',
      TPPS_COLUMN_PLANT_IDENTIFIER => t('Plant Identifier'),
      TPPS_COLUMN_COUNTRY => t('Country'),
      TPPS_COLUMN_STATE => t('State'),
      TPPS_COLUMN_LATITUDE => t('Latitude'),
      TPPS_COLUMN_LONGITUDE => t('Longitude'),
      TPPS_COLUMN_COUNTY => t('County'),
      TPPS_COLUMN_DISTRICT => t('District'),
      TPPS_COLUMN_POPULATION_GROUP => t('Population Group'),
      TPPS_COLUMN_CLONE_NUMBER => t('Clone Number'),
    ];

    $title = t('@name Accession File: *', ['@name' => $name]);
    if ($species_number > 1 && !$check) {
      $title = t('Plant Accession File: *');
      $column_options[TPPS_COLUMN_GENUS] = t('Genus');
      $column_options[TPPS_COLUMN_SPECIES] = t('Species');
      $column_options[TPPS_COLUMN_GENUS_AND_SPECIES] = t('Genus + Species');
    }

    if ($form_state['saved_values'][TPPS_PAGE_2]['study_type'] != '1') {
      $column_options[TPPS_COLUMN_SOURCE_PLANT_IDENTIFIER]
        = t('Source Plant Identifier');
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
    $form['tree-accession']["species-$i"]['file'] = [
      '#type' => 'managed_file',
      '#title' => $title,
      '#upload_location' => $file_upload_location,
      '#upload_validators' => [
        'file_validate_extensions' => ['txt csv'],
      ],
      '#field_prefix' => '<br />' . t($file_description)
        . '<span style="width: 100%; display: block; text-align: right; '
        . 'padding-right: 2%;">Allowed file extensions: txt csv</span>',
      'empty' => [
        '#default_value' => $values['tree-accession']["species-$i"]['file']['empty'] ?? 'NA',
      ],
      'columns' => [
        '#description' => t('Please define which columns hold the required '
          . 'data: Plant Identifier and Location. If your plants are located '
          . 'based on a population group, you can provide the population '
          . 'group column and a mapping of population group to location below.'
        ),
      ],
      'no-header' => [],
      'columns-options' => [
        '#type' => 'hidden',
        '#value' => $column_options,
      ],
    ];

    $form['tree-accession']["species-$i"]['coord-format'] = [
      '#type' => 'select',
      '#title' => t('Coordinate Projection'),
      '#options' => [
        'WGS 84',
        'NAD 83',
        'ETRS 89',
        'Other Coordinate Projection',
        'My file does not use coordinates for plant locations',
      ],
      '#states' => $form['tree-accession']["species-$i"]['#states'] ?? NULL,
      // @TODO Should closing div be here?
      '#prefix' => "<div id=\"population-mapping-species-$i\">",
    ];

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
    if ($file = tpps_file_load($fid)) {
      // The same code in tpps_table_display().
      $wrapper_id = $fid . '_map_wrapper';
      $button_id = $fid . '_map_button';
      $form['tree-accession']["species-$i"]['coord-format']['#suffix'] =
        '<div id="' . $wrapper_id . '"></div>'
        . '<input id="' . $button_id . '" type="button" '
        . ' class="form-button form-submit" value="'
        . t('Click here to view plants on map') . '"></input>';
      $js_settings = [
        'map_buttons' => [
          $fid => [
            'wrapper' => $wrapper_id,
            'button' => $button_id,
            'fid' => $fid,
          ],
        ],
      ];
      $form['#attached']['js'][] = [
        'type' => 'setting',
        'scope' => 'footer',
        'data' => ['tpps' => $js_settings],
      ];
      tpps_add_css_js('google_map', $form);
    }

    if ($file = tpps_file_load($fid)) {
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
          ]
        ]
      ];
    }
    $form['tree-accession']["species-$i"]['pop-group'] = [
      '#type' => 'hidden',
      '#title' => 'Population group mapping',
      '#suffix' => '</div>',
      '#tree' => TRUE,
    ];

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

  if (variable_get('tpps_maps_api_key', NULL)) {
    // @todo Add JS using drupal_add_js().

    // Wrapper is NOT required here.
    $form['tree-accession']['#suffix'] .= tpps_get_markercluster_code(FALSE);
      // @todo Move CSS to /css/tpps.css.
      //. '<style>#map_wrapper { height: 450px; } </style>';
  }
  tpps_form_autofocus($form, ['tree-accession', 'species-1', 'file']);
  tpps_add_css_js('google_map', $form);
  tpps_form_add_buttons(['form' => &$form, 'page' => 'page_3']);
  return $form;
}
