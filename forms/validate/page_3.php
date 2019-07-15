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
function page_3_validate_form(array &$form, array &$form_state) {
  if ($form_state['submitted'] == '1') {
    
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

    $species_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
    if ($species_number == 1 or $form_state['values']['tree-accession']['check'] == '0') {
      if ($form_state['values']['tree-accession']['file'] != "") {

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

        if ($species_number != 1) {
          $required_groups['Genus and Species'] = array(
            'separate' => array(6, 7),
            'combined' => array(10),
          );
        }

        $file_element = $form['tree-accession']['file'];
        $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

        if (!form_get_errors()) {
          $id_name = $groups['Tree Id']['1'];
          $col_names = $form_state['values']['tree-accession']['file-columns'];
          $fid = $form_state['values']['tree-accession']['file'];
          $file = file_load($fid);
          $file_name = $file->uri;
          $location = drupal_realpath($file_name);
          $content = tpps_parse_xlsx($location, 0, !empty($form_state['values']['tree-accession']['file-no-header']));

          $empty = $form_state['values']['tree-accession']['file-empty'];
          foreach ($content as $row => $vals) {
            if ($row !== 'headers' and isset($vals[$id_name]) and $vals[$id_name] !== "") {
              foreach ($col_names as $item => $val) {
                if ((!isset($vals[$item]) or $vals[$item] === $empty) and $val) {
                  $field = $file_element['columns'][$item]['#options'][$val];
                  form_set_error("tree-accession][file][columns][{$vals[$id_name]}", "Tree Accession file: the required field $field is empty for tree \"{$vals[$id_name]}\".");
                }
              }
            }
          }

          if (!form_get_errors()) {
            $form_state['values']['tree-accession']['tree_count'] = count($content) - 1;
          }
        }

        if (!form_get_errors()) {
          // Preserve file if it is valid.
          $file = file_load($form_state['values']['tree-accession']['file']);
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
        }
      }
      else {
        form_set_error('tree-accession][file', 'Tree Accession file: field is required.');
      }

      if (gettype($form_state['values']['tree-accession']['pop-group']) === 'array') {
        foreach ($form_state['values']['tree-accession']['pop-group'] as $pop_name => $location) {
          if (empty($location)) {
            form_set_error("tree-accession][pop-group][$pop_name", "Population Group $pop_name Location: field is required.");
          }
        }
      }
    }
    else {
      $form_state['values']['tree-accession']['tree_count'] = 0;
      for ($i = 1; $i <= $species_number; $i++) {
        if ($form_state['values']['tree-accession']["species-$i"]['file'] != "") {

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

          $file_element = $form['tree-accession']["species-$i"]['file'];
          $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

          if (!form_get_errors()) {
            $id_name = $groups['Tree Id']['1'];
            $col_names = $form_state['values']['tree-accession']["species-$i"]['file-columns'];
            $fid = $form_state['values']['tree-accession']["species-$i"]['file'];
            $file = file_load($fid);
            $file_name = $file->uri;
            $location = drupal_realpath($file_name);
            $content = tpps_parse_xlsx($location, 0, !empty($form_state['values']['tree-accession']["species-$i"]['file-no-header']));

            $empty = $form_state['values']['tree-accession']["species-$i"]['file-empty'];
            foreach ($content as $row => $vals) {
              if ($row !== 'headers' and isset($vals[$id_name]) and $vals[$id_name] !== "") {
                foreach ($col_names as $item => $val) {
                  if ((!isset($vals[$item]) or $vals[$item] === $empty) and $val) {
                    $field = $file_element['columns'][$item]['#options'][$val];
                    form_set_error("tree-accession][species-$i][file][columns][{$vals[$id_name]}", "Tree Accession $i file: the required field $field is empty for tree \"{$vals[$id_name]}\".");
                  }
                }
              }
            }

            if (!form_get_errors()) {
              $form_state['values']['tree-accession']['tree_count'] += count($content) - 1;
            }
          }

          if (!form_get_errors()) {
            // Preserve file if it is valid.
            $file = file_load($form_state['values']['tree-accession']["species-$i"]['file']);
            file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
          }
        }
        else {
          form_set_error("tree-accession][species-$i][file", "Species $i Tree Accession file: field is required.");
        }

        if (gettype($form_state['values']['tree-accession']["species-$i"]['pop-group']) === 'array') {
          foreach ($form_state['values']['tree-accession']["species-$i"]['pop-group'] as $pop_name => $location) {
            if (empty($location)) {
              form_set_error("tree-accession][species-$i][pop-group][$pop_name", "Species $i Population Group $pop_name Location: field is required.");
            }
          }
        }
      }
    }

    if (form_get_errors() and ($species_number == 1 or $form_state['values']['tree-accession']['check'] == '0')) {
      $form_state['rebuild'] = TRUE;
      $new_form = drupal_rebuild_form('tpps_main', $form_state, $form);
      $form['tree-accession']['file']['upload'] = $new_form['tree-accession']['file']['upload'];
      $form['tree-accession']['file']['columns'] = $new_form['tree-accession']['file']['columns'];
      $form['tree-accession']['file']['upload']['#id'] = "edit-tree-accession-file-upload";
      $form['tree-accession']['file']['columns']['#id'] = "edit-tree-accession-file-columns";
    }
    elseif (form_get_errors()) {
      $form_state['rebuild'] = TRUE;
      $new_form = drupal_rebuild_form('tpps_main', $form_state, $form);
      for ($i = 1; $i <= $species_number; $i++) {
        $form['tree-accession']["species-$i"]['file']['upload'] = $new_form['tree-accession']["species-$i"]['file']['upload'];
        $form['tree-accession']["species-$i"]['file']['columns'] = $new_form['tree-accession']["species-$i"]['file']['columns'];
        $form['tree-accession']["species-$i"]['file']['upload']['#id'] = "edit-tree-accession-species-$i-file-upload";
        $form['tree-accession']["species-$i"]['file']['columns']['#id'] = "edit-tree-accession-species-$i-file-columns";
      }
    }
  }

}
