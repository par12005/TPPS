<?php

/**
 * @file
 * Defines the data integrity checks for the third page of the form.
 */

/**
 * Defines the data integrity checks for the third page of the form.
 *
 * @param array $form
 *   The form that is being validated.
 * @param array $form_state
 *   The state of the form that is being validated.
 */
function tpps_page_3_validate_form(array &$form, array &$form_state) {
  if ($form_state['submitted'] == '1') {
    unset($form_state['file_info'][TPPS_PAGE_3]);

    if (!empty($form_state['values']['study_location'])) {
      if (!$form_state['values']['study_location']['type']) {
        form_set_error('study_location][type', 'Location Format: field is required.');
      }
      else {
        $locs = $form_state['values']['study_location']['locations'];
        for ($i = 1; $i <= $locs['number']; $i++) {
          if (empty($locs[$i])) {
            form_set_error("study_location][locations][$i", "Location $i: field is required.");
          }
        }
      }
    }

    $species_number = $form_state['stats']['species_count'];
    $multi_file = !empty($form_state['values']['tree-accession']['check']);

    for ($i = 1; $i <= $species_number; $i++) {
      $values = &$form_state['values']['tree-accession']["species-$i"];
      $file_element = $form['tree-accession']["species-$i"]['file'];

      if (empty($values['file'])) {
        form_set_error("tree-accession][species-$i][file", 'Plant Accession file: field is required.');
      }
      else {
        $required_groups = array(
          'Tree Id' => array(
            'id' => array(1),
          ),
          'Location (latitude/longitude or country/state or population group)' => array(
            'approx' => array(2, 3),
            'gps' => array(4, 5),
            'pop_group' => array(12),
          ),
        );

        if (!empty($form_state['values']['skip_validation'])) {
          unset($required_groups['Location (latitude/longitude or country/state or population group)']);
        }

        if (!$multi_file and $species_number > 1) {
          $required_groups['Genus and Species'] = array(
            'separate' => array(6, 7),
            'combined' => array(10),
          );
        }

        $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

        if (gettype($values['pop-group']) === 'array') {
          foreach ($values['pop-group'] as $pop_name => $location) {
            if (empty($location)) {
              form_set_error("tree-accession][species-$i][pop-group][$pop_name", "Population Group $pop_name Location: field is required.");
            }
          }
        }

        if (isset($values['exact_coords']) and !$values['exact_coords'] and empty($values['coord_precision'])) {
          form_set_error("tree-accession][species-$i][coord_precision", "Coordinates accuracy: field is required.");
        }

        if (!form_get_errors() and empty($form_state['values']['skip_validation'])) {
          $options = array(
            'no_header' => !empty($values['file-no-header']),
            'loc_options' => $required_groups['Location (latitude/longitude or country/state or population group)'],
            'id_col' => $groups['Tree Id']['1'],
            'loc_cols' => $groups['Location (latitude/longitude or country/state or population group)'],
            'empty' => $values['file-empty'],
            'org_num' => $i,
          );
          tpps_file_iterator($values['file'], 'tpps_accession_valid_locations', $options);
        }

        if (!form_get_errors()) {
          $file = file_load($values['file']);
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
          $species = implode('_', explode(' ', $form_state['saved_values'][TPPS_PAGE_1]['organism'][$i]['name']));
          $form_state['file_info'][TPPS_PAGE_3][$file->fid] = "Plant_Accession_$species";
        }
      }

      if (!$multi_file) {
        break;
      }
    }

    if (form_get_errors()) {
      $form_state['rebuild'] = TRUE;
      $new_form = drupal_rebuild_form('tpps_main', $form_state, $form);
      for ($i = 1; $i <= $species_number; $i++) {
        $form['tree-accession']["species-$i"]['file']['upload'] = $new_form['tree-accession']["species-$i"]['file']['upload'];
        $form['tree-accession']["species-$i"]['file']['columns'] = $new_form['tree-accession']["species-$i"]['file']['columns'];
        $form['tree-accession']["species-$i"]['file']['upload']['#id'] = "edit-tree-accession-species-$i-file-upload";
        $form['tree-accession']["species-$i"]['file']['columns']['#id'] = "edit-tree-accession-species-$i-file-columns";
        if (!$multi_file) {
          break;
        }
      }
    }
  }
}

/**
 * This function processes a single row of a plant accession file.
 *
 * This function validates that the accession file has valid/complete location
 * information for each plant. This function is meant to be used with
 * tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_accession_valid_locations($row, &$options) {
  $id_name = $options['id_col'];
  $empty = $options['empty'];
  $location_options = $options['loc_options'];
  $location_columns = $options['loc_cols'];
  $location_types = $location_columns['#type'] ?? array();
  $org_num = $options['org_num'];
  $reason = "";

  if (gettype($location_types) !== 'array') {
    $location_types = array($location_types);
  }

  if (!empty($row[$id_name])) {
    $valid_row = FALSE;
    foreach ($location_types as $type) {
      $valid_combination = TRUE;
      foreach ($location_options[$type] as $column) {
        if (empty($row[$location_columns[$column]]) or $row[$location_columns[$column]] == $empty) {
          $valid_combination = FALSE;
          $reason = "missing";
        }
        elseif ($type == 'gps' and abs($row[$location_columns[$column]]) > 180) {
          $valid_combination = FALSE;
          $reason = "invalid (invalid coordinate value: {$row[$location_columns[$column]]})";
        }
      }
      if ($valid_combination) {
        $valid_row = TRUE;
        break;
      }
    }
    if (!$valid_row) {
      form_set_error("tree-accession-species-$org_num-file-{$row[$id_name]}", "Plant Accession file: Some location information is $reason for plant \"{$row[$id_name]}\".");
    }
  }
}
