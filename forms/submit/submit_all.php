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
 * @param string $accession
 *   The accession number of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_all($accession, $job = NULL) {

  $job->logMessage('[INFO] Setting up...');
  $job->setInterval(1);
  $form_state = tpps_load_submission($accession);
  $form_state['status'] = 'Submission Job Running';
  tpps_update_submission($form_state, array('status' => 'Submission Job Running'));
  $job->logMessage('[INFO] Clearing Database...');
  tpps_submission_clear_db($accession);
  $job->logMessage('[INFO] Database Cleared.');
  $project_id = $form_state['ids']['project_id'] ?? NULL;
  $transaction = db_transaction();

  try {
    $form_state = tpps_load_submission($accession);
    tpps_clean_state($form_state);
    $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
    $form_state['file_rank'] = 0;
    $form_state['ids'] = array();

    $job->logMessage('[INFO] Creating project record...');
    $form_state['title'] = $firstpage['publication']['title'];
    $form_state['abstract'] = $firstpage['publication']['abstract'];
    $project_record = array(
      'name' => $firstpage['publication']['title'],
      'description' => $firstpage['publication']['abstract'],
    );
    if (!empty($project_id)) {
      $project_record['project_id'] = $project_id;
    }
    $form_state['ids']['project_id'] = tpps_chado_insert_record('project', $project_record);
    $job->logMessage("[INFO] Project record created. project_id: @pid\n", array('@pid' => $form_state['ids']['project_id']));

    tpps_tripal_entity_publish('Project', array($firstpage['publication']['title'], $form_state['ids']['project_id']));

    $job->logMessage("[INFO] Submitting Publication/Species information...");
    tpps_submit_page_1($form_state, $job);
    $job->logMessage("[INFO] Publication/Species information submitted!\n");

    $job->logMessage("[INFO] Submitting Study Details...");
    tpps_submit_page_2($form_state, $job);
    $job->logMessage("[INFO] Study Details sumbitted!\n");

    $job->logMessage("[INFO] Submitting Accession information...");
    tpps_submit_page_3($form_state, $job);
    $job->logMessage("[INFO] Accession information submitted!\n");

    $job->logMessage("[INFO] Submitting Raw data...");
    tpps_submit_page_4($form_state, $job);
    $job->logMessage("[INFO] Raw data submitted!\n");

    $job->logMessage("[INFO] Submitting Summary information...");
    tpps_submit_summary($form_state);
    $job->logMessage("[INFO] Summary information submitted!\n");

    tpps_update_submission($form_state);

    $job->logMessage("[INFO] Renaming files...");
    tpps_submission_rename_files($accession);
    $job->logMessage("[INFO] Files renamed!\n");
    $form_state = tpps_load_submission($accession);
    $form_state['status'] = 'Approved';
    $form_state['loaded'] = time();
    $job->logMessage("[INFO] Finishing up...");
    tpps_update_submission($form_state, array('status' => 'Approved'));
    $job->logMessage("[INFO] Complete!");
  }
  catch (Exception $e) {
    $transaction->rollback();
    $form_state = tpps_load_submission($accession);
    $form_state['status'] = 'Pending Approval';
    tpps_update_submission($form_state, array('status' => 'Pending Approval'));
    $job->logMessage('[ERROR] Job failed', array(), TRIPAL_ERROR);
    $job->logMessage('[ERROR] Error message: @msg', array('@msg' => $e->getMessage()), TRIPAL_ERROR);
    $job->logMessage("[ERROR] Trace: \n@trace", array('@trace' => $e->getTraceAsString()), TRIPAL_ERROR);
    watchdog_exception('tpps', $e);
  }
}

