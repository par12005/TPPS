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
    tpps_textfield_validate($form, $form_state, function($element) use ($num) {
      if ($element['#type'] != 'textfield') {
        return FALSE;
      }
      if (count($element['#parents']) >= 3 and $element['#parents'][1] == 'secondaryAuthors') {
        return ($element['#parents'][2] <= $num);
      }
      return TRUE;
    });

    /*if (!$vals['primaryAuthor']) {
      form_set_error('primaryAuthor', 'Primary Author: field is required.');
    }
    
    if (!$vals['organization']) {
      form_set_error('organization', 'Organization: field is required.');
    }

    if (!$vals['publication']['status']) {
      form_set_error('publication][status', 'Publication Status: field is required.');
    }

    if ($second['number'] and !$second['check']) {
      for ($i = 1; $i <= $second['number']; $i++) {
        if (!$second[$i]) {
          form_set_error("publication][secondaryAuthors][$i", "Secondary Author $i: field is required.");
        }
      }
    }*/

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

    /*if (!$vals['publication']['year']) {
      form_set_error('publication][year', 'Year of Publication: field is required.');
    }

    if (!$vals['publication']['title']) {
      form_set_error('publication][title', 'Title of Publication: field is required.');
    }

    if (!$vals['publication']['abstract']) {
      form_set_error('publication][abstract', 'Abstract: field is required.');
    }

    if (!$vals['publication']['journal']) {
      form_set_error('publication][journal', 'Journal: field is required.');
    }*/

    for ($i = 1; $i <= $vals['organism']['number']; $i++) {
      $name = $vals['organism'][$i];

      if ($name == '') {
        form_set_error("organism[$i", "Tree Species $i: field is required.");
      }
      else {
        $name = explode(" ", $name);
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
