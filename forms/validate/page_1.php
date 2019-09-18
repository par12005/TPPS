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
  // For testing only.
  /*foreach($form_state['values'] as $key => $value){
  print_r($key . " => " . $value . ";<br>");
  }*/

  if ($form_state['submitted'] == '1') {
    unset($form_state['file_info'][TPPS_PAGE_1]);

    $form_values = $form_state['values'];
    $primary_author = $form_values['primaryAuthor'];
    $organization = $form_values['organization'];
    $publication_status = $form_values['publication']['status'];
    $secondary_authors_number = $form_values['publication']['secondaryAuthors']['number'];
    $secondary_authors_array = array_slice($form_values['publication']['secondaryAuthors'], 3, 30, TRUE);
    $secondary_authors_file = $form_values['publication']['secondaryAuthors']['file'];
    $secondary_authors_check = $form_values['publication']['secondaryAuthors']['check'];
    $year = $form_values['publication']['year'];
    $publication_title = $form_values['publication']['title'];
    $publication_abstract = $form_values['publication']['abstract'];
    $publication_journal = $form_values['publication']['journal'];
    $organism = $form_values['organism'];
    $organism_number = $form_values['organism']['number'];

    if ($primary_author == '') {
      form_set_error('primaryAuthor', 'Primary Author: field is required.');
    }

    if ($organization == '') {
      form_set_error('organization', 'Organization: field is required.');
    }

    if (!$publication_status) {
      form_set_error('publication][status', 'Publication Status: field is required.');
    }

    if ($secondary_authors_number > 0 and !$secondary_authors_check) {
      for ($i = 1; $i <= $secondary_authors_number; $i++) {
        if ($secondary_authors_array[$i] == '') {
          form_set_error("publication][secondaryAuthors][$i", "Secondary Author $i: field is required.");
        }
      }
    }
    elseif ($secondary_authors_check) {
      $file_element = $form_values['publication']['secondaryAuthors']['file'];

      if ($secondary_authors_file) {
        $required_groups = array(
          'First Name' => array(
            'first' => array(1),
          ),
          'Last Name' => array(
            'last' => array(2),
          ),
        );

        $file_element = $form['publication']['secondaryAuthors']['file'];
        $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

        if (!form_get_errors()) {
          // Preserve file if it is valid.
          $file = file_load($form_state['values']['publication']['secondaryAuthors']['file']);
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
          $form_state['file_info'][TPPS_PAGE_1][$file->fid] = 'Secondary_Authors';
        }
      }
      else {
        form_set_error('publication][secondaryAuthors][file', 'Secondary Authors file: field is required.');
      }
    }

    if (!$year) {
      form_set_error('publication][year', 'Year of Publication: field is required.');
    }

    if ($publication_title == '') {
      form_set_error('publication][title', 'Title of Publication: field is required.');
    }

    if ($publication_abstract == '') {
      form_set_error('publication][abstract', 'Abstract: field is required.');
    }

    if ($publication_journal == '') {
      form_set_error('publication][journal', 'Journal: field is required.');
    }

    for ($i = 1; $i <= $organism_number; $i++) {
      $name = $organism[$i];

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