/**
 * Submits Publication and Species data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_page_1(array &$form_state, &$job = NULL) {

  $dbxref_id = $form_state['dbxref_id'];
  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];
  $seconds = $firstpage['publication']['secondaryAuthors'];

  tpps_chado_insert_record('project_dbxref', array(
    'project_id' => $form_state['ids']['project_id'],
    'dbxref_id' => $dbxref_id,
    'is_current' => TRUE,
  ));

  if (!empty($form_state['tpps_type']) and $form_state['tpps_type'] == 'tppsc' and !empty($form_state['saved_values'][TPPS_PAGE_1]['doi'])) {
    $dryad_db = chado_get_db(array('name' => 'dryad'));
    $dryad_dbxref = chado_insert_dbxref(array(
      'db_id' => $dryad_db->db_id,
      'accession' => $form_state['saved_values'][TPPS_PAGE_1]['doi'],
    ))->dbxref_id;
    tpps_chado_insert_record('project_dbxref', array(
      'project_id' => $form_state['ids']['project_id'],
      'dbxref_id' => $dryad_dbxref,
      'is_current' => TRUE,
    ));
  }

  if (!empty($firstpage['photo'])) {
    tpps_add_project_file($form_state, $firstpage['photo']);
  }

  $primary_author_id = tpps_chado_insert_record('contact', array(
    'name' => $firstpage['primaryAuthor'],
    'type_id' => tpps_load_cvterm('person')->cvterm_id,
  ));

  tpps_chado_insert_record('project_contact', array(
    'project_id' => $form_state['ids']['project_id'],
    'contact_id' => $primary_author_id,
  ));

  $authors = array($firstpage['primaryAuthor']);
  if ($seconds['number'] != 0) {
    for ($i = 1; $i <= $seconds['number']; $i++) {
      tpps_chado_insert_record('contact', array(
        'name' => $seconds[$i],
        'type_id' => tpps_load_cvterm('person')->cvterm_id,
      ));

      $names = explode(" ", $seconds[$i]);
      $first_name = implode(" ", array_slice($names, 0, -1));
      $last_name = end($names);

      $pubauthors[] = array(
        'rank' => "$i",
        'surname' => $last_name,
        'givennames' => $first_name,
      );
      $authors[] = $seconds[$i];
    }
  }

  $publication_id = tpps_chado_insert_record('pub', array(
    'title' => $firstpage['publication']['title'],
    'series_name' => $firstpage['publication']['journal'],
    'type_id' => tpps_load_cvterm('article')->cvterm_id,
    'pyear' => $firstpage['publication']['year'],
    'uniquename' => implode('; ', $authors) . " {$firstpage['publication']['title']}. {$firstpage['publication']['journal']}; {$firstpage['publication']['year']}",
  ));
  $form_state['ids']['pub_id'] = $publication_id;
  tpps_tripal_entity_publish('Publication', array($firstpage['publication']['title'], $publication_id));
  $form_state['pyear'] = $firstpage['publication']['year'];
  $form_state['journal'] = $firstpage['publication']['journal'];

  if (!empty($firstpage['publication']['abstract'])) {
    tpps_chado_insert_record('pubprop', array(
      'pub_id' => $publication_id,
      'type_id' => tpps_load_cvterm('abstract')->cvterm_id,
      'value' => $firstpage['publication']['abstract'],
    ));
  }

  tpps_chado_insert_record('pubprop', array(
    'pub_id' => $publication_id,
    'type_id' => tpps_load_cvterm('authors')->cvterm_id,
    'value' => implode(', ', $authors),
  ));
  $form_state['authors'] = $authors;

  tpps_chado_insert_record('project_pub', array(
    'project_id' => $form_state['ids']['project_id'],
    'pub_id' => $publication_id,
  ));

  if (!empty($firstpage['organization'])) {
    $organization_id = tpps_chado_insert_record('contact', array(
      'name' => $firstpage['organization'],
      'type_id' => tpps_load_cvterm('organization')->cvterm_id,
    ));

    tpps_chado_insert_record('contact_relationship', array(
      'type_id' => tpps_load_cvterm('contact_part_of')->cvterm_id,
      'subject_id' => $primary_author_id,
      'object_id' => $organization_id,
    ));
  }

  $names = explode(" ", $firstpage['primaryAuthor']);
  $first_name = implode(" ", array_slice($names, 0, -1));
  $last_name = end($names);

  tpps_chado_insert_record('pubauthor', array(
    'pub_id' => $publication_id,
    'rank' => '0',
    'surname' => $last_name,
    'givennames' => $first_name,
  ));

  if (!empty($pubauthors)) {
    foreach ($pubauthors as $info) {
      $info['pub_id'] = $publication_id;
      tpps_chado_insert_record('pubauthor', $info);
    }
  }

  $form_state['ids']['organism_ids'] = array();
  $organism_number = $firstpage['organism']['number'];

  for ($i = 1; $i <= $organism_number; $i++) {
    $parts = explode(" ", $firstpage['organism'][$i]['name']);
    $genus = $parts[0];
    $species = implode(" ", array_slice($parts, 1));
    $infra = NULL;
    if (isset($parts[2]) and ($parts[2] == 'var.' or $parts[2] == 'subsp.')) {
      $infra = implode(" ", array_slice($parts, 2));
    }

    $record = array(
      'genus' => $genus,
      'species' => $species,
      'infraspecific_name' => $infra,
    );

    if (preg_match('/ x /', $species)) {
      $record['type_id'] = tpps_load_cvterm('speciesaggregate')->cvterm_id;
    }
    $form_state['ids']['organism_ids'][$i] = tpps_chado_insert_record('organism', $record);

    if (!empty(tpps_load_cvterm('Type'))) {
      tpps_chado_insert_record('organismprop', array(
        'organism_id' => $form_state['ids']['organism_ids'][$i],
        'type_id' => tpps_load_cvterm('Type')->cvterm_id,
        'value' => $firstpage['organism'][$i]['is_tree'] ? 'Tree' : 'Non-tree',
      ));
    }

    if ($organism_number != 1) {
      $found = FALSE;
      if (empty($thirdpage['tree-accession']['check'])) {
        $options = array(
          'cols' => array(),
          'search' => $firstpage['organism'][$i]['name'],
          'found' => &$found,
        );
        $tree_accession = $thirdpage['tree-accession']["species-1"];
        $groups = $tree_accession['file-groups'];
        if ($groups['Genus and Species']['#type'] == 'separate') {
          $options['cols']['genus'] = $groups['Genus and Species']['6'];
          $options['cols']['species'] = $groups['Genus and Species']['7'];
        }
        else {
          $options['cols']['org'] = $groups['Genus and Species']['10'];
        }
        $fid = $tree_accession['file'];
        tpps_file_iterator($fid, 'tpps_check_organisms', $options);
      }
      else {
        $found = !empty($thirdpage['tree-accession']["species-$i"]['file']);
      }

      if (!$found) {
        continue;
      }
    }

    $code_exists = tpps_chado_prop_exists('organism', $form_state['ids']['organism_ids'][$i], 'organism 4 letter code');

    if (!$code_exists) {
      $g_offset = 0;
      $s_offset = 0;
      do {
        if (isset($trial_code)) {
          if ($s_offset < strlen($species) - 2) {
            $s_offset++;
          }
          elseif ($g_offset < strlen($genus) - 2) {
            $s_offset = 0;
            $g_offset++;
          }
          else {
            throw new Exception("TPPS was unable to create a 4 letter species code for the species '$genus $species'.");
          }
        }
        $trial_code = substr($genus, $g_offset, 2) . substr($species, $s_offset, 2);
        $new_code_query = chado_select_record('organismprop', array('value'), array(
          'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
          'value' => $trial_code,
        ));
      } while (!empty($new_code_query));

      tpps_chado_insert_record('organismprop', array(
        'organism_id' => $form_state['ids']['organism_ids'][$i],
        'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
        'value' => $trial_code,
      ));
    }

    $ranks = array(
      'family',
      'order',
      'subkingdom',
    );

    foreach ($ranks as $rank) {
      $exists = tpps_chado_prop_exists('organism', $form_state['ids']['organism_ids'][$i], $rank);
      if (!$exists) {
        $taxon = tpps_get_taxon($firstpage['organism'][$i]['name'], $rank);
        if ($taxon) {
          tpps_chado_insert_record('organismprop', array(
            'organism_id' => $form_state['ids']['organism_ids'][$i],
            'type_id' => tpps_load_cvterm($rank)->cvterm_id,
            'value' => $taxon,
          ));
        }
      }
    }

    tpps_chado_insert_record('project_organism', array(
      'organism_id' => $form_state['ids']['organism_ids'][$i],
      'project_id' => $form_state['ids']['project_id'],
    ));

    tpps_chado_insert_record('pub_organism', array(
      'organism_id' => $form_state['ids']['organism_ids'][$i],
      'pub_id' => $publication_id,
    ));

    tpps_tripal_entity_publish('Organism', array("$genus $species", $form_state['ids']['organism_ids'][$i]));
  }
}

/**
 * Submits Study Design data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_page_2(array &$form_state, &$job = NULL) {

  $secondpage = $form_state['saved_values'][TPPS_PAGE_2];

  if (!empty($secondpage['StartingDate'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('study_start')->cvterm_id,
      'value' => $secondpage['StartingDate']['month'] . " " . $secondpage['StartingDate']['year'],
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('study_end')->cvterm_id,
      'value' => $secondpage['EndingDate']['month'] . " " . $secondpage['EndingDate']['year'],
    ));
  }

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $form_state['ids']['project_id'],
    'type_id' => tpps_load_cvterm('association_results_type')->cvterm_id,
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
    'project_id' => $form_state['ids']['project_id'],
    'type_id' => tpps_load_cvterm('study_type')->cvterm_id,
    'value' => $studytype_options[$secondpage['study_type']],
  ));

  if (!empty($secondpage['study_info']['season'])) {
    $seasons = implode($secondpage['study_info']['season']);

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('assession_season')->cvterm_id,
      'value' => $seasons,
    ));
  }

  if (!empty($secondpage['study_info']['assessions'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('assession_number')->cvterm_id,
      'value' => $secondpage['study_info']['assessions'],
    ));
  }

  if (!empty($secondpage['study_info']['temp'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('temperature_high')->cvterm_id,
      'value' => $secondpage['study_info']['temp']['high'],
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('temperature_low')->cvterm_id,
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
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm("{$type}_control")->cvterm_id,
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm("{$type}_level")->cvterm_id,
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm("{$type}_level")->cvterm_id,
          'value' => $set['uncontrolled'],
        ));
      }
    }
  }

  if (!empty($secondpage['study_info']['rooting'])) {
    $root = $secondpage['study_info']['rooting'];

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('rooting_type')->cvterm_id,
      'value' => $root['option'],
    ));

    if ($root['option'] == 'Soil') {
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('soil_type')->cvterm_id,
        'value' => ($root['soil']['type'] == 'Other') ? $root['soil']['other'] : $root['soil']['type'],
      ));

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('soil_container')->cvterm_id,
        'value' => $root['soil']['container'],
      ));
    }

    if (!empty($secondpage['study_info']['rooting']['ph'])) {
      $set = $secondpage['study_info']['rooting']['ph'];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('pH_control')->cvterm_id,
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('pH_level')->cvterm_id,
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('pH_level')->cvterm_id,
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
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('treatment')->cvterm_id,
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
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('irrigation_type')->cvterm_id,
      'value' => ($irrigation['option'] == 'Other') ? $irrigation['other'] : $irrigation['option'],
    ));
  }

  if (!empty($form_state['values']['study_info']['biotic_env']['option'])) {
    foreach ($form_state['values']['study_info']['biotic_env']['option'] as $key => $check) {
      if ($check) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('biotic_environment')->cvterm_id,
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
            'project_id' => $form_state['ids']['project_id'],
            'type_id' => tpps_load_cvterm('treatment')->cvterm_id,
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
 * Submits Plant Accession data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_page_3(array &$form_state, &$job = NULL) {
  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];
  $organism_number = $firstpage['organism']['number'];
  $form_state['locations'] = array();
  $form_state['tree_info'] = array();
  $stock_count = 0;
  $loc_name = 'Location (latitude/longitude or country/state or population group)';

  if (!empty($thirdpage['study_location'])) {
    $type = $thirdpage['study_location']['type'];
    $locs = $thirdpage['study_location']['locations'];

    for ($i = 1; $i <= $locs['number']; $i++) {
      if ($type !== '2') {
        $standard_coordinate = explode(',', tpps_standard_coord($locs[$i]));
        $latitude = $standard_coordinate[0];
        $longitude = $standard_coordinate[1];

        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('gps_latitude')->cvterm_id,
          'value' => $latitude,
          'rank' => $i,
        ));

        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('gps_longitude')->cvterm_id,
          'value' => $longitude,
          'rank' => $i,
        ));
      }
      else {
        $loc = $locs[$i];
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('experiment_location')->cvterm_id,
          'value' => $loc,
          'rank' => $i,
        ));

        if (isset($geo_api_key)) {
          $query = urlencode($location);
          $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
          $response = json_decode(file_get_contents($url));

          if ($response->total_results) {
            $result = $response->results[0]->geometry;
            $form_state['locations'][$loc] = $result;

            tpps_chado_insert_record('projectprop', array(
              'project_id' => $form_state['ids']['project_id'],
              'type_id' => tpps_load_cvterm('gps_latitude')->cvterm_id,
              'value' => $result->lat,
              'rank' => $i,
            ));

            tpps_chado_insert_record('projectprop', array(
              'project_id' => $form_state['ids']['project_id'],
              'type_id' => tpps_load_cvterm('gps_longitude')->cvterm_id,
              'value' => $result->lng,
              'rank' => $i,
            ));
          }
        }
      }
    }
  }

  $cvterms = array(
    'org' => tpps_load_cvterm('organism')->cvterm_id,
    'clone' => tpps_load_cvterm('clone')->cvterm_id,
    'has_part' => tpps_load_cvterm('has_part')->cvterm_id,
    'lat' => tpps_load_cvterm('gps_latitude')->cvterm_id,
    'lng' => tpps_load_cvterm('gps_longitude')->cvterm_id,
    'country' => tpps_load_cvterm('country')->cvterm_id,
    'state' => tpps_load_cvterm('state')->cvterm_id,
    'county' => tpps_load_cvterm('county')->cvterm_id,
    'district' => tpps_load_cvterm('district')->cvterm_id,
    'loc' => tpps_load_cvterm('location')->cvterm_id,
    'gps_type' => tpps_load_cvterm('gps_type')->cvterm_id,
    'precision' => tpps_load_cvterm('gps_precision')->cvterm_id,
  );

  $records = array(
    'stock' => array(),
    'stockprop' => array(),
    'stock_relationship' => array(),
    'project_stock' => array(),
  );
  $overrides = array(
    'stock_relationship' => array(
      'subject' => array(
        'table' => 'stock',
        'columns' => array(
          'subject_id' => 'stock_id',
        ),
      ),
      'object' => array(
        'table' => 'stock',
        'columns' => array(
          'object_id' => 'stock_id',
        ),
      ),
    ),
  );

  $multi_insert_options = array(
    'fk_overrides' => $overrides,
    'fks' => 'stock',
    'entities' => array(
      'label' => 'Stock',
      'table' => 'stock',
      'prefix' => $form_state['accession'] . '-',
    ),
  );

  $names = array();
  for ($i = 1; $i <= $organism_number; $i++) {
    $names[$i] = $firstpage['organism'][$i]['name'];
  }
  $names['number'] = $firstpage['organism']['number'];

  $options = array(
    'cvterms' => $cvterms,
    'records' => $records,
    'overrides' => $overrides,
    'locations' => &$form_state['locations'],
    'accession' => $form_state['accession'],
    'single_file' => empty($thirdpage['tree-accession']['check']),
    'org_names' => $names,
    'saved_ids' => &$form_state['ids'],
    'stock_count' => &$stock_count,
    'multi_insert' => $multi_insert_options,
    'tree_info' => &$form_state['tree_info'],
    'job' => &$job,
  );

  for ($i = 1; $i <= $organism_number; $i++) {
    $tree_accession = $thirdpage['tree-accession']["species-$i"];
    $fid = $tree_accession['file'];

    tpps_add_project_file($form_state, $fid);

    $column_vals = $tree_accession['file-columns'];
    $groups = $tree_accession['file-groups'];

    $options['org_num'] = $i;
    $options['no_header'] = !empty($tree_accession['file-no-header']);
    $options['empty'] = $tree_accession['file-empty'];
    $options['pop_group'] = $tree_accession['pop-group'];
    $options['exact'] = $tree_accession['exact_coords'] ?? NULL;
    $options['precision'] = NULL;
    if (!$options['exact']) {
      $options['precision'] = $tree_accession['coord_precision'] ?? NULL;
    }
    $county = array_search('8', $column_vals);
    $district = array_search('9', $column_vals);
    $clone = array_search('13', $column_vals);
    $options['column_ids'] = array(
      'id' => $groups['Tree Id']['1'],
      'lat' => $groups[$loc_name]['4'] ?? NULL,
      'lng' => $groups[$loc_name]['5'] ?? NULL,
      'country' => $groups[$loc_name]['2'] ?? NULL,
      'state' => $groups[$loc_name]['3'] ?? NULL,
      'county' => ($county !== FALSE) ? $county : NULL,
      'district' => ($district !== FALSE) ? $district : NULL,
      'clone' => ($clone !== FALSE) ? $clone : NULL,
      'pop_group' => $groups[$loc_name]['12'] ?? NULL,
    );

    if ($organism_number != 1 and empty($thirdpage['tree-accession']['check'])) {
      if ($groups['Genus and Species']['#type'] == 'separate') {
        $options['column_ids']['genus'] = $groups['Genus and Species']['6'];
        $options['column_ids']['species'] = $groups['Genus and Species']['7'];
      }
      else {
        $options['column_ids']['org'] = $groups['Genus and Species']['10'];
      }
    }

    tpps_file_iterator($fid, 'tpps_process_accession', $options);

    $new_ids = tpps_chado_insert_multi($options['records'], $multi_insert_options);
    foreach ($new_ids as $t_id => $stock_id) {
      $form_state['tree_info'][$t_id]['stock_id'] = $stock_id;
    }
    unset($options['records']);
    $stock_count = 0;
    if (empty($thirdpage['tree-accession']['check'])) {
      break;
    }
  }

  if (!empty($thirdpage['existing_trees'])) {
    tpps_matching_trees($form_state['ids']['project_id']);
  }
}

/**
 * Submits Tripal FASTAImporter job for reference genome.
 *
 * The remaining data for the fourth page is submitted during the TPPS File
 * Parsing Tripal Job due to its size.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_page_4(array &$form_state, &$job = NULL) {
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
  $species_codes = array();

  for ($i = 1; $i <= $organism_number; $i++) {
    // Get species codes.
    $species_codes[$form_state['ids']['organism_ids'][$i]] = current(chado_select_record('organismprop', array('value'), array(
      'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
      'organism_id' => $form_state['ids']['organism_ids'][$i],
    ), array(
      'limit' => 1,
    )))->value;

    // Submit importer jobs.
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
        $organism_id = $form_state['ids']['organism_ids'][$i];
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
      elseif ($ref_genome === 'bio') {
        $eutils = $fourthpage["organism-$i"]['genotype']['tripal_eutils'];
        $class = 'EutilsImporter';
        tripal_load_include_importer_class($class);

        $run_args = array(
          'importer_class' => $class,
          'db' => $eutils['db'],
          'accession' => $eutils['accession'],
          'linked_records' => $eutils['options']['linked_records']
        );

        try {
          $importer = new $class();
          $importer->create($run_args);
          $importer->submitJob();
        }
        catch (Exception $ex) {
          drupal_set_message('Cannot submit BioProject: ' . $ex->getMessage(), 'error');
        }
      }
    }
  }

  $form_state['data']['phenotype'] = array();
  $form_state['data']['phenotype_meta'] = array();

  // Submit raw data.
  for ($i = 1; $i <= $organism_number; $i++) {
    tpps_submit_phenotype($form_state, $i, $job);
    tpps_submit_genotype($form_state, $species_codes, $i);
    tpps_submit_environment($form_state, $i);
  }
}

/**
 * Submits phenotype information for one species.
 *
 * @param array $form_state
 *   The TPPS submission object.
 * @param int $i
 *   The organism number we are submitting.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_phenotype(array &$form_state, $i, &$job = NULL) {
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $phenotype = $fourthpage["organism-$i"]['phenotype'] ?? NULL;
  if (empty($phenotype)) {
    return;
  }

  // Get appropriate cvterms.
  $phenotype_cvterms = array(
    'time' => tpps_load_cvterm('time')->cvterm_id,
    'desc' => tpps_load_cvterm('description')->cvterm_id,
    'unit' => tpps_load_cvterm('unit')->cvterm_id,
    'min' => tpps_load_cvterm('minimum')->cvterm_id,
    'max' => tpps_load_cvterm('maximum')->cvterm_id,
  );

  $records = array(
    'phenotype' => array(),
    'phenotypeprop' => array(),
    'stock_phenotype' => array(),
  );
  $phenotype_count = 0;

  $options = array(
    'records' => $records,
    'cvterms' => $phenotype_cvterms,
    'accession' => $form_state['accession'],
    'tree_info' => $form_state['tree_info'],
    'suffix' => 0,
    'phenotype_count' => $phenotype_count,
    'data' => &$form_state['data']['phenotype'],
    'job' => &$job,
  );

  if (empty($phenotype['iso-check'])) {
    $phenotype_number = $phenotype['phenotypes-meta']['number'];
    $phenotypes_meta = array();
    $data_fid = $phenotype['file'];
    $phenos_edit = $form_state['phenotypes_edit'] ?? NULL;

    tpps_add_project_file($form_state, $data_fid);

    // Populate $phenotypes_meta with manually entered metadata.
    for ($j = 1; $j <= $phenotype_number; $j++) {
      $name = strtolower($phenotype['phenotypes-meta'][$j]['name']);
      if (!empty($phenos_edit[$j])) {
        $result = $phenos_edit[$j] + $phenotype['phenotypes-meta'][$j];
        $phenotype['phenotypes-meta'][$j] = $result;
      }
      $phenotypes_meta[$name] = array();
      $phenotypes_meta[$name]['attr'] = $phenotype['phenotypes-meta'][$j]['attribute'];
      if ($phenotype['phenotypes-meta'][$j]['attribute'] == 'other') {
        $phenotypes_meta[$name]['attr-other'] = $phenotype['phenotypes-meta'][$j]['attr-other'];
      }
      $phenotypes_meta[$name]['desc'] = $phenotype['phenotypes-meta'][$j]['description'];
      $phenotypes_meta[$name]['unit'] = $phenotype['phenotypes-meta'][$j]['units'];
      if ($phenotype['phenotypes-meta'][$j]['units'] == 'other') {
        $phenotypes_meta[$name]['unit-other'] = $phenotype['phenotypes-meta'][$j]['unit-other'];
      }
      $phenotypes_meta[$name]['struct'] = $phenotype['phenotypes-meta'][$j]['structure'];
      if ($phenotype['phenotypes-meta'][$j]['structure'] == 'other') {
        $phenotypes_meta[$name]['struct-other'] = $phenotype['phenotypes-meta'][$j]['struct-other'];
      }
      if (!empty($phenotype['phenotypes-meta'][$j]['val-check']) or !empty($phenotype['phenotypes-meta'][$j]['bin-check'])) {
        $phenotypes_meta[$name]['min'] = $phenotype['phenotypes-meta'][$j]['min'];
        $phenotypes_meta[$name]['max'] = $phenotype['phenotypes-meta'][$j]['max'];
      }
      $phenotypes_meta[$name]['env'] = !empty($phenotype['phenotypes-meta'][$j]['env-check']);
    }

    if ($phenotype['check'] == '1') {
      $meta_fid = $phenotype['metadata'];
      tpps_add_project_file($form_state, $meta_fid);

      // Get metadata column values.
      $groups = $phenotype['metadata-groups'];
      $column_vals = $phenotype['metadata-columns'];
      $struct = array_search('5', $column_vals);
      $min = array_search('6', $column_vals);
      $max = array_search('7', $column_vals);
      $columns = array(
        'name' => $groups['Phenotype Id']['1'],
        'attr' => $groups['Attribute']['2'],
        'desc' => $groups['Description']['3'],
        'unit' => $groups['Units']['4'],
        'struct' => !empty($struct) ? $struct : NULL,
        'min' => !empty($min) ? $min : NULL,
        'max' => !empty($max) ? $max : NULL,
      );

      $meta_options = array(
        'no_header' => $phenotype['metadata-no-header'],
        'meta_columns' => $columns,
        'meta' => &$phenotypes_meta,
      );

      tpps_file_iterator($meta_fid, 'tpps_process_phenotype_meta', $meta_options);
    }

    $time_options = array();
    if ($phenotype['time']['time-check']) {
      $time_options = $phenotype['time'];
    }
    tpps_refine_phenotype_meta($phenotypes_meta, $time_options, $job);

    // Get metadata header values.
    $groups = $phenotype['file-groups'];
    $column_vals = $phenotype['file-columns'];
    $time_index = ($phenotype['format'] == 0) ? '2' : '4';
    $clone_index = ($phenotype['format'] == 0) ? '3' : '5';
    $time = array_search($time_index, $column_vals);
    $clone = array_search($clone_index, $column_vals);
    $meta_headers = array(
      'name' => $groups['Phenotype Name/Identifier']['2'] ?? NULL,
      'value' => $groups['Phenotype Value(s)']['3'] ?? NULL,
      'time' => !empty($time) ? $time : NULL,
      'clone' => !empty($clone) ? $clone : NULL,
    );

    // Get data header values.
    if ($phenotype['format'] == 0) {
      $file_headers = tpps_file_headers($data_fid, $phenotype['file-no-header']);
      $data_columns = array();
      foreach ($groups['Phenotype Data']['0'] as $col) {
        $data_columns[$col] = $file_headers[$col];
      }
      unset($file_headers);
    }

    $options['no_header'] = $phenotype['file-no-header'];
    $options['tree_id'] = $groups['Tree Identifier']['1'];
    $options['meta_headers'] = $meta_headers;
    $options['data_columns'] = $data_columns ?? NULL;
    $options['meta'] = $phenotypes_meta;
    $options['file_empty'] = $phenotype['file-empty'];

    tpps_file_iterator($data_fid, 'tpps_process_phenotype_data', $options);
    $form_state['data']['phenotype_meta'] += $phenotypes_meta;
  }
  else {
    $iso_fid = $phenotype['iso'];
    tpps_add_project_file($form_state, $iso_fid);

    $options['iso'] = TRUE;
    $options['records'] = $records;
    $options['cvterms'] = $phenotype_cvterms;
    $options['file_headers'] = tpps_file_headers($iso_fid);
    $options['meta'] = array(
      'desc' => "Mass Spectrometry",
      'unit' => "intensity (arbitrary units)",
      'attr_id' => tpps_load_cvterm('intensity')->cvterm_id,
    );

    tpps_file_iterator($iso_fid, 'tpps_process_phenotype_data', $options);
  }
  tpps_chado_insert_multi($options['records']);
}

/**
 * Submits genotype information for one species.
 *
 * @param array $form_state
 *   The TPPS submission object.
 * @param array $species_codes
 *   An array of 4-letter species codes associated with the submission.
 * @param int $i
 *   The organism number we are submitting.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_genotype(array &$form_state, array $species_codes, $i, &$job = NULL) {
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  if (empty($genotype)) {
    return;
  }
  $project_id = $form_state['ids']['project_id'];
  $record_group = variable_get('tpps_record_group', 10000);

  $genotype_count = 0;
  $genotype_total = 0;
  $seq_var_cvterm = tpps_load_cvterm('sequence_variant')->cvterm_id;
  $overrides = array(
    'genotype_call' => array(
      'variant' => array(
        'table' => 'feature',
        'columns' => array(
          'variant_id' => 'feature_id',
        ),
      ),
      'marker' => array(
        'table' => 'feature',
        'columns' => array(
          'marker_id' => 'feature_id',
        ),
      ),
    ),
  );

  $records = array(
    'feature' => array(),
    'genotype' => array(),
    'genotype_call' => array(),
    'stock_genotype' => array(),
  );

  $multi_insert_options = array(
    'fk_overrides' => $overrides,
    'entities' => array(
      'label' => 'Genotype',
      'table' => 'genotype',
    ),
  );

  $options = array(
    'records' => $records,
    'tree_info' => $form_state['tree_info'],
    'species_codes' => $species_codes,
    'genotype_count' => &$genotype_count,
    'genotype_total' => &$genotype_total,
    'project_id' => $project_id,
    'seq_var_cvterm' => $seq_var_cvterm,
    'multi_insert' => &$multi_insert_options,
    'job' => &$job,
  );

  /*if ($genotype['ref-genome'] == 'bio') {

    $bioproject_id = tpps_chado_insert_record('dbxref', array(
      'db_id' => array(
        'name' => 'NCBI BioProject',
      ),
      'accession' => $genotype['BioProject-id'],
    ));

    $project_dbxref_id = tpps_chado_insert_record('project_dbxref', array(
      'project_id' => $project_id,
      'dbxref_id' => $bioproject_id,
    ));

    $bioproject_assembly_file_ids = array();
    foreach ($genotype['assembly-auto'] as $key => $val) {
      if ($val == '1') {
        array_push($bioproject_assembly_file_ids, tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => "https://www.ncbi.nlm.nih.gov/nuccore/$key",
          'rank' => $file_rank,
        )));
        $file_rank++;
      }
    }
  }
  else*/
  if ($genotype['ref-genome'] == 'manual' or $genotype['ref-genome'] == 'manual2' or $genotype['ref-genome'] == 'url') {
    if ($genotype['tripal_fasta']['file_upload']) {
      // Uploaded new file.
      $assembly_user = $genotype['tripal_fasta']['file_upload'];
      tpps_add_project_file($form_state, $assembly_user);
    }
    if ($genotype['tripal_fasta']['file_upload_existing']) {
      // Uploaded existing file.
      $assembly_user = $genotype['tripal_fasta']['file_upload_existing'];
      tpps_add_project_file($form_state, $assembly_user);
    }
    if ($genotype['tripal_fasta']['file_remote']) {
      // Provided url to file.
      $assembly_user = $genotype['tripal_fasta']['file_remote'];
      $assembly_user_id = tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
        'type_id' => tpps_load_cvterm('file_path')->cvterm_id,
        'value' => $assembly_user,
        'rank' => $form_state['file_rank'],
      ));
      $form_state['file_rank']++;
    }
  }
  elseif ($genotype['ref-genome'] != 'none') {
    $reference_genome_id = tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => tpps_load_cvterm('reference_genome')->cvterm_id,
      'value' => $genotype['ref-genome'],
    ));
  }

  if (!empty($genotype['files']['file-type']['SNPs Genotype Assay'])) {
    $snp_fid = $genotype['files']['snps-assay'];
    tpps_add_project_file($form_state, $snp_fid);

    $options['type'] = 'snp';
    $options['headers'] = tpps_file_headers($snp_fid);
    $options['marker'] = 'SNP';
    $options['type_cvterm'] = tpps_load_cvterm('snp')->cvterm_id;

    if (!empty($genotype['files']['file-type']['SNPs Associations'])) {
      $assoc_fid = $genotype['files']['snps-association'];
      tpps_add_project_file($form_state, $assoc_fid);

      $options['records']['featureloc'] = array();
      $options['records']['featureprop'] = array();
      $options['records']['feature_relationship'] = array();
      $options['records']['feature_cvterm'] = array();
      $options['records']['feature_cvtermprop'] = array();

      $options['associations'] = array();
      $options['associations_tool'] = $genotype['files']['snps-association-tool'];
      $options['associations_groups'] = $genotype['files']['snps-association-groups'];
      $options['scaffold_cvterm'] = tpps_load_cvterm('scaffold')->cvterm_id;
      $options['phenotype_meta'] = $form_state['data']['phenotype_meta'];
      $options['pub_id'] = $form_state['ids']['pub_id'];

      switch($genotype['files']['snps-association-type']) {
        case 'P value':
          $options['associations_type'] = tpps_load_cvterm('p_value')->cvterm_id;
          break;

        case 'Genomic Inflation Factor (GIF)':
          $options['associations_type'] = tpps_load_cvterm('lambda')->cvterm_id;
          break;

        case 'P-adjusted (FDR) / Q value':
          $options['associations_type'] = tpps_load_cvterm('q_value')->cvterm_id;
          break;

        case 'P-adjusted (FWE)':
          $options['associations_type'] = tpps_load_cvterm('p_adj_fwe')->cvterm_id;
          break;

        case 'P-adjusted (Bonferroni)':
          $options['associations_type'] = tpps_load_cvterm('bonferroni')->cvterm_id;
          break;

        default:
          break;
      }

      tpps_file_iterator($assoc_fid, 'tpps_process_snp_association', $options);

      $multi_insert_options['fk_overrides']['featureloc'] = array(
        'srcfeature' => array(
          'table' => 'feature',
          'columns' => array(
            'srcfeature_id' => 'feature_id',
          ),
        ),
      );
      $multi_insert_options['fk_overrides']['feature_relationship'] = array(
        'subject' => array(
          'table' => 'feature',
          'columns' => array(
            'subject_id' => 'feature_id',
          ),
        ),
        'object' => array(
          'table' => 'feature',
          'columns' => array(
            'object_id' => 'feature_id',
          ),
        ),
      );

      $pop_struct_fid = $genotype['files']['snps-pop-struct'];
      tpps_add_project_file($form_state, $pop_struct_fid);

      $kinship_fid = $genotype['files']['snps-kinship'];
      tpps_add_project_file($form_state, $kinship_fid);
    }

    tpps_file_iterator($snp_fid, 'tpps_process_genotype_spreadsheet', $options);

    tpps_chado_insert_multi($options['records'], $multi_insert_options);
    $options['records'] = $records;
    $genotype_total += $genotype_count;
    $genotype_count = 0;
  }

  if (!empty($genotype['files']['file-type']['Assay Design']) and $genotype['marker-type']['SNPs']) {
    $design_fid = $genotype['files']['assay-design'];
    tpps_add_project_file($form_state, $design_fid);
  }

  if (!empty($genotype['files']['file-type']['SSRs/cpSSRs Genotype Spreadsheet'])) {
    $ssr_fid = $genotype['files']['ssrs'];
    tpps_add_project_file($form_state, $ssr_fid);

    $options['type'] = 'ssrs';
    $options['headers'] = tpps_ssrs_headers($ssr_fid, $genotype['files']['ploidy']);
    $options['marker'] = $genotype['SSRs/cpSSRs'];
    $options['type_cvterm'] = tpps_load_cvterm('ssr')->cvterm_id;
    $options['empty'] = $genotype['files']['ssrs-empty'];

    tpps_file_iterator($ssr_fid, 'tpps_process_genotype_spreadsheet', $options);

    tpps_chado_insert_multi($options['records'], $multi_insert_options);
    $options['records'] = $records;
    $genotype_count = 0;

    if (!empty($genotype['files']['ssr-extra-check'])) {
      $extra_fid = $genotype['files']['ssrs_extra'];
      tpps_add_project_file($form_state, $extra_fid);

      $options['marker'] = $genotype['files']['extra-ssr-type'];
      $options['headers'] = tpps_ssrs_headers($extra_fid, $genotype['files']['extra-ploidy']);

      tpps_file_iterator($extra_fid, 'tpps_process_genotype_spreadsheet', $options);

      tpps_chado_insert_multi($options['records'], $multi_insert_options);
      $options['records'] = $records;
      $genotype_count = 0;
    }
  }

  if (!empty($genotype['files']['file-type']['Indel Genotype Spreadsheet'])) {
    $indel_fid = $genotype['files']['indels'];
    tpps_add_project_file($form_state, $indel_fid);

    $options['type'] = 'indel';
    $options['headers'] = tpps_file_headers($indel_fid);
    $options['marker'] = 'Indel';
    $options['type_cvterm'] = tpps_load_cvterm('indel')->cvterm_id;

    tpps_file_iterator($indel_fid, 'tpps_process_genotype_spreadsheet', $options);

    tpps_chado_insert_multi($options['records'], $multi_insert_options);
    $options['records'] = $records;
    $genotype_total += $genotype_count;
    $genotype_count = 0;
  }

  if (!empty($genotype['files']['file-type']['Other Marker Genotype Spreadsheet'])) {
    $other_fid = $genotype['files']['other'];
    tpps_add_project_file($form_state, $other_fid);

    $options['headers'] = tpps_file_headers($other_fid);
    if (!empty($genotype['files']['other-groups'])) {
      $groups = $genotype['files']['other-groups'];
      $options['headers'] = tpps_other_marker_headers($other_fid, $groups['Genotype Data'][0]);
      $options['tree_id'] = $groups['Tree Id'][1];
    }

    $options['type'] = 'other';
    $options['marker'] = $genotype['other-marker'];
    $options['type_cvterm'] = tpps_load_cvterm('genetic_marker')->cvterm_id;

    tpps_file_iterator($other_fid, 'tpps_process_genotype_spreadsheet', $options);

    tpps_chado_insert_multi($options['records'], $multi_insert_options);
    $options['records'] = $records;
    $genotype_count = 0;
  }

  if (!empty($genotype['files']['file-type']['VCF'])) {
    // TODO: we probably want to use tpps_file_iterator to parse vcf files.

    $vcf_fid = $genotype['files']['vcf'];
    tpps_add_project_file($form_state, $vcf_fid);

    $marker = 'SNP';

    $records['genotypeprop'] = array();

    $snp_cvterm = tpps_load_cvterm('snp')->cvterm_id;
    $format_cvterm = tpps_load_cvterm('format')->cvterm_id;
    $qual_cvterm = tpps_load_cvterm('quality_value')->cvterm_id;
    $filter_cvterm = tpps_load_cvterm('filter')->cvterm_id;
    $freq_cvterm = tpps_load_cvterm('allelic_frequency')->cvterm_id;
    $depth_cvterm = tpps_load_cvterm('read_depth')->cvterm_id;
    $n_sample_cvterm = tpps_load_cvterm('number_samples')->cvterm_id;

    $vcf_file = file_load($vcf_fid);
    $location = tpps_get_location($vcf_file->uri);
    $vcf_content = fopen($location, 'r');
    $stocks = array();
    $format = "";
    $current_id = $form_state['ids']['organism_ids'][$i];
    $species_code = $species_codes[$current_id];

    // dpm('start: ' . date('r'));.
    while (($vcf_line = fgets($vcf_content)) !== FALSE) {
      if ($vcf_line[0] != '#') {
        $genotype_count++;
        $vcf_line = explode("\t", $vcf_line);
        $scaffold_id = &$vcf_line[0];
        $position = &$vcf_line[1];
        $variant_name = &$vcf_line[2];
        $ref = &$vcf_line[3];
        $alt = &$vcf_line[4];
        $qual = &$vcf_line[5];
        $filter = &$vcf_line[6];
        $info = &$vcf_line[7];

        if (empty($variant_name) or $variant_name == '.') {
          $variant_name = "{$scaffold_id}{$position}$ref:$alt";
        }
        $marker_name = $variant_name . $marker;
        $description = "$ref:$alt";
        $genotype_name = "$marker-$species_code-$scaffold_id-$position";
        $genotype_desc = "$marker-$species_code-$scaffold_id-$position-$description";

        $records['feature'][$marker_name] = array(
          'organism_id' => $current_id,
          'uniquename' => $marker_name,
          'type_id' => $seq_var_cvterm,
        );

        $records['feature'][$variant_name] = array(
          'organism_id' => $current_id,
          'uniquename' => $variant_name,
          'type_id' => $seq_var_cvterm,
        );

        $records['genotype'][$genotype_desc] = array(
          'name' => $genotype_name,
          'uniquename' => $genotype_desc,
          'description' => $description,
          'type_id' => $snp_cvterm,
        );

        if ($format != "") {
          $records['genotypeprop']["$genotype_desc-format"] = array(
            'type_id' => $format_cvterm,
            'value' => $format,
            '#fk' => array(
              'genotype' => $genotype_desc,
            ),
          );
        }

        for ($j = 9; $j < count($vcf_line); $j++) {
          $records['genotype_call']["{$stocks[$j - 9]}-$genotype_name"] = array(
            'project_id' => $project_id,
            'stock_id' => $stocks[$j - 9],
            '#fk' => array(
              'genotype' => $genotype_desc,
              'variant' => $variant_name,
              'marker' => $marker_name,
            ),
          );

          $records['stock_genotype']["{$stocks[$j - 9]}-$genotype_name"] = array(
            'stock_id' => $stocks[$j - 9],
            '#fk' => array(
              'genotype' => $genotype_desc,
            ),
          );
        }

        // Quality score.
        $records['genotypeprop']["$genotype_desc-qual"] = array(
          'type_id' => $qual_cvterm,
          'value' => $qual,
          '#fk' => array(
            'genotype' => $genotype_desc,
          ),
        );

        // filter: pass/fail.
        $records['genotypeprop']["$genotype_desc-filter"] = array(
          'type_id' => $filter_cvterm,
          'value' => ($filter == '.') ? "P" : "NP",
          '#fk' => array(
            'genotype' => $genotype_desc,
          ),
        );

        // Break up info column.
        $info_vals = explode(";", $info);
        foreach ($info_vals as $key => $val) {
          $parts = explode("=", $val);
          unset($info_vals[$key]);
          $info_vals[$parts[0]] = isset($parts[1]) ? $parts[1] : '';
        }

        // Allele frequency, assuming that the info code for allele
        // frequency is 'AF'.
        if (isset($info_vals['AF']) and $info_vals['AF'] != '') {
          $records['genotypeprop']["$genotype_desc-freq"] = array(
            'type_id' => $freq_cvterm,
            'value' => $info_vals['AF'],
            '#fk' => array(
              'genotype' => $genotype_desc,
            ),
          );
        }

        // Depth coverage, assuming that the info code for depth coverage is
        // 'DP'.
        if (isset($info_vals['DP']) and $info_vals['DP'] != '') {
          $records['genotypeprop']["$genotype_desc-depth"] = array(
            'type_id' => $depth_cvterm,
            'value' => $info_vals['DP'],
            '#fk' => array(
              'genotype' => $genotype_desc,
            ),
          );
        }

        // Number of samples, assuming that the info code for number of
        // samples is 'NS'.
        if (isset($info_vals['NS']) and $info_vals['NS'] != '') {
          $records['genotypeprop']["$genotype_desc-n_sample"] = array(
            'type_id' => $n_sample_cvterm,
            'value' => $info_vals['NS'],
            '#fk' => array(
              'genotype' => $genotype_desc,
            ),
          );
        }
        // Tripal Job has issues when all submissions are made at the same
        // time, so break them up into groups of 10,000 genotypes along with
        // their relevant genotypeprops.
        if ($genotype_count > $record_group) {
          $genotype_count = 0;
          tpps_chado_insert_multi($records, $multi_insert_options);
          $records = array(
            'feature' => array(),
            'genotype' => array(),
            'genotype_call' => array(),
            'genotypeprop' => array(),
            'stock_genotype' => array(),
          );
          $genotype_count = 0;
        }
      }
      elseif (preg_match('/##FORMAT=/', $vcf_line)) {
        $format .= substr($vcf_line, 9, -1);
      }
      elseif (preg_match('/#CHROM/', $vcf_line)) {
        $vcf_line = explode("\t", $vcf_line);
        for ($j = 9; $j < count($vcf_line); $j++) {
          $stocks[] = $form_state['tree_info'][trim($vcf_line[$j])]['stock_id'];
        }
      }
    }
    // Insert the last set of values.
    tpps_chado_insert_multi($records, $multi_insert_options);
    unset($records);
    $genotype_count = 0;
    // dpm('done: ' . date('r'));.
  }
}

