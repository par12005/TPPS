<?php

/**
 * @file
 * Defines the data integrity checks for the first page of the form.
 */

/**
 * Defines the data integrity checks for the first page of the form.
 *
 * @param array $form
 *   The form that is being validated.
 * @param array $form_state
 *   The state of the form that is being validated.
 */
function tpps_page_1_validate_form(array &$form, array &$form_state) {
  if ($form_state['submitted'] == '1') {
    unset($form_state['file_info'][TPPS_PAGE_1]);
    $vals = $form_state['values'];
    $second = $vals['publication']['secondaryAuthors'];
    $form_state['final']['authors'] = array();
    
    $num = $second['number'];
    tpps_empty_validate($form, $form_state, function($element) use ($num) {
      if (count($element['#parents']) >= 3 and $element['#parents'][1] == 'secondaryAuthors') {
        return ($element['#parents'][2] <= $num);
      }
      return TRUE;
    });

    if ($second['check']) {
      if ($second['file']) {
        $required_groups = array(
          'First Name' => array(
            'first' => array(1),
          ),
          'Last Name' => array(
            'last' => array(2),
          ),
        );

        $element = $form['publication']['secondaryAuthors']['file'];
        $groups = tpps_file_validate_columns($form_state, $required_groups, $element);
        if (!form_get_errors()) {
          $file = file_load($second['file']);
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
          $form_state['file_info'][TPPS_PAGE_1][$file->fid] = 'Secondary_Authors.xlsx';
        }
      }
      if (!$second['file']) {
        form_set_error('publication][secondaryAuthors][file', 'Secondary Authors file: field is required.');
      }
    }

    for ($i = 1; $i <= $vals['organism']['number']; $i++) {
      if (!empty($vals['organism'][$i])) {
        $name = explode(" ", $vals['organism'][$i]);
        $genus = $name[0];
        $species = implode(" ", array_slice($name, 1));
        $name = implode(" ", $name);
        $empty_pattern = '/^ *$/';
        $correct_pattern = '/^[A-Z|a-z|.| ]+$/';
        if (!isset($genus) or !isset($species) or preg_match($empty_pattern, $genus) or preg_match($empty_pattern, $species) or !preg_match($correct_pattern, $genus) or !preg_match($correct_pattern, $species)) {
          form_set_error("organism[$i", check_plain("Tree Species $i: please provide both genus and species in the form \"<genus> <species>\"."));
        }
      }
    }
  }
}
