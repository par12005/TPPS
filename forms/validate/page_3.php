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
      elseif ($form_state['values']['study_location']['type'] != '2') {
        if (!$form_state['values']['study_location']['coordinates']) {
          form_set_error('study_location][coordinates', 'Coordinates: field is required.');
        }
      }
      else {
        if (!$form_state['values']['study_location']['custom']) {
          form_set_error('study_location][custom', 'Custom Location: field is required.');
        }
      }
    }

    $species_number = $form_state['stats']['species_count'];
    $form_state['stats']['tree_count'] = 0;
    $multi_file = !empty($form_state['values']['tree-accession']['check']);

    for ($i = 1; $i <= $species_number; $i++) {
      $values = &$form_state['values']['tree-accession']["species-$i"];
      $file_element = $form['tree-accession']["species-$i"]['file'];

      if (empty($values['file'])) {
        form_set_error("tree-accession][species-$i][file", 'Tree Accession file: field is required.');
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

        if (!form_get_errors()) {
          $id_name = $groups['Tree Id']['1'];
          $col_names = $values['file-columns'];
          $content = tpps_parse_file($values['file'], 0, !empty($values['file-no-header']));
          $empty = $values['file-empty'];
          $location_options = $required_groups['Location (latitude/longitude or country/state or population group)'];;
          $location_columns = $groups['Location (latitude/longitude or country/state or population group)'];
          $location_types = $location_columns['#type'];

          if (gettype($location_types) !== 'array') {
            $location_types = array($location_types);
          }

          foreach ($content as $row => $vals) {
            if ($row !== 'headers' and !empty($vals[$id_name])) {
              $valid_row = FALSE;
              foreach ($location_types as $type) {
                $valid_combination = TRUE;
                foreach ($location_options[$type] as $column) {
                  if (empty($vals[$location_columns[$column]]) or $vals[$location_columns[$column]] == $empty) {
                    $valid_combination = FALSE;
                  }
                }
                if ($valid_combination) {
                  $valid_row = TRUE;
                  break;
                }
              }
              if (!$valid_row) {
                form_set_error("tree-accession][species-$i][file", "Tree Accession file: Some location information is missing for tree \"{$vals[$id_name]}\".");
              }
            }
          }
        }

        if (!form_get_errors()) {
          $file = file_load($values['file']);
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
          $form_state['file_info'][TPPS_PAGE_3][$file->fid] = 'Tree_Accession';
          $form_state['stats']['tree_count'] += count($content) - 1;
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