/**
 * Submits environmental information for one species.
 *
 * @param array $form_state
 *   The TPPS submission object.
 * @param int $i
 *   The organism number we are submitting.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_environment(array &$form_state, $i, &$job = NULL) {
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $environment = $fourthpage["organism-$i"]['environment'] ?? NULL;
  if (empty($environment)) {
    return;
  }

  $env_layers = isset($environment['env_layers']) ? $environment['env_layers'] : FALSE;
  $env_params = isset($environment['env_params']) ? $environment['env_params'] : FALSE;
  $env_count = 0;

  $species_index = "species-$i";
  if (empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
    $species_index = "species-1";
  }
  $tree_accession = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index];
  $tree_acc_fid = $tree_accession['file'];
  if (!empty($form_state['revised_files'][$tree_acc_fid]) and ($file = file_load($form_state['revised_files'][$tree_acc_fid]))) {
    $tree_acc_fid = $form_state['revised_files'][$tree_acc_fid];
  }

  $env_cvterm = tpps_load_cvterm('environment')->cvterm_id;

  if (db_table_exists('cartogratree_layers') and db_table_exists('cartogratree_fields')) {
    $layers_params = array();
    $records = array(
      'phenotype' => array(),
      'phenotype_cvterm' => array(),
      'stock_phenotype' => array(),
    );

    foreach ($env_layers as $layer_name => $layer_id) {
      if ($layer_name == 'other' or $layer_name == 'other_db' or $layer_name = 'other_name' or $layer_name == 'other_params') {
        continue;
      }
      if (!empty($layer_id) and !empty($env_params[$layer_name])) {
        $layers_params[$layer_id] = array();
        $params = $env_params[$layer_name];
        foreach ($params as $param_name => $param_id) {
          if (!empty($param_id)) {
            $layers_params[$layer_id][$param_id] = $param_name;
          }
        }
      }
      elseif (!empty($layer_id) and preg_match('/worldclim_subgroup_(.+)/', $layer_id, $matches)) {
        $subgroup_id = $matches[1];
        $layers = db_select('cartogratree_layers', 'l')
          ->fields('l', array('layer_id'))
          ->condition('subgroup_id', $subgroup_id)
          ->execute();
        while (($layer = $layers->fetchObject())) {
          $params = db_select('cartogratree_fields', 'f')
            ->fields('f', array('field_id', 'display_name'))
            ->condition('layer_id', $layer->layer_id)
            ->execute();
          while (($param = $params->fetchObject())) {
            $layers_params[$layer->layer_id][$param->field_id] = $param->display_name;
          }
        }
      }
    }

    $options = array(
      'no_header' => !empty($tree_accession['file-no-header']),
      'records' => $records,
      'tree_id' => $tree_accession['file-groups']['Tree Id'][1],
      'accession' => $form_state['accession'],
      'tree_info' => $form_state['tree_info'],
      'layers_params' => $layers_params,
      'env_count' => &$env_count,
      'env_cvterm' => $env_cvterm,
      'suffix' => 0,
      'job' => &$job,
    );

    tpps_file_iterator($tree_acc_fid, 'tpps_process_environment_layers', $options);

    tpps_chado_insert_multi($options['records']);
    unset($options['records']);
    $env_count = 0;
  }
}

/**
 * This function will process a row from an accession file.
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_check_organisms($row, array &$options = array()) {
  $cols = $options['cols'];
  $search = &$options['search'];
  $found = &$options['found'];
  $org_full_name = $row[$cols['org']] ?? "{$row[$cols['genus']]} {$row[$cols['species']]}";
  if ($search == $org_full_name) {
    $found = TRUE;
  }
}

/**
 * This function will process a row from a phenotype metadata file.
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_phenotype_meta($row, array &$options = array()) {
  $columns = $options['meta_columns'];
  $meta = &$options['meta'];

  $name = strtolower($row[$columns['name']]);
  $meta[$name] = array();
  $meta[$name]['attr'] = $row[$columns['attr']];
  $meta[$name]['desc'] = $row[$columns['desc']];
  $meta[$name]['unit'] = $row[$columns['unit']];
  if (!empty($columns['struct']) and isset($row[$columns['struct']]) and $row[$columns['struct']] != '') {
    $meta[$name]['struct'] = 'other';
    $meta[$name]['struct-other'] = $row[$columns['struct']];
  }
  if (!empty($columns['min']) and isset($row[$columns['min']]) and $row[$columns['min']] != '') {
    $meta[$name]['min'] = $row[$columns['min']];
  }
  if (!empty($columns['max']) and isset($row[$columns['max']]) and $row[$columns['max']] != '') {
    $meta[$name]['max'] = $row[$columns['max']];
  }
}

/**
 * This function will further refine existing phenotype metadata.
 *
 * The function mostly just adds cvterm ids where applicable.
 *
 * @param array $meta
 *   The existing metadata array.
 * @param array $time_options
 *   The array of options for time-based phenotypes.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_refine_phenotype_meta(array &$meta, array $time_options = array(), &$job = NULL) {
  $cvt_cache = array();
  $local_cv = chado_get_cv(array('name' => 'local'));
  $local_db = variable_get('tpps_local_db');
  foreach ($meta as $name => $data) {
    if ($data['attr'] != 'other') {
      $meta[$name]['attr_id'] = $data['attr'];
    }
    else {
      if (!empty($cvt_cache[$data['attr-other']])) {
        $meta[$name]['attr_id'] = $cvt_cache[$data['attr-other']];
      }
      else {
        $result = tpps_ols_install_term("pato:{$data['attr-other']}");
        if ($result !== FALSE) {
          $meta[$name]['attr_id'] = $result->cvterm_id;
          $job->logMessage("[INFO] New OLS Term pato:{$data['attr-other']} installed");
        }

        if (empty($meta[$name]['attr_id'])) {
          $attr = chado_select_record('cvterm', array('cvterm_id'), array(
            'name' => array(
              'data' => $data['attr-other'],
              'op' => 'LIKE',
            ),
          ), array(
            'limit' => 1,
          ));
          $meta[$name]['attr_id'] = current($attr)->cvterm_id ?? NULL;
        }

        if (empty($meta[$name]['attr_id'])) {
          $meta[$name]['attr_id'] = chado_insert_cvterm(array(
            'id' => "{$local_db->name}:{$data['attr-other']}",
            'name' => $data['attr-other'],
            'definition' => '',
            'cv_name' => $local_cv->name,
          ))->cvterm_id;
          if (!empty($meta[$name]['attr_id'])) {
            $job->logMessage("[INFO] New Local Attribute Term {$data['attr-other']} installed");
          }
        }
        $cvt_cache[$data['attr-other']] = $meta[$name]['attr_id'];
      }
    }

    if ($data['unit'] != 'other') {
      $meta[$name]['unit_id'] = $data['unit'];
    }
    else {
      if (!empty($cvt_cache[$data['unit-other']])) {
        $meta[$name]['unit_id'] = $cvt_cache[$data['unit-other']];
      }
      else {
        $result = tpps_ols_install_term("po:{$data['unit-other']}");
        if ($result !== FALSE) {
          $meta[$name]['unit_id'] = $result->cvterm_id;
          $job->logMessage("[INFO] New OLS Term po:{$data['unit-other']} installed");
        }

        if (empty($meta[$name]['unit_id'])) {
          $obs = chado_select_record('cvterm', array('cvterm_id'), array(
            'name' => array(
              'data' => $data['unit-other'],
              'op' => 'LIKE',
            ),
          ), array(
            'limit' => 1,
          ));
          $meta[$name]['unit_id'] = current($obs)->cvterm_id ?? NULL;
        }

        if (empty($meta[$name]['unit_id'])) {
          $meta[$name]['unit_id'] = chado_insert_cvterm(array(
            'id' => "{$local_db->name}:{$data['unit-other']}",
            'name' => $data['unit-other'],
            'definition' => '',
            'cv_name' => $local_cv->name,
          ))->cvterm_id;
          if (!empty($meta[$name]['unit_id'])) {
            $job->logMessage("[INFO] New Local Unit Term {$data['unit-other']} installed");
          }
        }
        $cvt_cache[$data['unit-other']] = $meta[$name]['unit_id'];
      }
    }

    if ($data['struct'] != 'other') {
      $meta[$name]['struct_id'] = $data['struct'];
    }
    else {
      if (!empty($cvt_cache[$data['struct-other']])) {
        $meta[$name]['struct_id'] = $cvt_cache[$data['struct-other']];
      }
      else {
        $result = tpps_ols_install_term("po:{$data['struct-other']}");
        if ($result !== FALSE) {
          $meta[$name]['struct_id'] = $result->cvterm_id;
          $job->logMessage("[INFO] New OLS Term po:{$data['struct-other']} installed");
        }

        if (empty($meta[$name]['struct_id'])) {
          $obs = chado_select_record('cvterm', array('cvterm_id'), array(
            'name' => array(
              'data' => $data['struct-other'],
              'op' => 'LIKE',
            ),
          ), array(
            'limit' => 1,
          ));
          $meta[$name]['struct_id'] = current($obs)->cvterm_id ?? NULL;
        }

        if (empty($meta[$name]['struct_id'])) {
          $meta[$name]['struct_id'] = chado_insert_cvterm(array(
            'id' => "{$local_db->name}:{$data['struct-other']}",
            'name' => $data['struct-other'],
            'definition' => '',
            'cv_name' => $local_cv->name,
          ))->cvterm_id;
          if (!empty($meta[$name]['struct_id'])) {
            $job->logMessage("[INFO] New Local Structure Term {$data['struct-other']} installed");
          }
        }
        $cvt_cache[$data['struct-other']] = $meta[$name]['struct_id'];
      }
    }

    if (!empty($time_options['time_phenotypes'][strtolower($name)])) {
      $meta[$name]['time'] = $time_options['time_values'][strtolower($name)];
      if (empty($meta[$name]['time'])) {
        $meta[$name]['time'] = TRUE;
      }
    }
  }
}

/**
 * This function will process a row from a phenotype data file.
 *
 * This function is used for standard phenotypes of both phenotype formats, as
 * well as phenotype isotope files. The functionality is slightly different
 * based on the type of phenotype file being processed (set in the options
 * array). This function is meant to be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_phenotype_data($row, array &$options = array()) {
  $iso = $options['iso'] ?? FALSE;
  $records = &$options['records'];
  $meta_headers = $options['meta_headers'] ?? NULL;
  $file_headers = $options['file_headers'] ?? NULL;
  $cvterms = $options['cvterms'];
  $meta = $options['meta'];
  $empty = $options['file-empty'] ?? NULL;
  $accession = $options['accession'];
  $suffix = &$options['suffix'];
  $tree_info = &$options['tree_info'];
  $phenotype_count = &$options['phenotype_count'];
  $record_group = variable_get('tpps_record_group', 10000);

  if (!$iso) {
    if (isset($meta_headers['name']) and (isset($meta_headers['value']))) {
      $id = $row[$meta_headers['value']];
      $values = array($id => $row[$meta_headers['name']]);
    }

    if (!empty($options['data_columns'])) {
      $values = $options['data_columns'];
    }

    $tree_id = $row[$options['tree_id']];
    $clone_col = $meta_headers['clone'] ?? NULL;
    if (isset($clone_col) and !empty($row[$clone_col]) and $row[$clone_col] !== $empty) {
      $tree_id .= "-" . $row[$clone_col];
    }
  }
  else {
    foreach ($row as $id => $value) {
      if (empty($tree_id)) {
        $tree_id = $value;
        continue;
      }
      $values[$id] = $file_headers[$id];
    }
  }

  foreach ($values as $id => $name) {
    $attr_id = $iso ? $meta['attr_id'] : $meta[strtolower($name)]['attr_id'];
    $value = $row[$id];
    $phenotype_name = "$accession-$tree_id-$name-$suffix";
    $options['data']["$tree_id-$name-$suffix"] = array(
      'uniquename' => "$tree_id-$name-$suffix",
      'name' => $name,
      'stock_id' => $tree_info[$tree_id]['stock_id'],
      'time' => NULL,
      'value' => $value,
    );

    $records['phenotype'][$phenotype_name] = array(
      'uniquename' => $phenotype_name,
      'name' => $name,
      'attr_id' => $attr_id,
      'observable_id' => $meta[strtolower($name)]['struct_id'] ?? NULL,
      'value' => $value,
    );

    $records['stock_phenotype'][$phenotype_name] = array(
      'stock_id' => $tree_info[$tree_id]['stock_id'],
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );

    if (isset($meta[strtolower($name)]['time'])) {
      $records['phenotypeprop']["$phenotype_name-time"] = array(
        'type_id' => $cvterms['time'],
        'value' => $meta[strtolower($name)]['time'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
      $options['data'][$phenotype_name]['time'] = $meta[strtolower($name)]['time'];
    }
    elseif (isset($meta_headers['time'])) {
      $val = $row[$meta_headers['time']];
      if (is_int($val)) {
        $val = tpps_xlsx_translate_date($val);
      }
      $records['phenotypeprop']["$phenotype_name-time"] = array(
        'type_id' => $cvterms['time'],
        'value' => $val,
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
      $options['data'][$phenotype_name]['time'] = $val;
    }

    $records['phenotypeprop']["$phenotype_name-desc"] = array(
      'type_id' => $cvterms['desc'],
      'value' => $iso ? $meta['desc'] : $meta[strtolower($name)]['desc'],
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );

    $records['phenotypeprop']["$phenotype_name-unit"] = array(
      'type_id' => $cvterms['unit'],
      'value' => $iso ? $meta['unit'] : $meta[strtolower($name)]['unit_id'],
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );

    if (isset($meta[strtolower($name)]['min'])) {
      $records['phenotypeprop']["$phenotype_name-min"] = array(
        'type_id' => $cvterms['min'],
        'value' => $meta[strtolower($name)]['min'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
    }

    if (isset($meta[strtolower($name)]['max'])) {
      $records['phenotypeprop']["$phenotype_name-max"] = array(
        'type_id' => $cvterms['max'],
        'value' => $meta[strtolower($name)]['max'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
    }

    if ($phenotype_count > $record_group) {
      tpps_chado_insert_multi($records);
      $records = array(
        'phenotype' => array(),
        'phenotypeprop' => array(),
        'stock_phenotype' => array(),
      );
      $phenotype_count = 0;
    }

    $phenotype_count++;
  }
  $suffix++;
}

/**
 * This function processes a single row of a genotype spreadsheet.
 *
 * This function is used for SNP assay files, SSR spreadsheets, and other
 * marker type spreadsheets. The functionality is slightly different based on
 * the type of marker being processed (this is set in the options array). This
 * function is meant to be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_genotype_spreadsheet($row, array &$options = array()) {
  $type = $options['type'];
  $records = &$options['records'];
  $headers = $options['headers'];
  $tree_info = &$options['tree_info'];
  $species_codes = $options['species_codes'];
  $genotype_count = &$options['genotype_count'];
  $genotype_total = &$options['genotype_total'];
  $project_id = $options['project_id'];
  $marker = $options['marker'];
  $type_cvterm = $options['type_cvterm'];
  $seq_var_cvterm = $options['seq_var_cvterm'];
  $multi_insert_options = $options['multi_insert'];
  $associations = $options['associations'] ?? array();

  $record_group = variable_get('tpps_record_group', 10000);
  $stock_id = NULL;

  if (!empty($options['tree_id'])) {
    $val = $row[$options['tree_id']];
    $stock_id = $tree_info[trim($val)]['stock_id'];
    $current_id = $tree_info[trim($val)]['organism_id'];
    $species_code = $species_codes[$current_id];
  }
  foreach ($row as $key => $val) {
    if (empty($headers[$key])) {
      continue;
    }

    if (!isset($stock_id)) {
      $stock_id = $tree_info[trim($val)]['stock_id'];
      $current_id = $tree_info[trim($val)]['organism_id'];
      $species_code = $species_codes[$current_id];
      continue;
    }
    $genotype_count++;

    if ($type == 'ssrs' and !empty($options['empty']) and $val == $options['empty']) {
      continue;
    }

    if ($type == 'ssrs' and ($val === 0 or $val === "0")) {
      $val = "NA";
    }

    $variant_name = $headers[$key];
    $marker_name = $variant_name . $marker;
    $genotype_name = "$marker-$variant_name-$species_code-$val";

    $records['feature'][$marker_name] = array(
      'organism_id' => $current_id,
      'uniquename' => $marker_name,
      'type_id' => $seq_var_cvterm,
    );

    $records['feature'][$variant_name] = array(
      'organism_id' => $current_id,
      'uniquename' => $variant_name,
      'type_id' => $seq_var_cvterm,
    );

    if (!empty($associations) and !empty($associations[$variant_name])) {
      $association = $associations[$variant_name];
      $assoc_feature_name = "{$variant_name}-{$options['associations_type']}-{$association['trait']}";

      $records['feature'][$association['scaffold']] = array(
        'organism_id' => $current_id,
        'uniquename' => $association['scaffold'],
        'type_id' => $options['scaffold_cvterm'],
      );

      $records['feature'][$assoc_feature_name] = array(
        'organism_id' => $current_id,
        'uniquename' => $assoc_feature_name,
        'type_id' => $seq_var_cvterm,
      );

      if (!empty($association['trait_attr'])) {
        $records['feature_cvterm'][$assoc_feature_name] = array(
          'cvterm_id' => $association['trait_attr'],
          'pub_id' => $options['pub_id'],
          '#fk' => array(
            'feature' => $assoc_feature_name,
          ),
        );

        if (!empty($association['trait_obs'])) {
          $records['feature_cvtermprop'][$assoc_feature_name] = array(
            'type_id' => $association['trait_obs'],
            '#fk' => array(
              'feature_cvterm' => $assoc_feature_name,
            ),
          );
        }
      }

      $records['featureprop'][$assoc_feature_name] = array(
        'type_id' => $options['associations_type'],
        '#fk' => array(
          'feature' => $assoc_feature_name,
        ),
      );

      $records['featureloc'][$variant_name] = array(
        'fmin' => $association['start'],
        'fmax' => $association['stop'],
        'residue_info' => $association['allele'],
        '#fk' => array(
          'feature' => $variant_name,
          'srcfeature' => $association['scaffold'],
        ),
      );

      $records['feature_relationship'][$assoc_feature_name] = array(
        'type_id' => $options['associations_type'],
        'value' => $association['confidence'],
        '#fk' => array(
          'subject' => $variant_name,
          'object' => $assoc_feature_name,
        ),
      );
    }

    $records['genotype'][$genotype_name] = array(
      'name' => $genotype_name,
      'uniquename' => $genotype_name,
      'description' => $val,
      'type_id' => $type_cvterm,
    );

    $records['genotype_call']["$stock_id-$genotype_name"] = array(
      'project_id' => $project_id,
      'stock_id' => $stock_id,
      '#fk' => array(
        'genotype' => $genotype_name,
        'variant' => $variant_name,
        'marker' => $marker_name,
      ),
    );

    $records['stock_genotype']["$stock_id-$genotype_name"] = array(
      'stock_id' => $stock_id,
      '#fk' => array(
        'genotype' => $genotype_name,
      ),
    );

    if ($genotype_count >= $record_group) {
      tpps_chado_insert_multi($records, $multi_insert_options);
      $records = array(
        'feature' => array(),
        'genotype' => array(),
        'genotype_call' => array(),
        'stock_genotype' => array(),
      );
      if (!empty($associations)) {
        $records['featureloc'] = array();
        $records['featureprop'] = array();
      }
      $genotype_total += $genotype_count;
      $genotype_count = 0;
    }
  }
}

/**
 * This function processes a single row of a genotype association file.
 *
 * This function is used for SNP association files. This function is meant to
 * be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_snp_association($row, array &$options = array()) {
  $groups = $options['associations_groups'];
  $associations = &$options['associations'];

  $id = $row[$groups['SNP ID'][1]];

  preg_match('/^(\d+):(\d+)$/', $row[$groups['Position'][3]], $matches);
  $start = $matches[1];
  $stop = $matches[2];
  if ($start > $stop) {
    $temp = $start;
    $start = $stop;
    $stop = $temp;
  }

  $trait = $row[$groups['Associated Trait'][5]];

  $associations[$id] = array(
    'id' => $id,
    'scaffold' => $row[$groups['Scaffold'][2]],
    'start' => $start,
    'stop' => $stop,
    'allele' => $row[$groups['Allele'][4]],
    'trait' => $trait,
    'trait_attr' => $options['phenotype_meta'][strtolower($trait)]['attr_id'],
    'trait_obs' => $options['phenotype_meta'][strtolower($trait)]['struct_id'] ?? NULL,
    'confidence' => $row[$groups['Confidence Value'][6]],
  );
}

/**
 * This function formats headers for a microsatellite spreadsheet.
 *
 * SSR/cpSSR spreadsheets will often have blank or duplicate headers, depending
 * on the ploidy of the organism they are meant for. This file standardizes the
 * headers for the spreadsheet so that they can be used with the
 * tpps_process_genotype_spreadsheet() function.
 *
 * @param int $fid
 *   The Drupal managed file id of the file.
 * @param string $ploidy
 *   The ploidy of the organism, as indicated by the user.
 *
 * @return array
 *   The array of standardized headers for the spreadsheet.
 */
