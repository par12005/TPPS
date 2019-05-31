<?php

/**
 * @file
 * Defines function tpps_submit_all and its helper functions.
 *
 * The functions defined in this file do not actually submit the genotype,
 * phenotype, or environmental data collected from page 4. That data is instead
 * submitted through a Tripal job due to the size of the data.
 */

/**
 * Creates a record for the project and calls the submission helper functions.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 */
function tpps_submit_all($accession) {

  $form_state = tpps_load_submission($accession);
  $values = $form_state['saved_values'];
  $firstpage = $values[TPPS_PAGE_1];
  $file_rank = 0;

  $project_id = tpps_chado_insert_record('project', array(
    'name' => $firstpage['publication']['title'],
    'description' => $firstpage['publication']['abstract'],
  ));

  $organism_ids = tpps_submit_page_1($form_state, $project_id, $file_rank);

  tpps_submit_page_2($form_state, $project_id, $file_rank);

  tpps_submit_page_3($form_state, $project_id, $file_rank, $organism_ids);

  tpps_submit_page_4($form_state, $project_id, $file_rank, $organism_ids);

  tpps_update_submission($form_state, array('status' => 'Approved'));

  // For simplicity and efficiency, all fourth page submissions take place in
  // the TPPS File Parsing Tripal Job.
  $args = array($form_state['accession']);
  $jid = tripal_add_job("TPPS File Parsing - {$form_state['accession']}", 'tpps', 'tpps_file_parsing', $args, $uid, 10, $includes, TRUE);
  $form_state['job_id'] = $jid;
  tpps_update_submission($form_state);
}

/**
 * Submits Publication and Species data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 * @param int $project_id
 *   The project_id of the project that the data will reference in the database.
 * @param int $file_rank
 *   The rank number for files associated with the project.
 *
 * @return array
 *   An array of the organism_ids associated with the project.
 */
