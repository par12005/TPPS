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
      //$old_tgdr = $form_state['saved_values']['frontpage']['old_tgdr'] ?? NULL;
      // DOI.
      $doi = $form_values['doi'] ?? NULL;
      $dataset_doi = $form_values['dataset_doi'] ?? NULL;
      // Publication.
      $primary_author = $form_values['primaryAuthor'] ?? NULL;
      $publication_status = $form_values['publication']['status'] ?? NULL;

      $second = $form_values['publication']['secondaryAuthors'] ?? NULL;
      $second_num = $second['number'] ?? NULL;
      // Organism's data.
      $organism = $form_values['organism'] ?? NULL;
      $organism_number = $form_values['organism']['number'] ?? NULL;
      // Publication.
      tpps_is_required_field_empty($form_state, ['publication', 'status']);
      // Note:
      // $form_state['saved_values][PAGE_1]['primaryAuthor']
      // but $form['publication']['primaryAuthor'];
      tpps_is_required_field_empty($form_state, ['publication', 'primaryAuthor']);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      module_load_include('inc', 'tpps', 'includes/manage_doi');
      if ($publication_status == 'Published') {
        // 'Publication DOI' field is required (even for existing studies).
        if (!tpps_is_required_field_empty($form_state, ['publication', 'doi'])) {
          if (!preg_match(tpps_doi_regex(), $doi)) {
            form_set_error('doi', 'Publication DOI: invalid format. '
              . 'Example DOI: "10.1111/dryad.111".'
            );
          }
        }
        // 'Dataset DOI' is optional.
        if ($dataset_doi && !preg_match(tpps_doi_regex(), $dataset_doi)) {
          form_set_error('dataset_doi', 'Dataset DOI: invalid format. '
            . 'Example DOI: "10.1111/dryad.111".'
          );
        }
        // Required Publication Extra Fields.
        foreach (['year', 'title', 'abstract', 'journal'] as $name) {
          tpps_is_required_field_empty($form_state,
            ['publication', $name]
          );
        }
      }
      else {
        // We need to clear all not visible fields to avoid any problems on
        // study processing. This is possible when first set 'Published'
        // status and filled DOI/Extra Publication fields and then publication
        // status field changed to something different from 'Published'.
        // Note: Authors and Organisms fields do not need those extra step.
        // Clear Publication Extra Fields.
        foreach (['year', 'title', 'abstract', 'journal'] as $name) {
          $form_state['values']['publication'][$name] = NULL;
          // @TODO [VS] Minor. Check if this is required.
          $form_state['saved_values']['publication'][$name] = NULL;
        }
        // Clear DOI fields.
        $form_state['values']['doi'] = NULL;
        $form_state['saved_values']['dataset_doi'] = NULL;
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Organisms.
      for ($i = 1; $i <= $organism_number; $i++) {
        $name = $organism[$i]['name'];
        if ($name == '') {
          // @TODO Check why 'Name' field not highlighted when validation failed.
          // form_set_error("organism[$i][name", "Plant Species $i: field is required.");
          // This is workaround. Just highlight all fields in 'Organism' fieldset.
          form_set_error("organism", "Plant Species $i: field is required.");
        }
        else {
          $name = explode(" ", $name);
          $genus = $name[0];
          $species = implode(" ", array_slice($name, 1));
          $name = implode(" ", $name);
          $empty_pattern = '/^ *$/';
          $correct_pattern = '/^[A-Z|a-z|.| |-]+$/';
          if (
            !isset($genus)
            or !isset($species)
            or preg_match($empty_pattern, $genus)
            or preg_match($empty_pattern, $species)
            or !preg_match($correct_pattern, $genus)
            or !preg_match($correct_pattern, $species)
          ) {
            form_set_error("organism[$i][name",
              check_plain("Plant Species $i: please provide both genus "
              . "and species in the form \"<genus> <species>\".")
            );
          }
        }
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      if (!form_get_errors()) {
        $form_state['stats']['author_count']
          = $form_state['values']['publication']['secondaryAuthors']['number'] + 1;
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
          $name = explode(" ", trim($name));
          $genus = $name[0];
          $species = implode(" ", array_slice($name, 1));
          $name = implode(" ", $name);
          $empty_pattern = '/^ *$/';
          $correct_pattern = '/^[A-Z|a-z|.| ]+$/';
          if (
            !isset($genus)
            or !isset($species)
            or preg_match($empty_pattern, $genus)
            or preg_match($empty_pattern, $species)
            or !preg_match($correct_pattern, $genus)
            or !preg_match($correct_pattern, $species)
          ) {
            form_set_error("organism[$i][name",
              t('Plant Species @number: please provide both genus and species '
                . 'in the form "[genus] [species]" separated with space.',
                ['@number' => $i]
              )
            );
          }
        }
      }
    }
  }
}