function tpps_ssrs_headers($fid, $ploidy) {
  $headers = tpps_file_headers($fid);
  if ($ploidy == 'Haploid') {
    return $headers;
  }
  $row_len = count($headers);
  $results = $headers;

  while (($k = array_search(NULL, $results))) {
    unset($results[$k]);
  }

  $marker_num = 0;
  $first = TRUE;
  reset($headers);
  $num_headers = count($results);
  $num_unique_headers = count(array_unique($results));

  foreach ($headers as $key => $val) {
    next($headers);
    $next_key = key($headers);
    if ($first) {
      $first = FALSE;
      continue;
    }

    switch ($ploidy) {
      case 'Diploid':
        if ($num_headers == ($row_len + 1) / 2) {
          // Every other marker column name is left blank.
          if (array_key_exists($key, $results)) {
            $last = $results[$key];
            $results[$key] .= "_A";
            break;
          }
          $results[$key] = $last . "_B";
          break;
        }
        
        if ($num_headers == $row_len) {
          // All of the marker column names are filled out.
          if ($num_headers != $num_unique_headers) {
            // The marker column names are duplicates, need to append
            // _A and _B.
            if ($results[$key] == $results[$next_key]) {
              $results[$key] .= "_A";
              break;
            }
            $results[$key] .= "_B";
          }
        }
        break;

      case 'Polyploid':
        if ($num_headers == $row_len) {
          // All of the marker column names are filled out.
          if ($num_unique_headers != $num_headers) {
            // The marker column names are duplicates, need to append
            // _1, _2, up to X ploidy.
            // The total number of headers divided by the number of
            // unique headers should be equal to the ploidy.
            $ploidy_suffix = ($marker_num % ($num_headers - 1 / $num_unique_headers - 1)) + 1;
            $results[$key] .= "_$ploidy_suffix";
          }
          $marker_num++;
          break;
        }
        $ploidy_suffix = ($marker_num % ($row_len - 1 / $num_headers - 1)) + 1;
        if (array_key_exists($key, $results)) {
          $last = $results[$key];
          $results[$key] .= "_$ploidy_suffix";
        }
        else {
          $results[$key] = "{$last}_$ploidy_suffix";
        }
        $marker_num++;
        break;

      default:
        break;
    }
  }

  return $results;
}

