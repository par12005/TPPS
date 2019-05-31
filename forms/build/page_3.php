<?php

/**
 * @file
 * Creates the Accession file form page and includes helper files.
 */

require_once 'page_3_ajax.php';

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
function page_3_create_form(array &$form, array &$form_state) {

  if (isset($form_state['saved_values'][TPPS_PAGE_3])) {
    $values = $form_state['saved_values'][TPPS_PAGE_3];
  }
  else {
    $values = array();
  }

  $form['tree-accession'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );

  $species_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];

  if ($species_number > 1) {
    // Create the single/multiple file checkbox.
    $form['tree-accession']['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('I would like to upload a separate tree accession file for each species.'),
    );
  }

  $file_description = "Please upload a spreadsheet file containing tree population data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may omit that drop-down menu. Your file must contain columns with information about at least the Tree Identifier and the Location of the tree (either gps coordinates or country/state).";
  $file_upload_location = 'public://' . variable_get('tpps_accession_files_dir', 'tpps_accession');

  if ($form_state['saved_values'][TPPS_PAGE_2]['study_type'] == '4') {
    $file_description .= ' Location columns should describe the location of the source tree for the Common Garden.';
  }

  $form['tree-accession']['file'] = array(
    '#type' => 'managed_file',
    '#title' => t("Tree Accession File: please provide a spreadsheet with columns for the Tree ID and location of trees used in this study: *"),
    '#upload_location' => "$file_upload_location",
    '#upload_validators' => array(
      'file_validate_extensions' => array('txt csv xlsx'),
    ),
    '#description' => $file_description,
    '#states' => ($species_number > 1) ? (array(
      'visible' => array(
        ':input[name="tree-accession[check]"]' => array('checked' => FALSE),
      ),
    )) : NULL,
  );

  if ($species_number > 1) {
    $form['tree-accession']['file']['#description'] .= " If you are uploading a single file with multiple species, your file must also specify the genus and species of each tree.";
  }

  $form['tree-accession']['file']['empty'] = array(
    '#default_value' => isset($values['tree-accession']['file']['empty']) ? $values['tree-accession']['file']['empty'] : 'NA',
  );

  $form['tree-accession']['file']['columns'] = array(
    '#description' => 'Please define which columns hold the required data: Tree Identifier and Location. If your trees are located based on a population group, you can provide the population group column and a mapping of population group to location below.',
  );

  $column_options = array(
    '0' => 'N/A',
    '1' => 'Tree Identifier',
    '2' => 'Country',
    '3' => 'State',
    '4' => 'Latitude',
    '5' => 'Longitude',
    '8' => 'County',
    '9' => 'District',
    '12' => 'Population Group',
  );

  if ($species_number > 1) {
    $column_options['6'] = 'Genus';
    $column_options['7'] = 'Species';
    $column_options['10'] = 'Genus + Species';
  }

  if ($form_state['saved_values'][TPPS_PAGE_2]['study_type'] != '1') {
    $column_options['11'] = 'Source Tree Identifier';
  }

  $form['tree-accession']['file']['columns-options'] = array(
    '#type' => 'hidden',
    '#value' => $column_options,
  );

  $form['tree-accession']['file']['no-header'] = array();

  $form['tree-accession']['coord-format'] = array(
    '#type' => 'select',
    '#title' => t('Coordinate Projection'),
    '#options' => array(
      'WGS 84',
      'NAD 83',
      'ETRS 89',
      'Other Coordinate Projection',
      'My file does not use coordinates for tree locations',
    ),
    '#states' => $form['tree-accession']['file']['#states'],
    // Add map button after coordinate format option.
    '#suffix' => "<div id=\"map_wrapper\"></div>"
    . "<input id=\"map_button\" type=\"button\" value=\"Click here to view trees on map!\"></input>",
  );

  // Add the google maps api call after the map button.
  $map_api_key = variable_get('tpps_maps_api_key', NULL);
  if (!empty($map_api_key)) {
    $form['tree-accession']['coord-format']['#suffix'] .= '
    <script src="https://maps.googleapis.com/maps/api/js?key=' . $map_api_key . '&callback=initMap"
    async defer></script>
    <style>
      #map_wrapper {
      height: 450px;
      }
    </style>';
  }

  $pop_group_show = FALSE;
  if (isset($form_state['complete form']['tree-accession']['file']['columns'])) {
    foreach ($form_state['complete form']['tree-accession']['file']['columns'] as $col_name => $data) {
      if ($col_name[0] == '#') {
        continue;
      }
      elseif ($data['#value'] == '12') {
        $pop_group_show = TRUE;
        $pop_col = $col_name;
        $fid = $form_state['complete form']['tree-accession']['file']['#value']['fid'];
        break;
      }
    }
  }
  elseif (isset($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file-columns'])) {
    foreach ($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file-columns'] as $col_name => $data) {
      if ($data == '12') {
        $pop_group_show = TRUE;
        $pop_col = $col_name;
        $fid = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file'];
        break;
      }
    }
  }

  $form['tree-accession']['pop-group'] = array(
    '#type' => 'hidden',
    '#title' => 'Population group mapping',
    '#prefix' => '<div id="population-mapping">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
  );

  if (!empty($fid) and ($file = file_load($fid)) and $pop_group_show) {
    $form['tree-accession']['pop-group']['#type'] = 'fieldset';

    $file_name = $file->uri;
    $location = drupal_realpath("$file_name");
    $content = tpps_parse_xlsx($location);

    for ($i = 0; $i < count($content) - 1; $i++) {
      $pop_group = $content[$i][$pop_col];
      if (empty($form['tree-accession']['pop-group'][$pop_group])) {
        $form['tree-accession']['pop-group'][$pop_group] = array(
          '#type' => 'textfield',
          '#title' => "Location for trees from group $pop_group:",
        );
      }
    }
  }

  if ($species_number > 1) {
    for ($i = 1; $i <= $species_number; $i++) {
      $name = $form_state['saved_values'][TPPS_PAGE_1]['organism']["$i"];

      $form['tree-accession']["species-$i"] = array(
        '#type' => 'fieldset',
        '#title' => "<div class=\"fieldset-title\">" . t("Tree Accession information for @name trees:", array('@name' => $name)) . "</div>",
        '#states' => array(
          'visible' => array(
            ':input[name="tree-accession[check]"]' => array('checked' => TRUE),
          ),
        ),
        '#collapsible' => TRUE,
      );

      $form['tree-accession']["species-$i"]['file'] = array(
        '#type' => 'managed_file',
        '#title' => t("@name Accession File: please provide a spreadsheet with columns for the Tree ID and location of the @name trees used in this study: *", array('@name' => $name)),
        '#upload_location' => "$file_upload_location",
        '#upload_validators' => array(
          'file_validate_extensions' => array('txt csv xlsx'),
        ),
        '#description' => $file_description,
        '#tree' => TRUE,
      );

      $form['tree-accession']["species-$i"]['file']['empty'] = array(
        '#default_value' => isset($values['tree-accession']["species-$i"]['file']['empty']) ? $values['tree-accession']["species-$i"]['file']['empty'] : 'NA',
      );

      $form['tree-accession']["species-$i"]['file']['columns'] = array(
        '#description' => 'Please define which columns hold the required data: Tree Identifier and Location. If your trees are located based on a population group, you can provide the population group column and a mapping of population group to location below.',
      );

      $column_options = array(
        '0' => 'N/A',
        '1' => 'Tree Identifier',
        '2' => 'Country',
        '3' => 'State',
        '4' => 'Latitude',
        '5' => 'Longitude',
        '8' => 'County',
        '9' => 'District',
        '12' => 'Population Group',
      );

      if ($form_state['saved_values'][TPPS_PAGE_2]['study_type'] != '1') {
        $column_options['11'] = 'Source Tree Identifier';
      }

      $form['tree-accession']["species-$i"]['file']['columns-options'] = array(
        '#type' => 'hidden',
        '#value' => $column_options,
      );

      $form['tree-accession']["species-$i"]['file']['no-header'] = array();
      $parts = explode(" ", $name);
      $id_name = implode("_", $parts);
      $form['tree-accession']["species-$i"]['#suffix'] = "<div id=\"{$id_name}_map_wrapper\"></div>"
        . "<input id=\"{$id_name}_map_button\" type=\"button\" value=\"Click here to view $name trees on map!\"></input>"
        . "<div id=\"{$id_name}_species_number\" style=\"display:none;\">$i</div>";

      $pop_group_show = FALSE;
      if (isset($form_state['complete form']['tree-accession']["species-$i"]['file']['columns'])) {
        foreach ($form_state['complete form']['tree-accession']["species-$i"]['file']['columns'] as $col_name => $data) {
          if ($col_name[0] == '#') {
            continue;
          }
          elseif ($data['#value'] == '12') {
            $pop_group_show = TRUE;
            $pop_col = $col_name;
            break;
          }
        }
      }

      $form['tree-accession']["species-$i"]['pop-group'] = array(
        '#type' => 'hidden',
        '#title' => 'Population group mapping',
        '#prefix' => '<div id="population-mapping">',
        '#suffix' => '</div>',
        '#tree' => TRUE,
      );

      if (!empty($form_state['complete form']['tree-accession']["species-$i"]['file']['#value']['fid']) and ($file = file_load($form_state['complete form']['tree-accession']["species-$i"]['file']['#value']['fid'])) and $pop_group_show) {
        $form['tree-accession']["species-$i"]['pop-group']['#type'] = 'fieldset';

        $file_name = $file->uri;
        $location = drupal_realpath("$file_name");
        $content = tpps_parse_xlsx($location);

        for ($i = 0; $i < count($content) - 1; $i++) {
          $pop_group = $content[$i][$pop_col];
          if (empty($form['tree-accession']["species-$i"]['pop-group'][$pop_group])) {
            $form['tree-accession']["species-$i"]['pop-group'][$pop_group] = array(
              '#type' => 'textfield',
              '#title' => "Location for $name trees from group $pop_group:",
            );
          }
        }
      }

    }
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
