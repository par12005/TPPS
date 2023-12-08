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
  if ($form_state['submitted'] != 1) {
    return;
  }
  unset($form_state['file_info'][TPPS_PAGE_3]);

  if (!empty($form_state['values']['study_location'])) {
    if (!$form_state['values']['study_location']['type']) {
      form_set_error('study_location][type', t('Location Format: field is required.'));
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

    $fid = $values['file'] ?? '';
    // Check if file was uploaded.
    if (empty($fid)) {
      form_set_error("tree-accession][species-$i][file",
        t('Plant Accession File: Field is required.')
      );
    }
    else {
      $file = file_load($fid);
      // Check if file has zero length (empty).
      if ($file->filesize == 0) {
        form_set_error("tree-accession][species-$i][file",
          t('Plant Accession File: File is empty.')
        );
        break;
      }
      // Non-empty file.
      else {
        $required_groups = [
          'Tree Id' => [
            'id' => [TPPS_COLUMN_PLANT_IDENTIFIER],
          ],
          'Location (latitude/longitude or country/state or population group)' => [
           // @TODO [VS] Replace 'magic' numbers with constant with more
            // descriptive names. See tpps_page_3_create_form() for columns.
            'approx' => [TPPS_COLUMN_COUNTRY, TPPS_COLUMN_STATE],
            'gps' => [TPPS_COLUMN_LATITUDE, TPPS_COLUMN_LONGITUDE],
            'pop_group' => [TPPS_COLUMN_POPULATION_GROUP],
          ],
        ];

        if (!$multi_file and $species_number > 1) {
          $required_groups['Genus and Species'] = [
            'separate' => [TPPS_COLUMN_GENUS, TPPS_COLUMN_SPECIES],
            'combined' => [TPPS_COLUMN_GENUS_AND_SPECIES],
          ];
        }

        $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

        if (gettype($values['pop-group']) === 'array') {
          foreach ($values['pop-group'] as $pop_name => $location) {
            if (empty($location)) {
              form_set_error(
                "tree-accession][species-$i][pop-group][$pop_name",
                "Population Group $pop_name Location: field is required."
              );
            }
          }
        }

        // [VS] #8669py308
        if (
          ($values['location_accuracy'] ?? NULL) == 'descriptive_place'
          && empty($values['descriptive_place'])
        ) {
          form_set_error("tree-accession][species-$i][descriptive_place",
            t("Descriptive Place: field is required."));
        }

        if (
          ($values['location_accuracy'] ?? NULL) == 'approximate'
          && empty($values['coord_precision'])
        ) {
          form_set_error("tree-accession][species-$i][coord_precision",
            t("Coordinates accuracy: field is required."));
        }
        // [/VS] #8669py308
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        if (!form_get_errors()) {
          $options = [
            'no_header' => !empty($values['file-no-header']),
            'loc_options' =>
              $required_groups['Location (latitude/longitude or country/state or population group)'],
            'id_col' => $groups['Tree Id']['1'],
            'loc_cols' => $groups['Location (latitude/longitude or country/state or population group)'],
            'empty' => $values['file-empty'],
            'org_num' => $i,
          ];
          tpps_file_iterator($fid 'tpps_accession_valid_locations', $options);
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        if (!form_get_errors() and (!$multi_file and $species_number > 1)) {
          $options = array(
            'no_header' => !empty($values['file-no-header']),
            'species_options' => $required_groups['Genus and Species'],
            'id_col' => $groups['Tree Id']['1'],
            'species_cols' => $groups['Genus and Species'],
            'empty' => $values['file-empty'],
            'org_num' => $i,
            'page_1_species' => $form_state['saved_values'][TPPS_PAGE_1]['organism'],
          );
          tpps_file_iterator($fid, 'tpps_accession_valid_species', $options);
        }
        tpps_preserve_valid_file($form_state, $fid, $i, "Plant_Accession");
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
      $form['tree-accession']["species-$i"]['file']['upload']
        = $new_form['tree-accession']["species-$i"]['file']['upload'];
      $form['tree-accession']["species-$i"]['file']['columns']
        = $new_form['tree-accession']["species-$i"]['file']['columns'];
      $form['tree-accession']["species-$i"]['file']['upload']['#id']
        = "edit-tree-accession-species-$i-file-upload";
      $form['tree-accession']["species-$i"]['file']['columns']['#id']
        = "edit-tree-accession-species-$i-file-columns";
      if (!$multi_file) {
        break;
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
function tpps_accession_valid_locations($row, array &$options) {
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
        $params = [
          '@plant_id' => $row[$id_name] ?? NULL,
          '@value' => $row[$location_columns[$column]] ?? NULL,
        ];
        if (empty($row[$location_columns[$column]]) or $row[$location_columns[$column]] == $empty) {
          $valid_combination = FALSE;
          $reason = t('Some location information is missing '
            . 'for plant "@plant_id".', $params
          );
        }
        elseif ($type == 'gps') {
          // Check if coordinates is not out of range.
          // @todo Validate also other values.
          // $column is an index in the list of column roles in the header
          // of the table. See tpps_page_3_create_form() for $column_options.
          if ($column == TPPS_COLUMN_LATITUDE
            && abs($row[$location_columns[$column]]) > 90
          ) {
            $valid_combination = FALSE;
            $reason = t('Latitude must be within the range [-90, 90] '
              . 'for plant "@plant_id" which has "@value".', $params
            );
          }
          elseif ($column == TPPS_COLUMN_LONGITUDE
            && abs($row[$location_columns[$column]]) > 180
          ) {
            $valid_combination = FALSE;
            $reason = t('Longitude must be within the range [-180, 180]. '
              . 'for plant "@plant_id" which has "@value".', $params
            );
          }
        }
      }
      if ($valid_combination) {
        $valid_row = TRUE;
        break;
      }
    }
    if (!$valid_row) {
      form_set_error(
        "tree-accession-species-$org_num-file-{$row[$id_name]}",
        'Plant Accession file: ' . $reason
      );
    }
  }
}

/**
 * Processes a single row of a plant accession file.
 *
 * This function validates that the accession file has valid/complete species
 * information for each plant. This function is meant to be used with
 * tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_accession_valid_species($row, array &$options) {
  $id_name = $options['id_col'];
  $species_options = $options['species_options'];
  $species_columns = $options['species_cols'];
  $species_type = $species_columns['#type'];
  $org_num = $options['org_num'];
  $organisms = $options['page_1_species'];

  if (!empty($row[$id_name])) {
    $valid_row = FALSE;

    if ($species_type == 'separate') {
      $species = array();
      foreach ($species_options[$species_type] as $column) {
        $species[] = $row[$species_columns[$column]];
      }
      $species = implode(' ', $species);
    }
    else {
      $species = $row[$species_columns[current($species_options[$species_type])]];
    }

    for ($i = 1; $i <= $organisms['number']; $i++) {
      if ($species == $organisms[$i]['name']) {
        $valid_row = TRUE;
        break;
      }
    }

    if (!$valid_row) {
      form_set_error(
        'tree-accession-species-' . $org_num . '-file-' . $row[$id_name],
        t('Plant Accession file: Some species information is invalid for'
          . ' plant "@plaint_id". The species name, "@species_name", does '
          . 'not match any species name supplied on the Author and Species '
          . 'information page. Please correct the file or add the correct '
          . 'species name.',
          [
            '@plant_id' => $row[$id_name],
            '@species_name' => $species,
          ]
        )
      );
    }
  }
}