/**
 * This function formats headers for the "other" type genotype markers.
 *
 * The headers for the "other" genotype marker types are set by the users, so
 * we need to return the names of the headers they have indicated, rather than
 * the values provided in the file-groups array.
 *
 * @param int $fid
 *   The Drupal managed file id of the file.
 * @param array $cols
 *   An array of columns indicating which of the columns contain genotype data.
 *
 * @return array
 *   The array of standardized headers for the spreadsheet.
 */
function tpps_other_marker_headers($fid, array $cols) {
  $headers = tpps_file_headers($fid);
  $results = array();
  foreach ($cols as $col) {
    $results[$col] = $headers[$col];
  }
  return $results;
}

/**
 * This function processes a single row of a plant accession file.
 *
 * This function populates the db with environmental data provided through
 * CartograPlant layers. This function is meant to be used with
 * tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_environment_layers($row, array &$options = array()) {
  $id_col = $options['tree_id'];
  $records = &$options['records'];
  $tree_info = &$options['tree_info'];
  $layers_params = $options['layers_params'];
  $env_count = &$options['env_count'];
  $accession = $options['accession'];
  $suffix = &$options['suffix'];
  $env_cvterm = $options['env_cvterm'];
  $record_group = variable_get('tpps_record_group', 10000);

  $tree_id = $row[$id_col];
  $stock_id = $tree_info[$tree_id]['stock_id'];

  $gps_query = chado_select_record('stockprop', array('value'), array(
    'stock_id' => $stock_id,
    'type_id' =>tpps_load_cvterm('gps_latitude')->cvterm_id,
  ), array(
    'limit' => 1,
  ));
  $lat = current($gps_query)->value;

  $gps_query = chado_select_record('stockprop', array('value'), array(
    'stock_id' => $stock_id,
    'type_id' => tpps_load_cvterm('gps_longitude')->cvterm_id,
  ), array(
    'limit' => 1,
  ));
  $long = current($gps_query)->value;

  foreach ($layers_params as $layer_id => $params) {
    $layer_query = db_select('cartogratree_layers', 'l')
      ->fields('l', array('title'))
      ->condition('layer_id', $layer_id)
      ->execute();

    $layer_name = $layer_query->fetchObject()->title;

    foreach ($params as $param_id => $param) {
      $param_query = db_select('cartogratree_fields', 'f')
        ->fields('f', array('field_name'))
        ->condition('field_id', $param_id)
        ->execute();

      $param_name = $param_query->fetchObject()->field_name;
      $phenotype_name = "$accession-$tree_id-$layer_name-$param_name-$suffix";

      $value = tpps_get_environmental_layer_data($layer_id, $lat, $long, $param_name);
      $type = variable_get("tpps_param_{$param_id}_type", 'attr_id');

      if ($type == 'attr_id') {
        $records['phenotype'][$phenotype_name] = array(
          'uniquename' => $phenotype_name,
          'name' => $param_name,
          'attr_id' => $env_cvterm,
          'value' => $value,
        );

        $records['stock_phenotype'][$phenotype_name] = array(
          'stock_id' => $stock_id,
          '#fk' => array(
            'phenotype' => $phenotype_name,
          ),
        );
      }
      else {
        $records['phenotype'][$phenotype_name] = array(
          'uniquename' => $phenotype_name,
          'name' => "$param_name",
          'value' => "$value",
        );

        $records['phenotype_cvterm'][$phenotype_name] = array(
          'cvterm_id' => $env_cvterm,
          '#fk' => array(
            'phenotype' => $phenotype_name,
          ),
        );

        $records['stock_phenotype'][$phenotype_name] = array(
          'stock_id' => $stock_id,
          '#fk' => array(
            'phenotype' => $phenotype_name,
          ),
        );
      }

      $env_count++;
      if ($env_count >= $record_group) {
        tpps_chado_insert_multi($records);
        $records = array(
          'phenotype' => array(),
          'phenotype_cvterm' => array(),
          'stock_phenotype' => array(),
        );
        $env_count = 0;
      }
    }
  }
  $suffix++;
}

/**
 * This function parses and returns a data point from a CartograPlant layer.
 *
 * The data point for the layer at the specified location is obtained by calling
 * tpps_get_env_response, and the resulting response string is parsed to return
 * the specified parameter.
 *
 * @param int $layer_id
 *   The identifier of the CartograPlant environmental layer.
 * @param float $lat
 *   The latitude coordinate being queried.
 * @param float $long
 *   The longitude coordinate being queried.
 * @param string $param
 *   The name of the parameter type.
 *
 * @return mixed
 *   The parsed environmental data. If no valid data was found, return NULL.
 */