function tpps_submit_page_1(array &$form_state, $project_id, &$file_rank) {

  $dbxref_id = $form_state['dbxref_id'];
  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];

  tpps_chado_insert_record('project_dbxref', array(
    'project_id' => $project_id,
    'dbxref_id' => $dbxref_id,
  ));

  tpps_chado_insert_record('contact', array(
    'name' => $firstpage['primaryAuthor'],
    'type_id' => array(
      'cv_id' => array(
        'name' => 'tripal_contact',
      ),
      'name' => 'Person',
      'is_obsolete' => 0,
    ),
  ));

  $author_string = $firstpage['primaryAuthor'];
  if ($firstpage['publication']['secondaryAuthors']['check'] == 0 and $firstpage['publication']['secondaryAuthors']['number'] != 0) {

    for ($i = 1; $i <= $firstpage['publication']['secondaryAuthors']['number']; $i++) {
      tpps_chado_insert_record('contact', array(
        'name' => $firstpage['publication']['secondaryAuthors'][$i],
        'type_id' => array(
          'cv_id' => array(
            'name' => 'tripal_contact',
          ),
          'name' => 'Person',
          'is_obsolete' => 0,
        ),
      ));
      $author_string .= "; {$firstpage['publication']['secondaryAuthors'][$i]}";
    }
  }
  elseif ($firstpage['publication']['secondaryAuthors']['check'] != 0) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'schema',
        ),
        'name' => 'url',
        'is_obsolete' => 0,
      ),
      'value' => file_create_url(file_load($firstpage['publication']['secondaryAuthors']['file'])->uri),
      'rank' => $file_rank,
    ));

    $file = file_load($firstpage['publication']['secondaryAuthors']['file']);
    $location = drupal_realpath($file->uri);
    $content = tpps_parse_xlsx($location);
    $column_vals = $firstpage['publication']['secondaryAuthors']['file-columns'];

    foreach ($column_vals as $col => $val) {
      if ($val == '1') {
        $first_name = $col;
      }
      if ($val == '2') {
        $last_name = $col;
      }
      if ($val == '3') {
        $middle_initial = $col;
      }
    }

    for ($i = 0; $i < count($content) - 1; $i++) {
      tpps_chado_insert_record('contact', array(
        'name' => "{$content[$i][$last_name]}, {$content[$i][$first_name]} {$content[$i][$middle_initial]}",
        'type_id' => array(
          'cv_id' => array(
            'name' => 'tripal_contact',
          ),
          'name' => 'Person',
          'is_obsolete' => 0,
        ),
      ));
      $author_string .= "; {$content[$i][$last_name]}, {$content[$i][$first_name]} {$content[$i][$middle_initial]}";
    }
    $file->status = FILE_STATUS_PERMANENT;
    $file = file_save($file);
    $file_rank++;
  }

  $publication_id = tpps_chado_insert_record('pub', array(
    'title' => $firstpage['publication']['title'],
    'series_name' => $firstpage['publication']['journal'],
    'type_id' => array(
      'cv_id' => array(
        'name' => 'tripal_pub',
      ),
      'name' => 'Journal Article',
      'is_obsolete' => 0,
    ),
    'pyear' => $firstpage['publication']['year'],
    'uniquename' => "$author_string {$firstpage['publication']['title']}. {$firstpage['publication']['journal']}; {$firstpage['publication']['year']}",
  ));

  tpps_chado_insert_record('project_pub', array(
    'project_id' => $project_id,
    'pub_id' => $publication_id,
  ));

  tpps_chado_insert_record('contact', array(
    'name' => $firstpage['organization'],
    'type_id' => array(
      'cv_id' => array(
        'name' => 'tripal_contact',
      ),
      'name' => 'Organization',
      'is_obsolete' => 0,
    ),
  ));

  $names = explode(" ", $firstpage['primaryAuthor']);
  $first_name = $names[0];
  $last_name = implode(" ", array_slice($names, 1));

  tpps_chado_insert_record('pubauthor', array(
    'pub_id' => $publication_id,
    'rank' => '0',
    'surname' => $last_name,
    'givennames' => $first_name,
  ));

  if ($firstpage['publication']['secondaryAuthors']['check'] == 0 and $firstpage['publication']['secondaryAuthors']['number'] != 0) {
    for ($i = 1; $i <= $firstpage['publication']['secondaryAuthors']['number']; $i++) {
      $names = explode(" ", $firstpage['publication']['secondaryAuthors'][$i]);
      $first_name = $names[0];
      $last_name = implode(" ", array_slice($names, 1));
      tpps_chado_insert_record('pubauthor', array(
        'pub_id' => $publication_id,
        'rank' => "$i",
        'surname' => $last_name,
        'givennames' => $first_name,
      ));
    }
  }
  elseif ($firstpage['publication']['secondaryAuthors']['check'] != 0) {

    $file = file_load($firstpage['publication']['secondaryAuthors']['file']);
    $location = drupal_realpath($file->uri);
    $content = tpps_parse_xlsx($location);
    $column_vals = $firstpage['publication']['secondaryAuthors']['file-columns'];
    $groups = $firstpage['publication']['secondaryAuthors']['file-groups'];

    if (!empty($firstpage['publication']['secondaryAuthors']['file-no-header'])) {
      tpps_content_no_header($content);
    }

    $first_name = $groups['First Name']['1'];
    $last_name = $groups['Last Name']['2'];

    foreach ($column_vals as $col => $val) {
      if ($val == '3') {
        $middle_initial = $col;
        break;
      }
    }

    for ($i = 0; $i < count($content) - 1; $i++) {
      $rank = $i + 1;
      tpps_chado_insert_record('pubauthor', array(
        'pub_id' => $publication_id,
        'rank' => "$rank",
        'surname' => $content[$i][$last_name],
        'givennames' => $content[$i][$first_name] . " " . $content[$i][$middle_initial],
      ));
    }
  }

  $organism_ids = array();
  $organism_number = $firstpage['organism']['number'];

  for ($i = 1; $i <= $organism_number; $i++) {
    $parts = explode(" ", $firstpage['organism'][$i]);
    $genus = $parts[0];
    $species = implode(" ", array_slice($parts, 1));
    if (isset($parts[2]) and ($parts[2] == 'var.' or $parts[2] == 'subsp.')) {
      $infra = implode(" ", array_slice($parts, 2));
    }
    else {
      $infra = NULL;
    }
    $organism_ids[$i] = tpps_chado_insert_record('organism', array(
      'genus' => $genus,
      'species' => $species,
      'infraspecific_name' => $infra,
    ));
    tpps_chado_insert_record('project_organism', array(
      'organism_id' => $organism_ids[$i],
      'project_id' => $project_id,
    ));
  }
  return $organism_ids;
}

