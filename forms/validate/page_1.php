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
  $is_tppsc = (($form_state['build_info']['form_id'] ?? 'tpps_main') == 'tppsc_main');

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Curation form.
  if ($is_tppsc) {
    if ($form_state['submitted'] == '1') {
      $form_values = $form_state['values'];
      $doi = $form_values['doi'] ?? NULL;
      $dataset_doi = $form_values['dataset_doi'] ?? NULL;
      $organism = $form_values['organism'];
      $organism_number = $form_values['organism']['number'];
      $old_tgdr = $form_state['saved_values']['frontpage']['old_tgdr'] ?? NULL;

      if (empty($old_tgdr)) {
        if (!$doi) {
          form_set_error('doi', "DOI: field is required.");
        }
        elseif (!preg_match(tpps_doi_regex(), $doi)) {
          form_set_error('doi', 'Publication DOI: invalid format. Example DOI: "10.1111/dryad.111".');
        }
        if ($dataset_doi && !preg_match(tpps_doi_regex(), $dataset_doi)) {
          form_set_error('dataset_doi', 'Dataset DOI: invalid format. Example DOI: "10.1111/dryad.111".');
        }
      }

      if (!$form_values['publication']['primaryAuthor']) {
        form_set_error('publication][primaryAuthor', 'Primary Author: field is required.');
      }
      if (!$form_values['publication']['title']) {
        form_set_error('publication][title', 'Title of Publication: field is required.');
      }
      if (!$form_values['publication']['year']) {
        form_set_error('publication][year', 'Year of Publication: field is required.');
      }

      for ($i = 1; $i <= $organism_number; $i++) {
        $name = $organism[$i]['name'];

        if ($name == '') {
          form_set_error("organism[$i][name", "Plant Species $i: field is required.");
        }
        else {
          $name = explode(" ", $name);
          $genus = $name[0];
          $species = implode(" ", array_slice($name, 1));
          $name = implode(" ", $name);
          $empty_pattern = '/^ *$/';
          $correct_pattern = '/^[A-Z|a-z|.| |-]+$/';
          if (!isset($genus) or !isset($species) or preg_match($empty_pattern, $genus) or preg_match($empty_pattern, $species) or !preg_match($correct_pattern, $genus) or !preg_match($correct_pattern, $species)) {
            form_set_error("organism[$i][name", check_plain("Plant Species $i: please provide both genus and species in the form \"<genus> <species>\"."));
          }
        }
      }

      if (!form_get_errors()) {
        $form_state['stats']['author_count'] = $form_state['values']['publication']['secondaryAuthors']['number'] + 1;
        $form_state['stats']['species_count'] = $organism_number;
      }
    }


  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Regular form.
  else {
    if ($form_state['submitted'] == '1') {
      unset($form_state['file_info'][TPPS_PAGE_1]);

      $form_values = $form_state['values'];
      $primary_author = $form_values['primaryAuthor'];
      $publication_status = $form_values['publication']['status'];
      $second = $form_values['publication']['secondaryAuthors'];
      $second_num = $second['number'];
      $year = $form_values['publication']['year'];
      $publication_title = $form_values['publication']['title'];
      $publication_abstract = $form_values['publication']['abstract'];
      $publication_journal = $form_values['publication']['journal'];
      $organism = $form_values['organism'];
      $organism_number = $form_values['organism']['number'];

      if ($primary_author == '') {
        form_set_error('primaryAuthor', t('Primary Author: field is required.'));
      }

      if (!$publication_status) {
        form_set_error('publication][status', t('Publication Status: field is required.'));
      }

      if ($second_num > 0) {
        for ($i = 1; $i <= $second_num; $i++) {
          if (empty($second[$i])) {
            form_set_error("publication][secondaryAuthors][$i", t("Secondary Author @i: field is required.", array('@i' => $i)));
          }
        }
      }

      if (!$year) {
        form_set_error('publication][year', t('Year of Publication: field is required.'));
      }

      if ($publication_title == '') {
        form_set_error('publication][title', t('Title of Publication: field is required.'));
      }

      if ($publication_abstract == '') {
        form_set_error('publication][abstract', t('Abstract: field is required.'));
      }

      if ($publication_journal == '') {
        form_set_error('publication][journal', t('Journal: field is required.'));
      }

      for ($i = 1; $i <= $organism_number; $i++) {
        $name = $organism[$i]['name'];

        if ($name == '') {
          form_set_error("organism[$i][name", "Plant Species $i: field is required.");
        }
        else {
          $name = explode(" ", $name);
          $genus = $name[0];
          $species = implode(" ", array_slice($name, 1));
          $name = implode(" ", $name);
          $empty_pattern = '/^ *$/';
          $correct_pattern = '/^[A-Z|a-z|.| ]+$/';
          if (!isset($genus) or !isset($species) or preg_match($empty_pattern, $genus) or preg_match($empty_pattern, $species) or !preg_match($correct_pattern, $genus) or !preg_match($correct_pattern, $species)) {
            form_set_error("organism[$i][name", check_plain("Plant Species $i: please provide both genus and species in the form \"<genus> <species>\"."));
          }
        }
      }
    }
  }
}