function tpps_get_environmental_layer_data($layer_id, $lat, $long, $param) {

  $response = tpps_get_env_response($layer_id, $lat, $long);
  if (($response = explode("\n", $response))) {
    $response = array_slice($response, 2, -2);
    foreach ($response as $line) {
      if (($item = explode("=", $line)) and trim($item[0]) == $param) {
        return trim($item[1]);
      }
    }
  }
  return NULL;
}

/**
 * This function loads data for a CartograPlant layer at a lat/long coordinate.
 *
 * @param int $layer_id
 *   The identifier of the CartograPlant environmental layer.
 * @param float $lat
 *   The latitude coordinate being queried.
 * @param float $long
 *   The longitude coordinate being queried.
 *
 * @return string
 *   The environmental data for that layer at that lat/long coordinate.
 */
function tpps_get_env_response($layer_id, $lat, $long) {
  if (db_table_exists('cartogratree_layers')) {
    $query = db_select('cartogratree_layers', 'l')
      ->fields('l', array('name'))
      ->condition('layer_id', $layer_id)
      ->execute();

    $result = $query->fetchObject();
    $layers = $result->name;

    $url = "http://treegenesdev.cam.uchc.edu:8080/geoserver/ct/wms?";
    $serv = "WMS";
    $ver = "1.3.0";
    $req = "GetFeatureInfo";
    $srs = "EPSG:4326";
    $format = "application/json";
    $bigger_lat = $lat + 0.0000001;
    $bigger_long = $long + 0.0000001;
    $bbox = "$lat,$long,$bigger_lat,$bigger_long";
    $pixels = "width=1&height=1&X=0&Y=0";

    $url .= "service=$serv&version=$ver&request=$req&layers=$layers&srs=$srs&format=$format&query_layers=$layers&bbox=$bbox&$pixels";

    return file_get_contents($url);
  }
}