/**
 * Submits Study Design data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 * @param int $project_id
 *   The project_id of the project that the data will reference in the database.
 * @param int $file_rank
 *   The rank number for files associated with the project.
 */
function tpps_submit_page_2(array &$form_state, $project_id, &$file_rank) {

  $secondpage = $form_state['saved_values'][TPPS_PAGE_2];

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $project_id,
    'type_id' => array(
      'cv_id' => array(
        'name' => 'local',
      ),
      'name' => 'study_start',
      'is_obsolete' => 0,
    ),
    'value' => $secondpage['StartingDate']['month'] . " " . $secondpage['StartingDate']['year'],
  ));

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $project_id,
    'type_id' => array(
      'cv_id' => array(
        'name' => 'local',
      ),
      'name' => 'study_end',
      'is_obsolete' => 0,
    ),
    'value' => $secondpage['EndingDate']['month'] . " " . $secondpage['EndingDate']['year'],
  ));

  if ($secondpage['study_location']['type'] !== '2') {
    $standard_coordinate = explode(',', tpps_standard_coord($secondpage['study_location']['coordinates']));
    $latitude = $standard_coordinate[0];
    $longitude = $standard_coordinate[1];

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'gps_latitude',
        'is_obsolete' => 0,
      ),
      'value' => $latitude,
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'gps_longitude',
        'is_obsolete' => 0,
      ),
      'value' => $longitude,
    ));
  }
  else {
    $location = $secondpage['study_location']['custom'];

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'experiment_location',
        'is_obsolete' => 0,
      ),
      'value' => $location,
    ));
  }

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $project_id,
    'type_id' => array(
      'cv_id' => array(
        'name' => 'local',
      ),
      'name' => 'association_results_type',
      'is_obsolete' => 0,
    ),
    'value' => $secondpage['data_type'],
  ));

  $studytype_options = array(
    0 => '- Select -',
    1 => 'Natural Population (Landscape)',
    2 => 'Growth Chamber',
    3 => 'Greenhouse',
    4 => 'Experimental/Common Garden',
    5 => 'Plantation',
  );

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $project_id,
    'type_id' => array(
      'cv_id' => array(
        'name' => 'local',
      ),
      'name' => 'study_type',
      'is_obsolete' => 0,
    ),
    'value' => $studytype_options[$secondpage['study_type']],
  ));

  if (!empty($secondpage['study_info']['season'])) {
    $seasons = implode($secondpage['study_info']['season']);

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'assession_season',
        'is_obsolete' => 0,
      ),
      'value' => $seasons,
    ));
  }

  if (!empty($secondpage['study_info']['assessions'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'assession_number',
        'is_obsolete' => 0,
      ),
      'value' => $secondpage['study_info']['assessions'],
    ));
  }

  if (!empty($secondpage['study_info']['temp'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'temperature_high',
        'is_obsolete' => 0,
      ),
      'value' => $secondpage['study_info']['temp']['high'],
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'temperature_low',
        'is_obsolete' => 0,
      ),
      'value' => $secondpage['study_info']['temp']['low'],
    ));
  }

  $types = array(
    'co2',
    'humidity',
    'light',
    'salinity',
  );

  foreach ($types as $type) {
    if (!empty($secondpage['study_info'][$type])) {
      $set = $secondpage['study_info'][$type];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => "{$type}_control",
          'is_obsolete' => 0,
        ),
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => "{$type}_level",
            'is_obsolete' => 0,
          ),
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => "{$type}_level",
            'is_obsolete' => 0,
          ),
          'value' => $set['uncontrolled'],
        ));
      }
    }
  }

  if (!empty($secondpage['study_info']['rooting'])) {
    $root = $secondpage['study_info']['rooting'];

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'rooting_type',
        'is_obsolete' => 0,
      ),
      'value' => $root['option'],
    ));

    if ($root['option'] == 'Soil') {
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'soil_type',
          'is_obsolete' => 0,
        ),
        'value' => ($root['soil']['type'] == 'Other') ? $root['soil']['other'] : $root['soil']['type'],
      ));

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'soil_container',
          'is_obsolete' => 0,
        ),
        'value' => $root['soil']['container'],
      ));
    }

    if (!empty($secondpage['study_info']['rooting']['ph'])) {
      $set = $secondpage['study_info']['rooting']['ph'];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => "pH_control",
          'is_obsolete' => 0,
        ),
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => "pH_level",
            'is_obsolete' => 0,
          ),
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => "pH_level",
            'is_obsolete' => 0,
          ),
          'value' => $set['uncontrolled'],
        ));
      }
    }

    $description = FALSE;
    $rank = 0;
    foreach ($root['treatment'] as $value) {
      if (!$description) {
        if ($value) {
          $record_next = TRUE;
        }
        else {
          $record_next = FALSE;
        }
        $description = TRUE;
        continue;
      }
      elseif ($record_next) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'treatment',
            'is_obsolete' => 0,
          ),
          'value' => $value,
          'rank' => $rank,
        ));
        $rank++;
      }
      $description = FALSE;
    }
  }

  if (!empty($form_state['values']['study_info']['irrigation'])) {
    $irrigation = $form_state['values']['study_info']['irrigation'];
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'irrigation_type',
        'is_obsolete' => 0,
      ),
      'value' => ($irrigation['option'] == 'Other') ? $irrigation['other'] : $irrigation['option'],
    ));
  }

  if (!empty($form_state['values']['study_info']['biotic_env']['option'])) {
    foreach ($form_state['values']['study_info']['biotic_env']['option'] as $key => $check) {
      if ($check) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'biotic_environment',
            'is_obsolete' => 0,
          ),
          'value' => ($key == 'Other') ? $form_state['values']['study_info']['biotic_env']['other'] : $key,
        ));
      }
    }
  }

  if (!empty($form_state['values']['study_info']['treatment']) and $form_state['values']['study_info']['treatment']['check']) {
    $description = FALSE;
    $rank = 0;

    foreach ($treatment as $field => $value) {
      if ($field != 'check') {
        if (!$description) {
          $description = TRUE;
          $record_next = $value;
          continue;
        }
        elseif ($record_next) {
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'treatment',
              'is_obsolete' => 0,
            ),
            'value' => $value,
            'rank' => $rank,
          ));
          $rank++;
        }
        $description = FALSE;
      }
    }
  }
}

/**
 * Submits Tree Accession data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 * @param int $project_id
 *   The project_id of the project that the data will reference in the database.
 * @param int $file_rank
 *   The rank number for files associated with the project.
 * @param array $organism_ids
 *   The array of organism_ids associated with the project.
 */
function tpps_submit_page_3(array &$form_state, $project_id, &$file_rank, array $organism_ids) {

  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];
  $organism_number = $firstpage['organism']['number'];

  $stock_ids = array();

  if ($organism_number == '1' or $thirdpage['tree-accession']['check'] == 0) {
    // Single file.
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'schema',
        ),
        'name' => 'url',
        'is_obsolete' => 0,
      ),
      'value' => file_create_url(file_load($thirdpage['tree-accession']['file'])->uri),
      'rank' => $file_rank,
    ));

    $file = file_load($thirdpage['tree-accession']['file']);
    $location = drupal_realpath($file->uri);
    $content = tpps_parse_xlsx($location);
    $column_vals = $thirdpage['tree-accession']['file-columns'];
    $groups = $thirdpage['tree-accession']['file-groups'];

    foreach ($column_vals as $col => $val) {
      if ($val == '8') {
        $county_col_name = $col;
      }
      if ($val == '9') {
        $district_col_name = $col;
      }
    }

    $id_col_accession_name = $groups['Tree Id']['1'];

    if ($organism_number == '1') {
      // Only one species.
      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        $stock_ids[$tree_id] = tpps_chado_insert_record('stock', array(
          'uniquename' => $tree_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'obi',
            ),
            'name' => 'organism',
            'is_obsolete' => 0,
          ),
          'organism_id' => $organism_ids[1],
        ));
      }
    }
    else {
      // Multiple species in one tree accession file -> users must define
      // species and genus columns get genus/species column.
      if ($groups['Genus and Species']['#type'] == 'separate') {
        $genus_col_name = $groups['Genus and Species']['6'];
        $species_col_name = $groups['Genus and Species']['7'];
      }
      else {
        $org_col_name = $groups['Genus and Species']['10'];
      }

      // Parse file.
      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        for ($j = 1; $j <= $organism_number; $j++) {
          // Match genus and species to genus and species given on page 1.
          if ($groups['Genus and Species']['#type'] == 'separate') {
            $genus_full_name = "{$content[$i][$genus_col_name]} {$content[$i][$species_col_name]}";
          }
          else {
            $genus_full_name = "{$content[$i][$org_col_name]}";
          }

          if ($firstpage['organism'][$j] == $genus_full_name) {
            // Obtain organism id from matching species.
            $id = $organism_ids[$j];
            break;
          }
        }

        // Create record with the new id.
        $stock_ids[$tree_id] = tpps_chado_insert_record('stock', array(
          'uniquename' => $tree_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'obi',
            ),
            'name' => 'organism',
            'is_obsolete' => 0,
          ),
          'organism_id' => $id,
        ));
      }
    }

    if ($groups['Location (latitude/longitude or country/state or population group)']['#type'] == 'gps') {
      $lat_name = $groups['Location (latitude/longitude or country/state or population group)']['4'];
      $long_name = $groups['Location (latitude/longitude or country/state or population group)']['5'];

      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        $stock_id = $stock_ids[$tree_id];

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'gps_latitude',
            'is_obsolete' => 0,
          ),
          'value' => $content[$i][$lat_name],
        ));

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'gps_longitude',
            'is_obsolete' => 0,
          ),
          'value' => $content[$i][$long_name],
        ));
      }
    }
    elseif ($groups['Location (latitude/longitude or country/state or population group)']['#type'] == 'approx') {
      $country_col_name = $groups['Location (latitude/longitude or country/state or population group)']['2'];
      $state_col_name = $groups['Location (latitude/longitude or country/state or population group)']['3'];

      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        $stock_id = $stock_ids[$tree_id];

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'tripal_contact',
            ),
            'name' => 'Country',
            'is_obsolete' => 0,
          ),
          'value' => $content[$i][$country_col_name],
        ));

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'tripal_contact',
            ),
            'name' => 'State',
            'is_obsolete' => 0,
          ),
          'value' => $content[$i][$state_col_name],
        ));

        if (isset($county_col_name)) {
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'county',
              'is_obsolete' => 0,
            ),
            'value' => $content[$i][$county_col_name],
          ));
        }

        if (isset($district_col_name)) {
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'district',
              'is_obsolete' => 0,
            ),
            'value' => $content[$i][$district_col_name],
          ));
        }
      }
    }
    else {
      $pop_group_name = $groups['Location (latitude/longitude or country/state or population group)']['12'];

      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        $stock_id = $stock_ids[$tree_id];

        $loc = $thirdpage['tree-accession']['pop-group'][$content[$i][$pop_group_name]];
        $coord = tpps_standard_coord($loc);

        if ($coord) {
          $parts = explode(',', $coord);
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'gps_latitude',
              'is_obsolete' => 0,
            ),
            'value' => $parts[0],
          ));

          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'gps_longitude',
              'is_obsolete' => 0,
            ),
            'value' => $parts[1],
          ));
        }
        else {
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'nd_geolocation_property',
              ),
              'name' => 'Location',
              'is_obsolete' => 0,
            ),
            'value' => $loc,
          ));
        }
      }
    }

    $file->status = FILE_STATUS_PERMANENT;
    $file = file_save($file);
    $file_rank++;
  }
  else {
    // Multiple files, sorted by species.
    for ($i = 1; $i <= $organism_number; $i++) {
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
        'type_id' => array(
          'cv_id' => array(
            'name' => 'schema',
          ),
          'name' => 'url',
          'is_obsolete' => 0,
        ),
        'value' => drupal_realpath(file_load($thirdpage['tree-accession']["species-$i"]['file'])->uri),
        'rank' => $file_rank,
      ));

      $file = file_load($thirdpage['tree-accession']["species-$i"]['file']);
      $location = drupal_realpath($file->uri);
      $content = tpps_parse_xlsx($location);
      $column_vals = $thirdpage['tree-accession']["species-$i"]['file-columns'];
      $groups = $thirdpage['tree-accession']["species-$i"]['file-groups'];

      $id_col_accession_name = $groups['Tree Id']['1'];

      foreach ($column_vals as $col => $val) {
        if ($val == '8') {
          $county_col_name = $col;
        }
        if ($val == '9') {
          $district_col_name = $col;
        }
      }

      for ($j = 0; $j < count($content) - 1; $j++) {
        $tree_id = $content[$j][$id_col_accession_name];
        $stock_ids[$tree_id] = tpps_chado_insert_record('stock', array(
          'uniquename' => $tree_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'obi',
            ),
            'name' => 'organism',
            'is_obsolete' => 0,
          ),
          'organism_id' => $organism_ids[$i],
        ));

        if ($groups['Location (latitude/longitude or country/state or population group)']['#type'] == 'gps') {
          $lat_name = $groups['Location (latitude/longitude or country/state or population group)']['4'];
          $long_name = $groups['Location (latitude/longitude or country/state or population group)']['5'];

          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_ids[$tree_id],
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'gps_latitude',
              'is_obsolete' => 0,
            ),
            'value' => $content[$j][$lat_name],
          ));

          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_ids[$tree_id],
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'gps_longitude',
              'is_obsolete' => 0,
            ),
            'value' => $content[$j][$long_name],
          ));
        }
        elseif ($groups['Location (latitude/longitude or country/state or population group)']['#type'] == 'approx') {
          $country_col_name = $groups['Location (latitude/longitude or country/state or population group)']['2'];
          $state_col_name = $groups['Location (latitude/longitude or country/state or population group)']['3'];

          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'tripal_contact',
              ),
              'name' => 'Country',
              'is_obsolete' => 0,
            ),
            'value' => $content[$j][$country_col_name],
          ));

          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'tripal_contact',
              ),
              'name' => 'State',
              'is_obsolete' => 0,
            ),
            'value' => $content[$j][$state_col_name],
          ));

          if (isset($county_col_name)) {
            tpps_chado_insert_record('stockprop', array(
              'stock_id' => $stock_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'local',
                ),
                'name' => 'county',
                'is_obsolete' => 0,
              ),
              'value' => $content[$j][$county_col_name],
            ));
          }

          if (isset($district_col_name)) {
            tpps_chado_insert_record('stockprop', array(
              'stock_id' => $stock_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'local',
                ),
                'name' => 'district',
                'is_obsolete' => 0,
              ),
              'value' => $content[$j][$district_col_name],
            ));
          }
        }
        else {
          $pop_group_name = $groups['Location (latitude/longitude or country/state or population group)']['12'];

          $loc = $thirdpage['tree-accession']['pop-group'][$content[$i][$pop_group_name]];
          $coord = tpps_standard_coord($loc);

          if ($coord) {
            $parts = explode(',', $coord);
            tpps_chado_insert_record('stockprop', array(
              'stock_id' => $stock_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'local',
                ),
                'name' => 'gps_latitude',
                'is_obsolete' => 0,
              ),
              'value' => $parts[0],
            ));

            tpps_chado_insert_record('stockprop', array(
              'stock_id' => $stock_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'local',
                ),
                'name' => 'gps_longitude',
                'is_obsolete' => 0,
              ),
              'value' => $parts[1],
            ));
          }
          else {
            tpps_chado_insert_record('stockprop', array(
              'stock_id' => $stock_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'nd_geolocation_property',
                ),
                'name' => 'Location',
                'is_obsolete' => 0,
              ),
              'value' => $loc,
            ));
          }
        }
      }

      $file->status = FILE_STATUS_PERMANENT;
      $file = file_save($file);
      $file_rank++;
    }
  }

  foreach ($stock_ids as $tree_id => $stock_id) {
    tpps_chado_insert_record('project_stock', array(
      'stock_id' => $stock_id,
      'project_id' => $project_id,
    ));
  }

  $form_state['file_rank'] = $file_rank;

}