/**
 * Submits additional data provided in the summary page to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 */
function tpps_submit_summary(array &$form_state) {
  $analysis_options = array(
    'diversity' => 'Diversity',
    'population_structure' => 'Population Structure',
    'association_genetics' => 'Association Genetics',
    'landscape_genomics' => 'Landscape Genomics',
    'phenotype_environment' => 'Phenotype-Environment',
  );

  foreach ($analysis_options as $option => $label) {
    if (!empty($form_state['saved_values']['summarypage']['analysis']["{$option}_check"])) {
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('analysis_type')->cvterm_id,
        'value' => $label,
      ));

      $fid = $form_state['saved_values']['summarypage']['analysis']["{$option}_file"];
      if (!empty($fid)) {
        tpps_add_project_file($form_state, $fid);

        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('source_description')->cvterm_id,
          'value' => $form_state['saved_values']['summarypage']['analysis']["{$option}_file_description"],
        ));
      }
    }
  }

  if (!empty($form_state['saved_values']['summarypage']['tree_pictures'])) {
    foreach ($form_state['saved_values']['summarypage']['tree_pictures'] as $name => $fid) {
      if (substr($name, -4) == '_url' or substr($name, -12) == '_attribution' or substr($name, -8) == '_license') {
        continue;
      }
      if (!empty($fid)) {
        $form_state['file_info']['summarypage'][$fid] = implode('_', explode(' ', $name)) . '.jpg';
        if (db_table_exists('treepictures_metadata')) {
          db_insert('treepictures_metadata')
            ->fields(array('species', 'source', 'attribution', 'license'))
            ->values(array(
              'species' => $form_state['file_info']['summarypage'][$fid],
              'source' => $form_state['saved_values']['summarypage']['tree_pictures']["{$name}_url"],
              'attribution' => $form_state['saved_values']['summarypage']['tree_pictures']["{$name}_attribution"],
              'license' => $form_state['saved_values']['summarypage']['tree_pictures']["{$name}_license"],
            ))
            ->execute();
        }
      }
    }
  }
}