/**
 * Submits Tripal FASTAImporter job for reference genome.
 *
 * The remaining data for the fourth page is submitted during the TPPS File
 * Parsing Tripal Job due to its size.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 * @param int $project_id
 *   The project_id of the project that the data will reference in the database.
 * @param int $file_rank
 *   The rank number for files associated with the project.
 * @param array $organism_ids
 *   The array of organism_ids associated with the project.
 */
function tpps_submit_page_4(array &$form_state, $project_id, &$file_rank, array $organism_ids) {
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];

  for ($i = 1; $i <= $organism_number; $i++) {
    if (isset($fourthpage["organism-$i"]['genotype'])) {
      $ref_genome = $fourthpage["organism-$i"]['genotype']['ref-genome'];

      if ($ref_genome === 'url' or $ref_genome === 'manual' or $ref_genome === 'manual2') {
        // Create job for tripal fasta importer.
        $class = 'FASTAImporter';
        tripal_load_include_importer_class($class);

        $fasta = $fourthpage["organism-$i"]['genotype']['tripal_fasta'];

        $file_upload = isset($fasta['file']['file_upload']) ? trim($fasta['file']['file_upload']) : 0;
        $file_existing = isset($fasta['file']['file_upload_existing']) ? trim($fasta['file']['file_upload_existing']) : 0;
        $file_remote = isset($fasta['file']['file_remote']) ? trim($fasta['file']['file_remote']) : 0;
        $analysis_id = $fasta['analysis_id'];
        $seqtype = $fasta['seqtype'];
        $organism_id = $organism_ids[$i];
        $re_accession = $fasta['db']['re_accession'];
        $db_id = $fasta['db']['db_id'];

        $run_args = array(
          'importer_class' => $class,
          'file_remote' => $file_remote,
          'analysis_id' => $analysis_id,
          'seqtype' => $seqtype,
          'organism_id' => $organism_id,
          'method' => '2',
          'match_type' => '0',
          're_name' => '',
          're_uname' => '',
          're_accession' => $re_accession,
          'db_id' => $db_id,
          'rel_type' => '',
          're_subject' => '',
          'parent_type' => '',
        );

        $file_details = array();

        if ($file_existing) {
          $file_details['fid'] = $file_existing;
        }
        elseif ($file_upload) {
          $file_details['fid'] = $file_upload;
        }
        elseif ($file_remote) {
          $file_details['file_remote'] = $file_remote;
        }

        try {
          $importer = new $class();
          $form = array();
          $importer->formSubmit($form, $form_state);

          $importer->create($run_args, $file_details);

          $importer->submitJob();

        }
        catch (Exception $ex) {
          drupal_set_message('Cannot submit import: ' . $ex->getMessage(), 'error');
        }
      }
    }
  }
}