/**
 * This function processes a single row of an accession file.
 *
 * This function is meant to be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_accession($row, array &$options) {
  $cvterm = $options['cvterms'];
  $records = &$options['records'];
  $accession = $options['accession'];
  $cols = $options['column_ids'];
  $saved_ids = &$options['saved_ids'];
  $stock_count = &$options['stock_count'];
  $multi_insert_options = $options['multi_insert'];
  $tree_info = &$options['tree_info'];
  $record_group = variable_get('tpps_record_group', 10000);
  $geo_api_key = variable_get('tpps_geocode_api_key', NULL);
  $site_based = FALSE;
  $exact = $options['exact'] ?? NULL;
  $precision = $options['precision'] ?? NULL;

  $tree_id = $row[$cols['id']];
  $id = $saved_ids['organism_ids'][$options['org_num']];
  if ($options['org_names']['number'] != 1 and $options['single_file']) {
    $org_full_name = $row[$cols['org']] ?? "{$row[$cols['genus']]} {$row[$cols['species']]}";
    $id = $saved_ids['organism_ids'][array_search($org_full_name, $options['org_names'])];
  }

  $records['stock'][$tree_id] = array(
    'uniquename' => "$accession-$tree_id",
    'type_id' => $cvterm['org'],
    'organism_id' => $id,
  );
  $tree_info[$tree_id] = array(
    'organism_id' => $id,
  );

  $records['project_stock'][$tree_id] = array(
    'project_id' => $saved_ids['project_id'],
    '#fk' => array(
      'stock' => $tree_id,
    ),
  );

  if (isset($row[$cols['clone']]) and $row[$cols['clone']] !== $options['empty']) {
    $clone_name = $tree_id . '-' . $row[$cols['clone']];

    $records['stock'][$clone_name] = array(
      'uniquename' => $accession . '-' . $clone_name,
      'type_id' => $cvterm['clone'],
      'organism_id' => $id,
    );
    $tree_info[$clone_name] = array(
      'organism_id' => $id,
    );

    $records['project_stock'][$clone_name] = array(
      'project_id' => $saved_ids['project_id'],
      '#fk' => array(
        'stock' => $clone_name,
      ),
    );

    $records['stock_relationship'][$clone_name] = array(
      'type_id' => $cvterm['has_part'],
      '#fk' => array(
        'subject' => $tree_id,
        'object' => $clone_name,
      ),
    );

    $tree_id = $clone_name;
  }

  if (!empty($row[$cols['lat']]) and !empty($row[$cols['lng']])) {
    $raw_coord = $row[$cols['lat']] . ',' . $row[$cols['lng']];
    $standard_coord = explode(',', tpps_standard_coord($raw_coord));
    $lat = $standard_coord[0];
    $lng = $standard_coord[1];
  }
  elseif (!empty($row[$cols['state']]) and !empty($row[$cols['country']])) {
    $exact = FALSE;
    $records['stockprop']["$tree_id-country"] = array(
      'type_id' => $cvterm['country'],
      'value' => $row[$cols['country']],
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );

    $records['stockprop']["$tree_id-state"] = array(
      'type_id' => $cvterm['state'],
      'value' => $row[$cols['state']],
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );

    $location = "{$row[$cols['state']]}, {$row[$cols['country']]}";

    if (!empty($row[$cols['county']])) {
      $records['stockprop']["$tree_id-county"] = array(
        'type_id' => $cvterm['county'],
        'value' => $row[$cols['county']],
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );
      $location = "{$row[$cols['county']]}, $location";
    }

    if (!empty($row[$cols['district']])) {
      $records['stockprop']["$tree_id-district"] = array(
        'type_id' => $cvterm['district'],
        'value' => $row[$cols['district']],
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );
      $location = "{$row[$cols['district']]}, $location";
    }

    $tree_info[$tree_id]['location'] = $location;

    if (isset($geo_api_key)) {
      if (!array_key_exists($location, $options['locations'])) {
        $query = urlencode($location);
        $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
        $response = json_decode(file_get_contents($url));

        if ($response->total_results) {
          $results = $response->results;
          $result = $results[0]->geometry;
          if ($response->total_results > 1 and !isset($cols['district']) and !isset($cols['county'])) {
            foreach ($results as $item) {
              if ($item->components->_type == 'state') {
                $result = $item->geometry;
                break;
              }
            }
          }
        }
        $options['locations'][$location] = $result ?? NULL;
      }
      else {
        $result = $options['locations'][$location];
      }

      if (!empty($result)) {
        $lat = $result->lat;
        $lng = $result->lng;
      }
    }
  }
  elseif (!empty($row[$cols['pop_group']])) {
    $site_based = TRUE;
    $location = $options['pop_group'][$row[$cols['pop_group']]];
    $coord = tpps_standard_coord($location);

    if ($coord) {
      $parts = explode(',', $coord);
      $lat = $parts[0];
      $lng = $parts[1];
    }
    else {
      $records['stockprop']["$tree_id-location"] = array(
        'type_id' => $cvterm['loc'],
        'value' => $location,
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );

      $tree_info[$tree_id]['location'] = $location;

      if (isset($geo_api_key)) {
        if (!array_key_exists($location, $options['locations'])) {
          $query = urlencode($location);
          $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
          $response = json_decode(file_get_contents($url));
          $result = ($response->total_results) ? $response->results[0]->geometry : NULL;
          $options['locations'][$location] = $result;
        }
        else {
          $result = $options['locations'][$location];
        }

        if (!empty($result)) {
          $lat = $result->lat;
          $lng = $result->lng;
        }
      }
    }
  }

  if (!empty($lat) and !empty($lng)) {
    $records['stockprop']["$tree_id-lat"] = array(
      'type_id' => $cvterm['lat'],
      'value' => $lat,
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );

    $records['stockprop']["$tree_id-long"] = array(
      'type_id' => $cvterm['lng'],
      'value' => $lng,
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );
    $tree_info[$tree_id]['lat'] = $lat;
    $tree_info[$tree_id]['lng'] = $lng;

    $gps_type = "Site-based";
    if (!$site_based) {
      $gps_type = "Exact";
      if (!$exact) {
        $gps_type = "Approximate";
      }
    }

    $records['stockprop']["$tree_id-gps-type"] = array(
      'type_id' => $cvterm['gps_type'],
      'value' => $gps_type,
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );

    if ($gps_type == "Approximate" and !empty($precision)) {
      $records['stockprop']["$tree_id-precision"] = array(
        'type_id' => $cvterm['precision'],
        'value' => $precision,
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );
    }
  }

  $stock_count++;
  if ($stock_count >= $record_group) {
    $new_ids = tpps_chado_insert_multi($records, $multi_insert_options);
    foreach ($new_ids as $t_id => $stock_id) {
      $tree_info[$t_id]['stock_id'] = $stock_id;
    }

    $records = array(
      'stock' => array(),
      'stockprop' => array(),
      'stock_relationship' => array(),
      'project_stock' => array(),
    );
    $stock_count = 0;
  }
}

/**
 * Cleans unnecessary information from the form state.
 *
 * Uses tpps_form_state_info() as a helper function.
 *
 * @param array $form_state
 *   The form state to be cleaned.
 */
function tpps_clean_state(array &$form_state) {
  $new = array();
  unset($form_state['ids']);
  tpps_form_state_info($new, $form_state);
  $form_state = $new;
}
