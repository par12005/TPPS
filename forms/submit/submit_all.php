<?php

/**
 * @file
 * Defines function tpps_submit_all and its helper functions.
 *
 * The functions defined in this file do not actually submit the genotype,
 * phenotype, or environmental data collected from page 4. That data is instead
 * submitted through a Tripal job due to the size of the data.
 */

 // Global variables
 $tpps_job_logger = NULL;
 $tpps_job = NULL;


/**
 * Creates a record for the project and calls the submission helper functions.
 *
 * @param string $accession
 *   The accession number of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_all($accession, TripalJob $job = NULL) {
  global $tpps_job;
  $tpps_job = $job;
  // Get public path
  $log_path = drupal_realpath('public://') . '/tpps_job_logs/';

  mkdir($log_path);

  // Update the global $tpps_job_logger variable
  global $tpps_job_logger;
  $tpps_job_logger = [];
  $tpps_job_logger['job_object'] = $job;
  $tpps_job_logger['log_file_path'] =  $log_path . $accession . '_' . $tpps_job_logger['job_object']->getJobID() . '.txt';
  $tpps_job_logger['log_file_handle'] = fopen($tpps_job_logger['log_file_path'], "w+");

  tpps_job_logger_write('[INFO] Setting up...');
  $job->logMessage('[INFO] Setting up...');
  $job->setInterval(1);
  $form_state = tpps_load_submission($accession);

  // PATCH to check if VCF exists, then remove the assay design
  // Advised by Meghan 7/6/2023
  

  // Do check for missing files, this is a bit crude but it works
  $display_results = strip_tags(tpps_table_display($form_state));
  // Find 'missing file' string
  echo $display_results;
  if(stripos($display_results, 'file missing') !== FALSE) {
    $display_results_lines = explode("\n", $display_results);
    foreach ($display_results_lines as $line) {
      if (stripos($line, 'file missing') !== FALSE) {
        tpps_job_logger_write('[FATAL] ' . $line . "\n");
        $job->logMessage('[FATAL] ' . $line . "\n");
      }
    }
    throw new Exception('Detected a missing file, please ensure missing files are resolved in the tpps-admin-panel and then rerun the study');
  }

  $form_state['status'] = 'Submission Job Running';
  tpps_update_submission($form_state, array('status' => 'Submission Job Running'));
  
  // RISH 7/18/2023 - Run some checks before going through most of the genotype
  // processing (this will error out if an issue is found to avoid long failing loads)
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
  for ($i = 1; $i <= $organism_number; $i++) {
    tpps_genotype_initial_checks($form_state, $i, $job);
  }
  
  
  $transaction = db_transaction();

  try {
    tpps_job_logger_write('[INFO] Clearing Database...');
    $job->logMessage('[INFO] Clearing Database...');
    tpps_submission_clear_db($accession);
    tpps_job_logger_write('[INFO] Database Cleared');
    $job->logMessage('[INFO] Database Cleared.');
    $project_id = $form_state['ids']['project_id'] ?? NULL;

    $form_state = tpps_load_submission($accession);
    tpps_clean_state($form_state);
    tpps_submission_clear_default_tags($accession);
    $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
    $form_state['file_rank'] = 0;
    $form_state['ids'] = array();

    tpps_job_logger_write('[INFO] Creating project record...');
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
    tpps_job_logger_write("[INFO] Project record created. project_id: @pid\n", array('@pid' => $form_state['ids']['project_id']));
    $job->logMessage("[INFO] Project record created. project_id: @pid\n", array('@pid' => $form_state['ids']['project_id']));

    tpps_tripal_entity_publish('Project', array(
      $firstpage['publication']['title'],
      $form_state['ids']['project_id'],
    ));

    tpps_job_logger_write("[INFO] Submitting Publication/Species information...");
    $job->logMessage("[INFO] Submitting Publication/Species information...");
    tpps_submit_page_1($form_state, $job);
    tpps_job_logger_write("[INFO] Publication/Species information submitted!\n");
    $job->logMessage("[INFO] Publication/Species information submitted!\n");

    tpps_job_logger_write("[INFO] Submitting Study Details...");
    $job->logMessage("[INFO] Submitting Study Details...");
    tpps_submit_page_2($form_state, $job);
    tpps_job_logger_write("[INFO] Study Details sumbitted!\n");
    $job->logMessage("[INFO] Study Details sumbitted!\n");

    tpps_job_logger_write("[INFO] Submitting Accession information...");
    $job->logMessage("[INFO] Submitting Accession information...");
    tpps_submit_page_3($form_state, $job);
    tpps_job_logger_write("[INFO] Accession information submitted!\n");
    $job->logMessage("[INFO] Accession information submitted!\n");

    tpps_job_logger_write("[INFO] Submitting Raw data...");
    $job->logMessage("[INFO] Submitting Raw data...");
    tpps_submit_page_4($form_state, $job);
    tpps_job_logger_write("[INFO] Raw data submitted!\n");
    $job->logMessage("[INFO] Raw data submitted!\n");

    tpps_job_logger_write("[INFO] Submitting Summary information...");
    $job->logMessage("[INFO] Submitting Summary information...");
    tpps_submit_summary($form_state);
    tpps_job_logger_write("[INFO] Summary information submitted!\n");
    $job->logMessage("[INFO] Summary information submitted!\n");

    tpps_update_submission($form_state);

    tpps_job_logger_write("[INFO] Renaming files...");
    $job->logMessage("[INFO] Renaming files...");
    tpps_submission_rename_files($accession);
    tpps_job_logger_write("[INFO] Files renamed!\n");
    $job->logMessage("[INFO] Files renamed!\n");
    $form_state = tpps_load_submission($accession);
    $form_state['status'] = 'Approved';
    $form_state['loaded'] = time();
    tpps_job_logger_write("[INFO] Finishing up...");
    $job->logMessage("[INFO] Finishing up...");
    tpps_update_submission($form_state, array('status' => 'Approved'));
    tpps_job_logger_write("[INFO] Complete!");
    $job->logMessage("[INFO] Complete!");

    fclose($tpps_job_logger['log_file_handle']);

  }
  catch (Exception $e) {
    $transaction->rollback();
    $form_state = tpps_load_submission($accession);
    $form_state['status'] = 'Pending Approval';
    tpps_update_submission($form_state, array('status' => 'Pending Approval'));

    tpps_job_logger_write('[ERROR] Job failed');
    $job->logMessage('[ERROR] Job failed', array(), TRIPAL_ERROR);
    tpps_job_logger_write('[ERROR] Error message: @msg', array('@msg' => $e->getMessage()));
    $job->logMessage('[ERROR] Error message: @msg', array('@msg' => $e->getMessage()), TRIPAL_ERROR);
    tpps_job_logger_write("[ERROR] Trace: \n@trace", array('@trace' => $e->getTraceAsString()));
    $job->logMessage("[ERROR] Trace: \n@trace", array('@trace' => $e->getTraceAsString()), TRIPAL_ERROR);

    fclose($tpps_job_logger['log_file_handle']);
    watchdog_exception('tpps', $e);
    throw new Exception('Job failed.');
  }
}

/**
 * Writes data to the tpps_job_logger_handle
 *
 * @param string $string
 *   Write string to the job log file using the tpps_job_logger object
 */
function tpps_job_logger_write($string, $replacements = []) {
  global $tpps_job_logger;
  try {
    foreach ($replacements as $key_string => $replace_string) {
      $string = str_replace($key_string, $replace_string, $string);
    }

    // Add timestamp
    $time_now = time();
    $timestamp_now = date('m/d/y g:i:s A', $time_now);

    $string = "\n" . $timestamp_now . " " . $string;

    fwrite($tpps_job_logger['log_file_handle'],$string);
    fflush($tpps_job_logger['log_file_handle']);
  }
  catch (Exception $e) {
    print_r($e->getMessage());
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
function tpps_submit_page_1(array &$form_state, TripalJob &$job = NULL) {

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
      if(!empty($seconds[$i]) || $seconds[$i] != "") {
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
      else {
        tpps_job_logger_write('[INFO] - Secondary publishers error - found an empty secondary publisher name. Ignoring this input.');
        $job->logMessage('[INFO] - Secondary publishers error - found an empty secondary publisher name. Ignoring this input.');
        // throw new Exception("Seconds[$i]" . $seconds[$i]);
      }
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
  tpps_tripal_entity_publish('Publication', array(
    $firstpage['publication']['title'],
    $publication_id,
  ));
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
      if (!empty($thirdpage['tree-accession']['check']) and empty($thirdpage['tree-accession']["species-$i"]['file'])) {
        continue;
      }

      if (empty($thirdpage['tree-accession']['check'])) {
        $options = array(
          'cols' => array(),
          'search' => $firstpage['organism'][$i]['name'],
          'found' => FALSE,
        );
        $tree_accession = $thirdpage['tree-accession']["species-1"];
        $groups = $tree_accession['file-groups'];
        if ($groups['Genus and Species']['#type'] == 'separate') {
          $options['cols']['genus'] = $groups['Genus and Species']['6'];
          $options['cols']['species'] = $groups['Genus and Species']['7'];
        }
        if ($groups['Genus and Species']['#type'] != 'separate') {
          $options['cols']['org'] = $groups['Genus and Species']['10'];
        }
        $fid = $tree_accession['file'];
        tpps_file_iterator($fid, 'tpps_check_organisms', $options);
        if (!$options['found']) {
          continue;
        }
      }
    }

    $code_exists = tpps_chado_prop_exists('organism', $form_state['ids']['organism_ids'][$i], 'organism 4 letter code');

    if (!$code_exists) {
      foreach (tpps_get_species_codes($genus, $species) as $trial_code) {
        $new_code_query = chado_select_record('organismprop', array('value'), array(
          'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
          'value' => $trial_code,
        ));

        if (empty($new_code_query)) {
          break;
        }
      }

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

    tpps_tripal_entity_publish('Organism', array(
      "$genus $species",
      $form_state['ids']['organism_ids'][$i],
    ));
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
function tpps_submit_page_2(array &$form_state, TripalJob &$job = NULL) {

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
        $record_next = ((bool) $value);
        $description = TRUE;
        continue;
      }
      if ($record_next) {
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

/**
 * Submits Plant Accession data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_page_3(array &$form_state, TripalJob &$job = NULL) {
  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];
  $organism_number = $firstpage['organism']['number'];
  $form_state['locations'] = array();
  $form_state['tree_info'] = array();
  $stock_count = 0;
  $loc_name = 'Location (latitude/longitude or country/state or population group)';

  if (!empty($thirdpage['skip_validation'])) {
    tpps_submission_add_tag($form_state['accession'], 'No Location Information');
  }

  if (!empty($thirdpage['study_location'])) {
    $type = $thirdpage['study_location']['type'];
    $locs = $thirdpage['study_location']['locations'];
    $geo_api_key = variable_get('tpps_geocode_api_key', NULL);

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
        continue;
      }
      $loc = $locs[$i];
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('experiment_location')->cvterm_id,
        'value' => $loc,
        'rank' => $i,
      ));

      if (isset($geo_api_key)) {
        $query = urlencode($loc);
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
      if (!array_key_exists(tpps_get_tag_id('No Location Information'), tpps_submission_get_tags($form_state['accession']))) {
        tpps_submission_add_tag($form_state['accession'], 'Approximate Coordinates');
      }
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
      if ($groups['Genus and Species']['#type'] != 'separate') {
        $options['column_ids']['org'] = $groups['Genus and Species']['10'];
      }
    }
    tpps_job_logger_write('[INFO] - Processing accession file data...');
    $job->logMessage('[INFO] - Processing accession file data...');
    tpps_file_iterator($fid, 'tpps_process_accession', $options);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');

    tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
    $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
    $new_ids = tpps_chado_insert_multi($options['records'], $multi_insert_options);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');
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
function tpps_submit_page_4(array &$form_state, TripalJob &$job = NULL) {
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
          drupal_set_message(t('Cannot submit import: @msg', array('@msg' => $ex->getMessage())), 'error');
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
          'linked_records' => $eutils['options']['linked_records'],
        );

        try {
          $importer = new $class();
          $importer->create($run_args);
          $importer->submitJob();
        }
        catch (Exception $ex) {
          drupal_set_message(t('Cannot submit BioProject: @msg', array('@msg' => $ex->getMessage())), 'error');
        }
      }
    }
  }

  $form_state['data']['phenotype'] = array();
  $form_state['data']['phenotype_meta'] = array();

  // Submit raw data.
  for ($i = 1; $i <= $organism_number; $i++) {
    tpps_submit_phenotype($form_state, $i, $job);
    echo "[DEBUG] Processing genotype data for species code $species_codes and i = $i\n";
    tpps_submit_genotype($form_state, $species_codes, $i, $job);
    tpps_submit_environment($form_state, $i, $job);
  }
  // Generate genotype view
  $test = false;
  $project_id = $form_state['ids']['project_id'];
  if (isset($project_id) && $test != true) {
    tpps_generate_genotype_materialized_view($project_id);
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
function tpps_submit_phenotype(array &$form_state, $i, TripalJob &$job = NULL) {
  tpps_job_logger_write('[INFO] - Submitting phenotype data...');
  $job->logMessage('[INFO] - Submitting phenotype data...');
  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $phenotype = $fourthpage["organism-$i"]['phenotype'] ?? NULL;
  $organism_name = $firstpage['organism'][$i]['name'];
  if (empty($phenotype)) {
    return;
  }
  tpps_submission_add_tag($form_state['accession'], 'Phenotype');

  // Get appropriate cvterms.
  $phenotype_cvterms = array(
    'time' => tpps_load_cvterm('time')->cvterm_id,
    'desc' => tpps_load_cvterm('description')->cvterm_id,
    'unit' => tpps_load_cvterm('unit')->cvterm_id,
    'min' => tpps_load_cvterm('minimum')->cvterm_id,
    'max' => tpps_load_cvterm('maximum')->cvterm_id,
    'environment' => tpps_load_cvterm('environment')->cvterm_id,
    'intensity' => tpps_load_cvterm('intensity')->cvterm_id,
  );

  $records = array(
    'phenotype' => array(),
    'phenotypeprop' => array(),
    'stock_phenotype' => array(),
    'phenotype_cvterm' => array(),
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

  if (!empty($phenotype['normal-check'])) {
    $phenotype_number = $phenotype['phenotypes-meta']['number'];
    $phenotypes_meta = array();
    $data_fid = $phenotype['file'];
    $phenos_edit = $form_state['phenotypes_edit'] ?? NULL;

    tpps_add_project_file($form_state, $data_fid);

    $env_phenotypes = FALSE;
    // Populate $phenotypes_meta with manually entered metadata.
    for ($j = 1; $j <= $phenotype_number; $j++) {
      $name = strtolower($phenotype['phenotypes-meta'][$j]['name']);
      if (!empty($phenos_edit[$j])) {
        // (Rish) BUGFIX related to sex -> age
        // keep track of the cvterm id
        $cvterm_id = $phenotype['phenotypes-meta'][$j]['attribute'];
        $result = $phenos_edit[$j] + $phenotype['phenotypes-meta'][$j];
        $phenotype['phenotypes-meta'][$j] = $result;
        // restore the cvterm_id from the original (since this is from verified cvterm table which populated the select list dropdown box on tpps form)
        $phenotype['phenotypes-meta'][$j]['attribute'] = $cvterm_id;
      }
      $phenotypes_meta[$name] = array();
      $phenotypes_meta[$name]['attr'] = $phenotype['phenotypes-meta'][$j]['attribute'];
      // print_r('LINE 1022:');
      // print_r($phenotype['phenotypes-meta'][$j]);
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
      if (!empty($phenotype['phenotypes-meta'][$j]['val-check']) or !empty($phenotype['phenotypes-meta'][$j]['bin-check'] or $phenotype['phenotypes-meta'][$j]['units'] == tpps_load_cvterm('boolean')->cvterm_id)) {
        $phenotypes_meta[$name]['min'] = $phenotype['phenotypes-meta'][$j]['min'];
        $phenotypes_meta[$name]['max'] = $phenotype['phenotypes-meta'][$j]['max'];
      }
      $phenotypes_meta[$name]['env'] = !empty($phenotype['phenotypes-meta'][$j]['env-check']);
      if ($phenotypes_meta[$name]['env']) {
        $env_phenotypes = TRUE;
      }
    }
    if ($env_phenotypes) {
      tpps_submission_add_tag($form_state['accession'], 'Environment');
    }

    // throw new Exception('$phenotype[check]:' . $phenotype['check'] . "\n");
    if ($phenotype['check'] == '1' || $phenotype['check'] == 'upload_file') {
      $meta_fid = intval($phenotype['metadata']);
      print_r('META_FID:' . $meta_fid . "\n");
      // Added because TGDR009 META FID was 0 which caused failures
      if ($meta_fid > 0) {

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
        // print_r($columns);

        $meta_options = array(
          'no_header' => $phenotype['metadata-no-header'],
          'meta_columns' => $columns,
          'meta' => &$phenotypes_meta,
        );

        tpps_job_logger_write('[INFO] - Processing phenotype_meta file data...');
        $job->logMessage('[INFO] - Processing phenotype_meta file data...');
        tpps_file_iterator($meta_fid, 'tpps_process_phenotype_meta', $meta_options);
        tpps_job_logger_write('[INFO] - Done.');
        $job->logMessage('[INFO] - Done.');
      }
      else {
        tpps_job_logger_write('[WARNING] - phenotype_meta file id looks incorrect but the UI checkbox was selected. Need to double check this!');
      }
    }

    // print_r("Phenotypes Meta:\n");
    // print_r($phenotypes_meta);
    // print_r("\n");

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
      if(is_array($groups['Phenotype Data']['0']) && !empty($groups['Phenotype Data']['0'])) {
        foreach ($groups['Phenotype Data']['0'] as $col) {
          $data_columns[$col] = $file_headers[$col];
        }
      }
      else {
        $col = $groups['Phenotype Data'][0];
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
    $options['organism_name'] = $organism_name;

    print_r('DATA_FID:' . $data_fid . "\n");
    tpps_job_logger_write('[INFO] - Processing phenotype_data file data...');
    $job->logMessage('[INFO] - Processing phenotype_data file data...');
    tpps_file_iterator($data_fid, 'tpps_process_phenotype_data', $options);
    $form_state['data']['phenotype_meta'] += $phenotypes_meta;
    tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
    $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
    // print_r($options['records']);
    tpps_chado_insert_multi($options['records']);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');
  }

  if (!empty($phenotype['iso-check'])) {
    $iso_fid = $phenotype['iso'];
    tpps_add_project_file($form_state, $iso_fid);

    $options['iso'] = TRUE;
    $options['records'] = $records;
    $options['cvterms'] = $phenotype_cvterms;
    $options['file_headers'] = tpps_file_headers($iso_fid);
    $options['organism_name'] = $organism_name;
    $options['meta'] = array(
      'desc' => "Mass Spectrometry",
      'unit' => "intensity (arbitrary units)",
      'attr_id' => tpps_load_cvterm('intensity')->cvterm_id,
      'struct_id' => tpps_load_cvterm('whole plant')->cvterm_id, // manual term for MASS Spec
    );

    print_r('ISO_FID:' . $iso_fid . "\n");
    tpps_job_logger_write('[INFO] - Processing phenotype_data file data...');
    $job->logMessage('[INFO] - Processing phenotype_data file data...');
    tpps_file_iterator($iso_fid, 'tpps_process_phenotype_data', $options);
    tpps_job_logger_write('[INFO] - Inserting phenotype_data into database using insert_multi...');
    $job->logMessage('[INFO] - Inserting phenotype_data into database using insert_multi...');
    tpps_chado_insert_multi($options['records']);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');

  }
  // throw new Exception('DEBUG');
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
function tpps_submit_genotype(array &$form_state, array $species_codes, $i, TripalJob &$job = NULL) {
  tpps_job_logger_write('[INFO] - Submitting genotype data...');
  $job->logMessage('[INFO] - Submitting genotype data...');

  // Pages data
  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  // Project id is how this study is recorded in chado tables instead of TGDRXXXX
  $project_id = $form_state['ids']['project_id'];
  // This record_group variable is the number of records in a batch
  // This can be set within the TPPS admin panel
  $record_group = variable_get('tpps_record_group', 10000);

  // VCF needed variables
  $vcf_processing_completed = false; // Remember if VCF is already processed or not
  $vcf_import_mode = $form_state['saved_values'][TPPS_PAGE_1]['vcf_import_mode'];
  if (!isset($vcf_import_mode)) {
    $vcf_import_mode = 'hybrid'; // if not set, set it as default hybrid
  }
  
  // If no genotype data, don't continue running this code
  if (empty($genotype)) {
    return;
  }

  // Add tag genotype to this study
  tpps_submission_add_tag($form_state['accession'], 'Genotype');

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

  // $records array is used for normal insertions
  $records = array(
    'feature' => array(),
    'genotype' => array(),
    'genotype_call' => array(),
    'stock_genotype' => array(),
  );

  // $records2 array is used For COPY / HYBRID inserts
  $records2 = array(
    'genotype_call' => array()
  );

  // RISH (7/18/2023)- Still don't understand this code but it is used 
  // during normal inserts
  $multi_insert_options = array(
    'fk_overrides' => $overrides,
    'entities' => array(
      'label' => 'Genotype',
      'table' => 'genotype',
    ),
  );

  // The options array is used for the tpps file iterator since the iterator
  // custom function has to know per line, some of this standard information to 
  // run and process the given data properly. So an assay file process might
  // need different information in the options compared to ssr file process. 
  $options = array(
    // This variable will cause tpps_iterator top do only 1 iteration
    // and speed up a test run of a study submit
    // 'test' => 1, 
    'records' => $records,
    'records2' => $records2,
    'tree_info' => $form_state['tree_info'],
    'species_codes' => $species_codes,
    'genotype_count' => &$genotype_count,
    'genotype_total' => &$genotype_total,
    'organism_index' => $i, // RISH [6/22/2023] to remember which organism this genotype data is connected to based on page 4
    'project_id' => $project_id,
    'seq_var_cvterm' => $seq_var_cvterm,
    'multi_insert' => &$multi_insert_options,
    'job' => &$job,
  );

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
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
        'type_id' => tpps_load_cvterm('file_path')->cvterm_id,
        'value' => $assembly_user,
        'rank' => $form_state['file_rank'],
      ));
      $form_state['file_rank']++;
    }
  }
  elseif ($genotype['ref-genome'] != 'none') {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => tpps_load_cvterm('reference_genome')->cvterm_id,
      'value' => $genotype['ref-genome'],
    ));
  }


  if (!empty($genotype['files']['snps-assay'])) {

    // RISH - Logic removed 7/20/2023
    // Check to see whether there is a VCF, since we want the genotype_calls
    // to be created based on a VCF if one exists
    // $vcf_exists = tpps_vcf_exists($form_state, $i);
    // if ($vcf_exists) {
    //   // process vcf first to insert genotype and genotype_calls
    //   tpps_genotype_vcf_processing($form_state, $species_codes, $i, $job, $vcf_import_mode);
    //   $vcf_processing_completed = true;
    // }

    $snp_fid = $genotype['files']['snps-assay'];
    tpps_add_project_file($form_state, $snp_fid);

    $options['type'] = 'snp';
    $options['headers'] = tpps_file_headers($snp_fid);
    $options['marker'] = 'SNP';
    $options['type_cvterm'] = tpps_load_cvterm('snp')->cvterm_id;
    $options['ref-genome'] = $genotype['ref-genome'];
    
    // This variable is used to determine whether to import
    // genotype and genotype calls from assay or not
    // If the VCF was loaded, we don't need to import the SNPs Assay
    // genotypes and genotype_calls
    $options['vcf_processing_completed'] = $vcf_processing_completed;
    $ref_genome = $genotype['ref-genome'];
    echo "Ref-genome: $ref_genome\n";



    // Lookup analysis id from reference genome and add it to options array
    $options['analysis_id'] = tpps_get_analysis_id_from_ref_genome($ref_genome);

    if (!empty($genotype['files']['snps-association'])) {
      $assoc_fid = $genotype['files']['snps-association'];
      print_r("Association file ID: " . $assoc_fid . "\n");
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

      switch ($genotype['files']['snps-association-type']) {
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
      tpps_job_logger_write('[INFO] - Processing snp_association file data...');
      $job->logMessage('[INFO] - Processing snp_association file data...');

      tpps_file_iterator($assoc_fid, 'tpps_process_snp_association', $options);
      tpps_job_logger_write('[INFO] - Done.');
      $job->logMessage('[INFO] - Done.');

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

    // DROP INDEXES FROM GENOTYPE_CALL TABLE
    tpps_drop_genotype_call_indexes($job);

    tpps_job_logger_write('[INFO] - Processing SNP genotype_spreadsheet file data...');
    $job->logMessage('[INFO] - Processing SNP genotype_spreadsheet file data...');
    echo "trace 1\n";
    echo "Species codes:\n";
    print_r($options['species_codes']);
    echo "Tree Info:\n";
    print_r($options['tree_info']);
    tpps_file_iterator($snp_fid, 'tpps_process_genotype_spreadsheet', $options);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');


    tpps_job_logger_write('[INFO] - Inserting SNP genotype_spreadsheet data into database using insert_multi...');
    $job->logMessage('[INFO] - Inserting SNP genotype_spreadsheet data into database using insert_multi...');
    tpps_chado_insert_multi($options['records'], $multi_insert_options);

    tpps_job_logger_write('[INFO] - Inserting SNP genotype_spreadsheet data into database using insert_hybrid...');
    $job->logMessage('[INFO] - Inserting SNP genotype_spreadsheet data into database using insert_hybrid...');
    tpps_chado_insert_hybrid($options['records2'], $multi_insert_options);

    tpps_job_logger_write('[INFO] - Done');
    $job->logMessage('[INFO] - Done');

    // RECREATE INDEXES FROM GENOTYPE_CALL TABLE
    tpps_create_genotype_call_indexes($job);

    $options['records'] = $records;
    $genotype_total += $genotype_count;
    tpps_job_logger_write('[INFO] - Genotype count:' . $genotype_count);
    $job->logMessage('[INFO] - Genotype count:' . $genotype_count);
    $genotype_count = 0;
  }

  // This if statement caters for the Genotype Assay Design file (which holds extra data like positions etc)
  // This is usually also accompanied with the Genotype SNP Assay (which holds the snps)
  // We want to insert location data into the database if both are found and the marker-type is snp
  // The previous step took care of the SNPs insertion via the Genotype SNP Assay (not to be confused with genotype SNP assay design file)
  if (!empty($genotype['files']['assay-design'])) {
    $design_fid = $genotype['files']['assay-design'];
    tpps_add_project_file($form_state, $design_fid);

    // Setup the options array which the tpps_file_iterator custom function
    // will be able to access necessary details.
    $options['type'] = 'snp';

    print_r("\n");
    $options['marker'] = 'SNP';
    $options['type_cvterm'] = tpps_load_cvterm('snp')->cvterm_id;
    $options['ref-genome'] = $genotype['ref-genome'];
    $ref_genome = $genotype['ref-genome'];
    echo "Ref-genome: $ref_genome\n";
    // Lookup analysis id from reference genome and add it to options array.
    $options['analysis_id'] = tpps_get_analysis_id_from_ref_genome($ref_genome);
    print_r("ANALYSIS ID: " . $options['analysis_id'] . "\n");

    // We must have an analysis_id to tie back to the srcfeature.
    if ($options['analysis_id'] != NULL) {
      // Initialize new records with featureloc array to store records.
      $options['records']['featureloc'] = array();
      $options['records']['featureprop'] = array();

      $options['headers'] = tpps_file_headers($design_fid);
      print_r("HEADERS:\n");
      print_r($options['headers']);
      print_r("\n");

      // Find the marker name header.
      $options['file_columns'] = [];
      foreach ($options['headers'] as $column => $column_name) {
        $column_name = strtolower(trim($column_name));
        print_r("spreadsheet column name:" . $column_name . " column: $column\n");
        switch ($column_name) {
          case 'chr':
            $options['file_columns']['chr'] = $column;
            break;

          case 'forward sequence':
            $options['file_columns']['forward_sequence'] = $column;
            break;

          case 'reverse sequence':
            $options['file_columns']['reverse_sequence'] = $column;
            break;

          case 'snp':
            $options['file_columns']['snp'] = $column;
            break;
        }
        if (strpos($column_name, 'position') !== FALSE) {
          $options['file_columns']['position'] = $column;
        }
        elseif (strpos($column_name, 'marker name') !== FALSE) {
          $options['file_columns']['marker_name'] = $column;
        }
        print($options['file_columns']);
        print_r($options['file_columns']);
      }

      // We want to process this Genotype SNP Assay Design file before
      // we add it as a project file.
      tpps_job_logger_write('[INFO] - Processing genotype_snp_assay_design file data...');
      $job->logMessage('[INFO] - Processing snp_association file data...');
      tpps_file_iterator($design_fid, 'tpps_process_genotype_snp_assay_design', $options);
      tpps_job_logger_write('[INFO] - Done.');
      $job->logMessage('[INFO] - Done.');

      tpps_job_logger_write('[INFO] - Inserting genotype_snp_assay_design_spreadsheet data into database using insert_multi...');
      $job->logMessage('[INFO] - Inserting genotype_snp_assay_design_spreadsheet data into database using insert_multi...');
      tpps_chado_insert_multi($options['records'], []);
      tpps_job_logger_write('[INFO] - Done');
      $job->logMessage('[INFO] - Done');
      // Reset options[records] with empty records arrays.
      $options['records'] = $records;

    }
    else {
      tpps_job_logger_write('[ERROR] - Analysis ID could not be found, skipping assay design file processing.');
      $job->logMessage('[ERROR] - Analysis ID could not be found, skipping assay design file processing.');
    }
  }
  if (!empty($genotype['files']['ssrs'])) {
    $ssr_fid = $genotype['files']['ssrs'];
    tpps_add_project_file($form_state, $ssr_fid);

    // DROP INDEXES FROM GENOTYPE_CALL TABLE
    tpps_drop_genotype_call_indexes($job);

    $options['type'] = 'ssrs';
    $options['headers'] = tpps_ssrs_headers($ssr_fid, $genotype['files']['ploidy']);
    $options['marker'] = $genotype['SSRs/cpSSRs'];
    $options['type_cvterm'] = tpps_load_cvterm('ssr')->cvterm_id;
    $options['empty'] = $genotype['files']['ssrs-empty'];
    tpps_job_logger_write('[INFO] - Processing SSR genotype_spreadsheet file data...');
    $job->logMessage('[INFO] - Processing SSR genotype_spreadsheet file data...');
    echo "trace 2\n";
    tpps_file_iterator($ssr_fid, 'tpps_process_genotype_spreadsheet', $options);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');

    tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
    $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
    tpps_chado_insert_multi($options['records'], $multi_insert_options);

    tpps_job_logger_write('[INFO] - Inserting data into database using insert_hybrid...');
    $job->logMessage('[INFO] - Inserting data into database using insert_hybrid...');
    tpps_chado_insert_hybrid($options['records2'], $multi_insert_options);    

    tpps_job_logger_write('[INFO] - Done');
    $job->logMessage('[INFO] - Done.');

    // RECREATE INDEXES FROM GENOTYPE_CALL TABLE
    tpps_create_genotype_call_indexes($job);

    $options['records'] = $records;
    $genotype_count = 0;

    // SSRS_EXTRA seems to be for cpSSRs file
    if (!empty($genotype['files']['ssrs_extra'])) {
      $extra_fid = $genotype['files']['ssrs_extra'];
      tpps_add_project_file($form_state, $extra_fid);

      // DROP INDEXES FROM GENOTYPE_CALL TABLE
      tpps_drop_genotype_call_indexes($job);

      $options['headers'] = tpps_ssrs_headers($extra_fid, $genotype['files']['extra-ploidy']);
      tpps_job_logger_write('[INFO] - Processing EXTRA genotype_spreadsheet file data...');
      $job->logMessage('[INFO] - Processing EXTRA genotype_spreadsheet file data...');
      echo "trace 3\n";
      tpps_file_iterator($extra_fid, 'tpps_process_genotype_spreadsheet', $options);
      tpps_job_logger_write('[INFO] - Done.');
      $job->logMessage('[INFO] - Done.');

      tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
      $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
      tpps_chado_insert_multi($options['records'], $multi_insert_options);

      tpps_job_logger_write('[INFO] - Inserting data into database using insert_hybrid...');
      $job->logMessage('[INFO] - Inserting data into database using insert_hybrid...');
      tpps_chado_insert_hybrid($options['records2'], $multi_insert_options);  

      tpps_job_logger_write('[INFO] - Done.');
      $job->logMessage('[INFO] - Done.');

      // CREATE INDEXES FROM GENOTYPE_CALL TABLE
      tpps_create_genotype_call_indexes($job);

      $options['records'] = $records;
      $genotype_count = 0;
    }
  }

  if (!empty($genotype['files']['other'])) {
    $other_fid = $genotype['files']['other'];
    tpps_add_project_file($form_state, $other_fid);

    $options['headers'] = tpps_file_headers($other_fid);
    if (!empty($genotype['files']['other-groups'])) {
      $groups = $genotype['files']['other-groups'];
      $options['headers'] = tpps_other_marker_headers($other_fid, $groups['Genotype Data'][0]);
      $options['tree_id'] = $groups['Tree Id'][1];
    }

    // DROP INDEXES FROM GENOTYPE_CALL TABLE
    tpps_drop_genotype_call_indexes($job);

    $options['type'] = 'other';
    $options['marker'] = $genotype['other-marker'];
    $options['type_cvterm'] = tpps_load_cvterm('genetic_marker')->cvterm_id;
    tpps_job_logger_write('[INFO] - Processing OTHER MARKER genotype_spreadsheet file data...');
    $job->logMessage('[INFO] - Processing OTHER MARKER genotype_spreadsheet file data...');
    echo "trace 5\n";
    tpps_file_iterator($other_fid, 'tpps_process_genotype_spreadsheet', $options);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');

    tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
    $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
    tpps_chado_insert_multi($options['records'], $multi_insert_options);

    tpps_job_logger_write('[INFO] - Inserting data into database using insert_hybrid...');
    $job->logMessage('[INFO] - Inserting data into database using insert_hybrid...');
    tpps_chado_insert_hybrid($options['records2'], $multi_insert_options);

    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');

    // CREATE INDEXES FROM GENOTYPE_CALL TABLE
    tpps_create_genotype_call_indexes($job);

    $options['records'] = $records;
    $genotype_count = 0;
  }

  // VCF PROCESSING OCCURS FROM HERE
  tpps_job_logger_write('[INFO] - VCF IMPORT MODE is ' . $vcf_import_mode);
  $job->logMessage('[INFO] - VCF IMPORT MODE is ' . $vcf_import_mode);
  if ($vcf_processing_completed == false) {
    // THE VCF PROCESSING CODE HAS BEEN MOVED TO A NEW FUNCTION
    tpps_genotype_vcf_processing($form_state, $species_codes, $i, $job, $vcf_import_mode);
  }
  else {
    tpps_job_logger_write('[INFO] - VCF was already processed!');
    $job->logMessage('[INFO] - VCF was already processed!');    
  }
}

/**
 * This function will process a vcf file's genotypic information and store it within the db.
 * Most importantly is the $import_mode option which can be set to 'multi' or 'hybrid'
 * Hybrid requires the the db user be granted SUPERUSER access to the Postgresql db in order
 * to utilize the COPY keyword for inserting data which is dramatically faster.
 */
function tpps_genotype_vcf_processing(array &$form_state, array $species_codes, $i, TripalJob &$job = NULL, $insert_mode = 'hybrid') {
  $organism_index = $i;
  // Some initial variables previously inherited from the parent function code. So we're reusing it to avoid
  // missing any important variables if we rewrote it.
  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;

  if ($insert_mode == '') {
    throw new Exception('VCF processing insert mode was empty - it should have a value of either hybrid or inserts.');
  }


  // Project ID is more for the database (it is different from the TPPS Accession)
  // but is unique as well.
  $project_id = $form_state['ids']['project_id'];

  // Record group is used to determine batch side per inserts
  $record_group = variable_get('tpps_record_group', 10000);

  // Some initialization variables used later down including the $records variable
  // which stores table => rows => fields
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

  // check to make sure admin has not set disable_vcf_importing.
  $disable_vcf_import = 0;
  if (isset($firstpage['disable_vcf_import'])) {
    $disable_vcf_import = $firstpage['disable_vcf_import'];
  }
  tpps_job_logger_write('[INFO] Disable VCF Import is set to ' . $disable_vcf_import . ' (0 means allow vcf import, 1 ignore vcf import)');

  if ($genotype['files']['file-type'] == 'VCF') {
    if ($disable_vcf_import == 0) {
      // DROP INDEXES FROM GENOTYPE_CALL TABLE
      tpps_drop_genotype_call_indexes($job);


      // @todo we probably want to use tpps_file_iterator to parse vcf files.
      $vcf_fid = $genotype['files']['vcf'];
      tpps_add_project_file($form_state, $vcf_fid);

      $records['genotypeprop'] = array();

      $snp_cvterm = tpps_load_cvterm('snp')->cvterm_id;
      $format_cvterm = tpps_load_cvterm('format')->cvterm_id;
      $qual_cvterm = tpps_load_cvterm('quality_value')->cvterm_id;
      $filter_cvterm = tpps_load_cvterm('filter')->cvterm_id;
      $freq_cvterm = tpps_load_cvterm('allelic_frequency')->cvterm_id;
      $depth_cvterm = tpps_load_cvterm('read_depth')->cvterm_id;
      $n_sample_cvterm = tpps_load_cvterm('number_samples')->cvterm_id;

      // This means it was uploaded
      if ($vcf_fid > 0) {
        $vcf_file = file_load($vcf_fid);
        $location = tpps_get_location($vcf_file->uri);
      }
      else {
        $location = $genotype['files']['local_vcf'];
      }
      if ($location == null || $location == "") {
        throw new Exception('Could not find location of VCF even though the VCF option was specified.
        File ID was 0 so its not an uploaded file. local_vcf variable returned empty so cannot use that');
      }
      echo "VCF location: $location\n";

      $vcf_content = gzopen($location, 'r');
      $stocks = array();
      $format = "";
      // This was done by Peter
      $current_id = $form_state['ids']['organism_ids'][$i];
      $species_code = $species_codes[$current_id];


      // Override the above code done by Rish to use organism_id from page 4
      // RISH NOTES: This addition uses the organism_id based on the organism order
      // of the fourth page (we likely have to pass the i from previous function here)
      // THIS TECHNICALLY OVERRIDES PETER'S LOGIC ABOVE. TO BE DETERMINED IF RISH'S WAY IS CORRECT
      // OR NOT [7/1/2023]
      // THIS WAS AN ISSUE BROUGHT UP BY EMILY REGARDING SNPS NOT BEING ASSOCIATED WITH POP TRICH (665 STUDY)
      $species_code = null;
      $organism_id = null;
      $count_tmp = 0;
      foreach ($species_codes as $organism_id_tmp => $species_code_tmp) {
        $count_tmp = $count_tmp + 1; // increment
        // Check if count_tmp matches $organism_index
        if ($count_tmp == $organism_index) {
          $species_code = $species_code_tmp;
          $organism_id = $organism_id_tmp;
          $current_id = $organism_id;
          break;
        }
      }
      echo "Organism id: $current_id\n";
      echo "Species code: $species_codes[$current_id]\n";

      // The following code is used to get the analysis_id from the genome assemble if it's selected
      // WE NEED THIS TO DO FEATURELOC INSERTS (basically to get to the point of srcfeature_id later on)
      // * GOALS: First we need to find the analysis id
      // We need to get the reference genome from the TPPS form ex:
      $ref_genome = $genotype['ref-genome'];
      $analysis_id = NULL;
      if (isset($ref_genome)) {
        // Get the species and version from the reference genome selected
        // if match occurs thats in index [0].
        // The group match index [1] is species, group match index [2] is version
        preg_match('/(.+) +v(\d*\.*\d*)/', $ref_genome, $matches);
        $ref_genome_species = NULL;
        $ref_genome_version = NULL;
        if (count($matches) > 0) {
          $ref_genome_species = $matches[1];
          $ref_genome_version = $matches[2];
        }

        if (isset($ref_genome_species) && isset($ref_genome_version)) {
          // Look up the analysis
          $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
            WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
            [
              ':name' => $ref_genome_species . '%',
              ':programversion' => $ref_genome_version
            ]
          );
          foreach ($analysis_results as $row) {
            $analysis_id = $row->analysis_id;
          }
        }

        if($analysis_id == NULL) {
          // Look up the analysis
          $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
            WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
            [
              ':name' => $ref_genome_species . '%',
              ':programversion' => 'v' . $ref_genome_version
            ]
          );
          foreach ($analysis_results as $row) {
            print_r("analysis_row\n");
            print_r($row);
            print_r("\n");
            $analysis_id = $row->analysis_id;
          }
        }

        // If an analysis_id still was not found, it's possibly from the db data source
        // instead of the genome directory. The genome directory code is in page_4_helper.php
        // New code to cater for new analysis checks via db - query given by Emily Grau (6/6/2023)
        if ($analysis_id == NULL) {
          $genome_query_results = chado_query("select * from chado.tpps_ref_genomes WHERE name LIKE :ref_genome;", [
            ':ref_genome' => $ref_genome
          ]);
          foreach ($genome_query_results as $genome_query_row) {
            $analysis_id = $genome_query_row->analysis_id;
          }
        }

        // Once an analysis_id was found, try to get srcfeature_id

      }
      else {
        $error_line = "A reference genome could not be found in the TPPS page 4 form.\n";
        $error_line .= "Without this, we cannot find the analysis_id and thus the srcfeature_id.\n";
        $error_line .=  "Featureloc data will not be recorded\n";
        tpps_job_logger_write('[REF GENOME NOT FOUND] - ' . $error_line);
        $job->logMessage('[REF GENOME NOT FOUND] - ' . $error_line);
      }

      print_r("Analysis ID Found: $analysis_id");
      // throw new Exception("DEBUG");

      echo "[INFO] Processing Genotype VCF file\n";
      $file_progress_line_count = 0;
      $record_count = 0;
      while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
        $file_progress_line_count++;
        if($file_progress_line_count % 10000 == 0 && $file_progress_line_count != 0) {
          echo '[INFO] [VCF PROCESSING STATUS] ' . $file_progress_line_count . " lines done\n";
        }
        if ($vcf_line[0] != '#' && stripos($vcf_line,'.vcf') === FALSE && trim($vcf_line) != "" && str_replace("\0", "", $vcf_line) != "") {
          $line_process_start_time = microtime(true);
          $record_count = $record_count + 1;
          // DEBUG - TEST ONLY 100 records
          // if ($record_count > 100) {
          //   break;
          // } 
          print_r('Record count:' . $record_count . "\n");
          $genotype_count += count($stocks);
          $vcf_line = explode("\t", $vcf_line);
          $scaffold_id = &$vcf_line[0];
          $position = &$vcf_line[1];
          $variant_name = &$vcf_line[2];
          $ref = &$vcf_line[3];
          $alt = &$vcf_line[4];
          $qual = &$vcf_line[5];
          $filter = &$vcf_line[6];
          $info = &$vcf_line[7];
          $marker_type = 'SNP';

          $cache_srcfeatures = []; // stores key = name of source feature, value is the feature_id

          if (empty($variant_name) or $variant_name == '.') {
            // $variant_name = "{$scaffold_id}{$position}$ref:$alt";
            $variant_name = $scaffold_id . '_' . $position . 'SNP';
          }
          // $marker_name = $variant_name . $marker; // Original by Peter
          $marker_name = $scaffold_id . '_' . $position . '-' . $species_code; // Emily updated suggestion on Tuesday August 9th 2022

          // $description = "$ref:$alt"; // Replaced with genotype_combination within $detected_genotypes array (5/31/2023)

          // $genotype_name = "$marker-$species_code-$scaffold_id-$position"; // Original by Peter

          // Instead, we have multiple genotypes we need to generate, so lets do a key val array
          $detected_genotypes = array();
          $first_genotypes = array(); // used to save the first genotype in each row of the VCF (used for genotype_call table)
          $count_columns = count($vcf_line);
          for ($j = 9; $j < $count_columns; $j++) {

            $genotype_combination = tpps_submit_vcf_render_genotype_combination($vcf_line[$j], $ref, $alt); // eg AG (removed the : part of code on 5/31/2023)

            // Check if marker type is indel
            // split ref by comma (based on Emily's demo), go through each split value
            $ref_comma_parts = explode(',', $ref); // eg G,GTAC
            foreach ($ref_comma_parts as $ref_comma_part) {
              $ref_comma_part = trim($ref_comma_part);
              // Check length of comma_part
              $len = strlen($ref_comma_part);
              // If len is more than 1, use this value to calculate the fmax position
              if($len > 1) {
                $marker_type = 'INDEL';
                break;
              }
            }

            $detected_genotypes[$marker_type . '-' . $marker_name . '-' . $genotype_combination] = [
              'marker_name' => $marker_name,
              'marker_type' => $marker_type,
              'genotype_combination' => $genotype_combination,
            ]; // [scaffold_pos_A:G] = scaffold_pos
            // $detected_genotypes[$marker_name] = TRUE;

            // Record the first genotype name to use for genotype_call table
            if($j == 9) {
              // print_r('[First Genotype]:' . $marker_name . $genotype_combination . "\n");
              $first_genotypes[$marker_type . '-' . $marker_name . '-' . $genotype_combination] = TRUE;
            }

          }


          // PETER'S CODE
          // $records['feature'][$marker_name] = array(
          //   'organism_id' => $current_id,
          //   'uniquename' => $marker_name,
          //   'type_id' => $seq_var_cvterm,
          // );

          // Rish code to test a single insert and get the id
          try {
            $results = chado_insert_record('feature', [
              'name' => $marker_name,
              'organism_id' => $current_id,
              'uniquename' => $marker_name,
              'type_id' => $seq_var_cvterm,
            ]);
          }
          catch (Exception $ex) {

          }
          // get the feature_id
          $results = chado_query('SELECT feature_id FROM chado.feature WHERE uniquename = :uniquename', [
            ':uniquename' => $marker_name
          ]);
          $row_object = $results->fetchObject();
          $marker_id = $row_object->feature_id;

          // PETER'S CODE
          // $records['feature'][$variant_name] = array(
          //   'organism_id' => $current_id,
          //   'uniquename' => $variant_name,
          //   'type_id' => $seq_var_cvterm,
          // );

          // Rish code to test a single insert and get the id
          try {
            $results = chado_insert_record('feature', [
              'name' => $variant_name,
              'organism_id' => $current_id,
              'uniquename' => $variant_name,
              'type_id' => $seq_var_cvterm,
            ]);
          }
          catch (Exception $ex) {

          }
          // get the feature_id
          $results = chado_query('SELECT feature_id FROM chado.feature WHERE uniquename = :uniquename', [
            ':uniquename' => $variant_name
          ]);
          $row_object = $results->fetchObject();
          $variant_id = $row_object->feature_id;

          // Lookup whether marker is already inserted into the features table
          $result = chado_query("SELECT * FROM chado.feature WHERE uniquename = :marker_name AND organism_id = :organism_id", [
            ':marker_name' => $variant_name, // column 3 of VCF
            ':organism_id' => $current_id
          ]);

          $feature_exists = false;
          foreach ($result as $row) {
            $feature_exists = true;
          }

          if ($feature_exists != true) {
            // SOME CODE FOR THIS IS OUTSIDE OF THE PER LINE PROCESSING ABOVE (OUTSIDE FOR LOOP)
            // 3/27/2023: Chromosome number and position (store this in featureloc table)
            // feature_id from marker or variant created
            // srcfeature_id for the genome / assembly used for reference (some sort of query - complicated)
            // store where marker starts on chromosome etc.
            $srcfeature_id = NULL;
            if (isset($analysis_id)) {
              // Get the srcfeature_id
              echo 'Scaffold ID (srcfeature_id search): ' . $scaffold_id . "\n";

              // the scaffold_id is not an integer value, proceed as normal lookup
              $srcfeature_results = chado_query('select feature.feature_id from chado.feature
                join chado.analysisfeature on feature.feature_id = analysisfeature.feature_id
                where feature.name = :scaffold_id and analysisfeature.analysis_id = :analysis_id',
                [
                  ':scaffold_id' => $scaffold_id,
                  ':analysis_id' => $analysis_id
                ]
              );

              foreach ($srcfeature_results as $row) {
                $srcfeature_id = $row->feature_id;
              }
            }

            // If reference genome found (analysis_id) but no srcfeature found
            if (isset($analysis_id) && !isset($srcfeature_id)) {
              throw new Exception("Genotype VCF processing found reference genome but no
                srcfeature could be found. This action was recommended by Database Administrator.");
            }

            // if srcfeature_id was found, then we have enough info to add featureloc data
            if (isset($srcfeature_id)) {
              $fmax = $position;

              // Check to see whether feature is an indel ($ref is non-singular value)
              // split ref by comma (based on Emily's demo), go through each split value
              $ref_comma_parts = explode(',', $ref); // eg G,GTAC
              foreach ($ref_comma_parts as $ref_comma_part) {
                $ref_comma_part = trim($ref_comma_part);
                // Check length of comma_part
                $len = strlen($ref_comma_part);
                // If len is more than 1, use this value to calculate the fmax position
                if($len > 1) {
                  $fmax = intval($position) + ($len - 1);
                  break;
                }
              }


              $featureloc_values = [
                'feature_id' => $marker_id,
                'srcfeature_id' => $srcfeature_id,
                'fmin' => $position,
                'fmax' => $fmax // ALPHA code above now caters for INDELS
              ];

              // Since we haven't catered for deletion of these featureloc records
              // there may already exist, we have to make sure the record doesn't already exist
              $featureloc_results = chado_query('SELECT count(*) as c1 FROM chado.featureloc
                WHERE feature_id = :feature_id AND srcfeature_id = :srcfeature_id;', [
                  ':feature_id' => $marker_id,
                  ':srcfeature_id' => $srcfeature_id
                ]
              );
              $featureloc_count = 0;
              foreach ($featureloc_results as $row) {
                $featureloc_count = $row->c1;
              }
              // This means no featureloc exists, so insert it
              if ($featureloc_count == 0) {
                // This will add it to the multiinsert record system for insertion
                $records['featureloc'][$marker_name] = $featureloc_values;
              }
            }
          }

          // Rish 12/08/2022: So we have multiple genotypes created
          // So I adjusted some of this code into a for statement
          // since the genotype_desc seems important and so I modified it to be unique
          // and based on the genotype_name
          $genotype_names = array_keys($detected_genotypes);

          // print_r($detected_genotypes);
          echo "\n";
          echo "line#$file_progress_line_count ";
          print_r('genotypes per line: ' . count($genotype_names) . " ");

          $genotype_name_progress_count = 0;
          foreach ($detected_genotypes as $genotype_name => $genotype_info_array) { // eg SNP-scaffold_pos_-POTR-AG
            $genotype_name_without_combination = $genotype_info_array['marker_name'];
            $marker_type = $genotype_info_array['marker_type'];
            $genotype_combination = $genotype_info_array['genotype_combination'];
            $genotype_name_progress_count++;
            $genotype_desc = $genotype_name; // Includes marker type. Ideally uniquename should be exactly the same as name with the gentoype read added to the end. (Emily 5/30/2023)
            // $genotype_desc = "$marker-$species_code-$genotype_name-$position-$description"; // Altered on advice from Emily 5/30/2023
            // print_r('[DEBUG: Genotype] genotype_name: ' . $genotype_name . ' ' . 'genotype_desc: ' . $genotype_desc . "\n");

            // PETER'S CODE
            // $records['genotype'][$genotype_desc] = array(
            //   'name' => $genotype_name,
            //   'uniquename' => $genotype_desc,
            //   'description' => $description,
            //   'type_id' => $snp_cvterm,
            // );

            // Rish code to test a single insert and get the id
            // First we need to get the genotype_name without the combination
            // TOO SLOW
            // $genotype_name_without_combination = '';
            // $genotype_name_parts = explode('_', $genotype_name);
            // $genotype_name_parts_count = count($genotype_name_parts);
            // for($gnpi = 0; $gnpi < ($genotype_name_parts_count - 1); $gnpi++) {
            //   $genotype_name_without_combination .= $genotype_name_parts[$gnpi] . '_';
            // }
            // $genotype_name_without_combination = rtrim($genotype_name_without_combination, '_');
            // echo "Genotype name without combination: $genotype_name_without_combination\n";


            try {
              $results = chado_insert_record('genotype', [
                'name' => $marker_type . '-' . $genotype_name_without_combination,
                'uniquename' => $genotype_desc,
                //'description' => $description, // Replaced since this produced weird values: G:A,N,NT,NC
                'description' => $genotype_combination, // Genotype combination from the detected_genotypes array result (5/31/2023)
                'type_id' => $snp_cvterm,
              ]);
              // print_r($results);
              // print_r("\n");
            }
            catch (Exception $ex) {

            }
            // throw new Exception('DEBUG');
            // get the feature_id
            $results = chado_query('SELECT genotype_id FROM chado.genotype WHERE uniquename = :uniquename', [
              ':uniquename' => $genotype_desc
            ]);
            $row_object = $results->fetchObject();
            $genotype_id = $row_object->genotype_id;
            // $debug_info = "Uniquename: $genotype_desc Type_id:$format_cvterm Value:$format Genotype_id:$genotype_id Variant_id:$variant_id Marker_id:$marker_id\n";
            // $debug_info = "Variant_name: $variant_name, Variant_id: $variant_id\n";
            // echo("DEBUG INFO: $debug_info");

             // 3/27/2023 Meeting - FORMAT: REVIEW THIS IN TERMS OF IF WE NEED IT
            if ($format != "") {
              $records['genotypeprop']["$genotype_desc-format"] = array(
                'type_id' => $format_cvterm,
                'value' => $format,
                // PETER
                // '#fk' => array(
                //   'genotype' => $genotype_desc,
                // ),
                // RISH
                'genotype_id' => $genotype_id,
              );
            }

            $vcf_cols_count = count($vcf_line);
            echo "gen_name_index:$genotype_name_progress_count colcount:$vcf_cols_count ";
            for ($j = 9; $j < $vcf_cols_count; $j++) {
              // Rish: This was added on 09/12/2022
              // This gets the name of the current genotype for the tree_id column
              // being checked.


              $column_genotype_name = $marker_type . '-' . $marker_name . '-' . tpps_submit_vcf_render_genotype_combination($vcf_line[$j], $ref, $alt);
              // echo 'Column Genotype Name: ' . $column_genotype_name . " Genotype Name: $genotype_name\n";
              if($column_genotype_name == $genotype_name) {
                // Found a match between the tree_id genotype and the genotype_name from records
                // echo "Found match (and using variant_name $variant_name ($variant_id) to add to genotype call\n";

                // print_r('[genotype_call insert]: ' . "{$stocks[$j - 9]}-$genotype_name" . "\n");
                $records['genotype_call']["{$stocks[$j - 9]}-$genotype_name"] = array(
                  'project_id' => $project_id,
                  'stock_id' => $stocks[$j - 9],
                  // PETER
                  // '#fk' => array(
                  //   'genotype' => $genotype_desc,
                  //   'variant' => $variant_name,
                  //   'marker' => $marker_name,
                  // ),
                  // RISH
                  'genotype_id' => $genotype_id,
                  'variant_id' => $variant_id,
                  'marker_id' => $marker_id,
                );

                // THIS ABOUT REMOVING THIS - but it is in use for genotype materialized views
                // which is used for tpps/details page
                // $records['stock_genotype']["{$stocks[$j - 9]}-$genotype_name"] = array(
                //   'stock_id' => $stocks[$j - 9],
                //   // PETER
                //   // '#fk' => array(
                //   //   'genotype' => $genotype_desc,
                //   // ),
                //   // RISH
                //   'genotype_id' => $genotype_id,
                // );
              }
            }
            // throw new Exception('DEBUG');

            // @TODO 3/28/2023 - Gabe thought we didn't need additional data from the VCF file
            // Basically chromosome and position
            // Featureloc table: the following are where to get the values for these fields
            //                   in order to create the featureloc record
            // Field: feature_id -> $records['feature'][$marker_name]
            // Field: srcfeature_id query needs analysis_id from TPPS FORM, then query feature table
            // Field: fmin
            // Field: fmax


            // 3/27/2023 - Jill question: Do we need to store in the database
            // Quality score.
            $records['genotypeprop']["$genotype_desc-qual"] = array(
              'type_id' => $qual_cvterm,
              'value' => $qual,

              // PETER
              // '#fk' => array(
              //   'genotype' => $genotype_desc,
              // ),

              // RISH
              'genotype_id' => $genotype_id,
            );

            // filter: pass/fail.
            $records['genotypeprop']["$genotype_desc-filter"] = array(
              'type_id' => $filter_cvterm,
              'value' => ($filter == '.') ? "P" : "NP",

              // PETER
              // '#fk' => array(
              //   'genotype' => $genotype_desc,
              // ),

              // RISH
              'genotype_id' => $genotype_id,
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

                // PETER
                // '#fk' => array(
                //   'genotype' => $genotype_desc,
                // ),

                // RISH
                'genotype_id' => $genotype_id,
              );
            }

            // 3/27/2023 - Jill question: Do we need to store in the database
            // Depth coverage, assuming that the info code for depth coverage is
            // 'DP'.
            if (isset($info_vals['DP']) and $info_vals['DP'] != '') {
              $records['genotypeprop']["$genotype_desc-depth"] = array(
                'type_id' => $depth_cvterm,
                'value' => $info_vals['DP'],

                // PETER
                // '#fk' => array(
                //   'genotype' => $genotype_desc,
                // ),

                // RISH
                'genotype_id' => $genotype_id,
              );
            }

            // 3/27/2023 - Jill question: Do we need to store in the database
            // Number of samples, assuming that the info code for number of
            // samples is 'NS'.
            if (isset($info_vals['NS']) and $info_vals['NS'] != '') {
              $records['genotypeprop']["$genotype_desc-n_sample"] = array(
                'type_id' => $n_sample_cvterm,
                'value' => $info_vals['NS'],

                // PETER
                // '#fk' => array(
                //   'genotype' => $genotype_desc,
                // ),

                // RISH
                'genotype_id' => $genotype_id,
              );
            }
          }
          $line_process_end_time = microtime(true);
          $line_process_elapsed_time = $line_process_end_time - $line_process_start_time;
          echo " PHP Proctime: $line_process_elapsed_time seconds\n";
          if(!isset($line_process_cumulative_time)) {
            $line_process_cumulative_time = 0;
          }
          $line_process_cumulative_time += $line_process_elapsed_time;
          echo "Cumulative PHP proctime: " . $line_process_cumulative_time . " seconds\n";
          echo "\nGenotype call records to insert (LINE:$file_progress_line_count): " . count($records['genotype_call']);
          echo "\nrecord group threshold: $record_group ";
          // throw new Exception('DEBUG');
          // Tripal Job has issues when all submissions are made at the same
          // time, so break them up into groups of 10,000 genotypes along with
          // their relevant genotypeprops.
          if ($genotype_count > $record_group) {
            tpps_job_logger_write('[INFO] - Last bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            $job->logMessage('[INFO] - Last bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            tpps_job_logger_write('[INFO] - Last bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            $job->logMessage('[INFO] - Last bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            tpps_job_logger_write('[INFO] - Last insert cumulative time: ' . $insert_cumulative_time . ' seconds');
            $job->logMessage('[INFO] - Last insert cumulative time: ' . $insert_cumulative_time . ' seconds');
            $genotype_count = 0;
            $insert_start_time = microtime(true);
            if ($insert_mode == 'multi') {
              tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
              $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
              tpps_chado_insert_multi($records, $multi_insert_options);
            }
            else if ($insert_mode == 'hybrid') {
              // THIS WILL DO SOME MULTI INSERTS BUT ALSO USE COPY FOR GENOTYPE_CALL TABLE
              tpps_job_logger_write('[INFO] - Inserting data into database using insert_hybrid...');
              $job->logMessage('[INFO] - Inserting data into database using insert_hybrid...');
              tpps_chado_insert_hybrid($records, $multi_insert_options);
            }



            tpps_job_logger_write('[INFO] - Done.');
            $job->logMessage('[INFO] - Done.');
            $insert_end_time = microtime(true);
            $insert_elapsed_time = $insert_end_time - $insert_start_time;
            tpps_job_logger_write('[INFO] - Bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            $job->logMessage('[INFO] - Bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            tpps_job_logger_write('[INFO] - Bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            $job->logMessage('[INFO] - Bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            if(!isset($insert_cumulative_time)) {
              $insert_cumulative_time = 0;
            }
            $insert_cumulative_time += $insert_elapsed_time;
            tpps_job_logger_write('[INFO] - Insert cumulative time: ' . $insert_cumulative_time . ' seconds');
            $job->logMessage('[INFO] - Insert cumulative time: ' . $insert_cumulative_time . ' seconds');
            // throw new Exception('DEBUG');
            $records = array(
              'feature' => array(),
              'genotype' => array(),
              'genotype_call' => array(),
              'genotypeprop' => array(),
              'stock_genotype' => array(),
            );
            $genotype_count = 0;

            // throw new Exception('DEBUG GENOTYPE_CALL TIME');
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

      if ($insert_mode == 'multi') {
        tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
        $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
        tpps_chado_insert_multi($records, $multi_insert_options);
      }
      else if ($insert_mode == 'hybrid') {
        tpps_job_logger_write('[INFO] - Inserting data into database using insert_hybrid...');
        $job->logMessage('[INFO] - Inserting data into database using insert_hybrid...');
        tpps_chado_insert_hybrid($records, $multi_insert_options);
      }

      // THIS RECREATES THE INDEXES
      tpps_create_genotype_call_indexes($job);

      tpps_job_logger_write('[INFO] - Done.');
      $job->logMessage('[INFO] - Done.');
      unset($records);
      $genotype_count = 0;
      // dpm('done: ' . date('r'));.
    }
  }

}



/**
 * @param mixed $options array of options
 *        keys: study_accession, form_state, job
 * @return void
 */

function tpps_generate_genotype_sample_file_from_vcf($options = NULL) {
  // If study accession value exists, use this to look up the form_state
  $form_state = NULL;
  if (isset($options['study_accession'])) {
    $form_state = tpps_load_submission($options['study_accession']);
  }
  else if (isset($options['form_state'])) {
    $form_state = $options['form_state'];
  }

  // If $form_state is not NULL
  if (isset($form_state)) {
    // Get page 1 form_state data
    $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
    // Get page 4 form_state data
    $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
    // Organism count
    $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
    // Project ID
    $project_id = $form_state['ids']['project_id'];

    // Go through each organism
    for ($i = 1; $i <= $organism_number; $i++) {
      $organism_name = $firstpage['organism'][$i]['name'];
      echo "Organism name: $organism_name\n";
      $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
      if (empty($genotype['files']['file-type']['VCF'])) {
        echo "Could not find a VCF file for organism-$i\n";
      }
      else {

        // Initialize sample.list text
        $sample_list_data = "VCF_header_sample\tSample_name\tSample_Accession\tGermplasm_name\tGermplasm_Accession\tGermplasm_type\tOrganism\n"; // header

        // Get the VCF fid
        $vcf_fid = $genotype['files']['vcf'];
        if (isset($vcf_fid) && $vcf_fid > 0) {
          echo "Found uploaded VCF with FID: " . $vcf_fid . "\n";
          $vcf_file = file_load($vcf_fid);
          $location = tpps_get_location($vcf_file->uri);
        }
        else {
          echo "Could not detect an uploaded VCF, checking for a local VCF file\n";
          $location = $genotype['files']['local_vcf'];
        }

        if (!isset($location)) {
          echo "[FAILED] Could not find a VCF location. Either upload a VCF file or set a local vcf location via the TPPS form on page 4.\n";
          return;
        }

        echo "VCF location: $location\n";
        $vcf_content = gzopen($location, 'r');

        echo "[INFO] Scanning Genotype VCF file to generate sample list\n";
        $file_progress_line_count = 0;
        while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
          $file_progress_line_count++;
          if($file_progress_line_count % 10000 == 0 && $file_progress_line_count != 0) {
            echo '[INFO] [VCF PROCESSING STATUS] ' . $file_progress_line_count . " lines done\n";
          }
          // We want to get a non header line
          if (stripos($vcf_line,'#CHROM') !== FALSE) {
            // This will take the line and get each tab value
            $vcf_line = explode("\t", $vcf_line);
            // print_r($vcf_line);
            $cols_count = count($vcf_line);
            echo "Found " . ($cols_count - 9) . " sample IDs. Generating sample list file.\n";
            for($j=9; $j<$cols_count; $j++) {
              // print_r($vcf_line[$j]);
              echo ".";
              $vcf_line[$j] = trim($vcf_line[$j]); // in case it's the last column which can contain a new line character which messes up TSV sample file generated
              $sample_list_data .= $vcf_line[$j] . "\t" . $vcf_line[$j] . "\t" . $vcf_line[$j] . "\t" . $organism_name . "-germplasm-name\t" . $vcf_line[$j] . "\taccession\t" . $organism_name . "\n";
            }
            echo "\n";
            // Break out of the while loop since we only want one line
            // to get the sample names
            break;
          }
        } // end while
        $dest_folder = 'public://tpps_vcf_sample_list_files/';
        file_prepare_directory($dest_folder, FILE_CREATE_DIRECTORY);
        $file_name = $form_state['accession'] . '-sample-list-' . $i . '.txt';
        $file = file_save_data($sample_list_data, $dest_folder . $file_name);
        echo "File managed as FID: " . $file->fid . "\n";
        echo "File managed location: " . $file->uri . "\n";
        echo "Real managed real path: " . drupal_realpath($file->uri) . "\n";
        // We could store this in the submit_state - TODO if we need this
        // $form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['genotype']['vcf_sample_list'] = $file->fid;
        // tpps_update_submission($form_state);
        // print_r($sample_list_data);
      } // end else
    } // end for
  }
  else {
    echo "Could not find a form_state\n";
  }
}

/**
 * TPPS Generate Population Structure
 * FastStructure requires pip install pip==9.0.1 to install dependencies
 */

 // drush php-eval 'include("/var/www/Drupal/sites/all/modules/TGDR/forms/submit/submit_all.php"); tpps_generate_popstruct("TGDR675", "/var/www/Drupal/sites/default/files/popstruct_temp/Panel4SNPv3.vcf");'
function tpps_generate_popstruct($study_accession, $vcf_location) {
  // Perform basic checks
  if ($study_accession == "") {
    tpps_job_logger_write("[FATAL ERROR] You must enter a non-empty study accession. Aborting.\n");
    return;
  }

  if ($vcf_location == "") {
    tpps_job_logger_write("[FATAL ERROR] You must enter a non-empty vcf_location. Aborting.\n");
    return;
  }

  // Get the correct path of the public directory
  $path = 'public://';
  $public_path = drupal_realpath($path);
  tpps_job_logger_write('[PUBLIC PATH] ' . $public_path . "\n");
  echo('[PUBLIC PATH] ' . $public_path . "\n");

  // Get the module path
  $module_path = DRUPAL_ROOT . '/' . drupal_get_path('module', 'tpps');
  tpps_job_logger_write('[MODULE PATH] ' . $module_path . "\n");
  echo('[MODULE PATH] ' . $module_path . "\n");

  // Tools path
  $tools_path = $module_path . "/tools";
  tpps_job_logger_write('[TOOLS PATH] ' .  $tools_path . "\n");
  echo('[TOOLS PATH] ' .  $tools_path . "\n");

  // Make temp directory just in case for vcf files etc
  $popstruct_temp_dir = $public_path . '/popstruct_temp/' . $study_accession;
  mkdir($popstruct_temp_dir, 0755, true);

  // In case there are already files in here, delete them
  $files = glob($popstruct_temp_dir . '/*'); // get all file names
  foreach($files as $file){ // iterate files
    if(is_file($file)) {
      tpps_job_logger_write("[CLEAN UP BEFORE BEGIN] Removing $file from the popstruct directory");
      echo("[CLEAN UP BEFORE BEGIN] Removing $file from the popstruct directory\n");
      tpps_job_logger_write("[FILE CLEAN/DELETE] $file");
      echo("[FILE CLEAN/DELETE] $file\n");
      // echo "TODO: Perform the actual delete\n";
      unlink($file); // delete file
    }
  }

  $flag_using_temp_file = false;

  // This variable is used to process the vcf since we may have to gunzip
  // the file. So we need to keep the original location variable (by not overwriting it).
  $vcf_location_temp = $vcf_location;
  if (stripos($vcf_location, '.gz') !== FALSE) {
    // we need to gunzip the file
    // Set flag to true that we are using a temp file
    // This will need to be deleted afterwards
    $flag_using_temp_file = true;

    // Get file name without extension so we use that as the gunzipped filename
    $file_name_without_ext = basename($vcf_location, ".gz");

    // Gunzip the the file
    shell_exec("gunzip -c " . $vcf_location . " > " . $popstruct_temp_dir . "/" . $file_name_without_ext);

    // Set the vcf_location_temp to where the gunzip file is
    $vcf_location_temp = $popstruct_temp_dir . "/" . $file_name_without_ext;
  }

  tpps_job_logger_write("[VCF_LOCATION_TEMP] $vcf_location_temp");
  echo("[VCF_LOCATION_TEMP] $vcf_location_temp");

  // So now we have th $vcf_location_temp which should be used accordingly


  // Step 1 - Perform PLINK
  // TODO: RESTORE THIS
  tpps_job_logger_write("PERFORM PLINK");
  echo("PERFORM PLINK");
  echo shell_exec($tools_path . '/plink/plink --vcf ' . $vcf_location_temp . " --allow-extra-chr --double-id --make-bed --out "  . $popstruct_temp_dir . '/' . $study_accession.  '_popstruct_plink');


  // Step 2 by x - Fast Structure run
  // To get fastStruct installed, we need the dependenices
  // These dependencies seem to need Python 3.8 / pip3
  // For CENTOS
  // sudo yum -y groupinstall "Development Tools"
  // sudo yum -y install openssl-devel bzip2-devel libffi-devel xz-devel

  // TODO: RESTORE THIS
  for($i=1; $i <= 10; $i++) {
    tpps_job_logger_write("Performing FastStructure for k = $i\n");
    echo("Performing FastStructure for k = $i\n");
    $fast_structure_cmd = 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib; export CFLAGS="-I/usr/local/include"; export LDFLAGS="-L/usr/local/lib";  python ' . $tools_path . "/fastStructure/structure.py -K " . $i . " --input=" . $popstruct_temp_dir . '/' . $study_accession.  '_popstruct_plink' . " --output="  . $popstruct_temp_dir . '/' . $study_accession.  '_popstruct_plink' . ' --full;';
    echo shell_exec($fast_structure_cmd);
  }


  // Step 3 is to select K from previous runs
  // TODO: RESTORE THIS
  tpps_job_logger_write("[INFO] Perform chooseK...\n");
  echo("[INFO] Perform chooseK...\n");
  $chooseK_cmd = 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib; export CFLAGS="-I/usr/local/include"; export LDFLAGS="-L/usr/local/lib"; python ' . $tools_path . '/fastStructure/chooseK.py --input=' . $popstruct_temp_dir . '/' . $study_accession.  '_popstruct_plink';
  $chooseK_output = shell_exec($chooseK_cmd);
  echo $chooseK_output . "\n";

  // Step 3b - from the output, get the suggested K value
  // Go through each line in $chooseK_output
  // TODO: RESTORE THIS
  $chooseK_lines = explode("\n", $chooseK_output);
  $chooseK_lines_count = count($chooseK_lines);
  $chooseK_optimal = 0;
  for ($i=0; $i<$chooseK_lines_count; $i++) {
    $line = $chooseK_lines[$i];
    if ($i == 0) {
      $chooseK_parts = explode('Model complexity that maximizes marginal likelihood = ', $line);
    }
    else if ($i == 1) {
      $chooseK_parts = explode('Model components used to explain structure in data = ', $line);
    }

    // Determine the highest value for use
    if($chooseK_parts[1] > $chooseK_optimal) {
      $chooseK_optimal = $chooseK_parts[1];
    }
  }
  tpps_job_logger_write("Optimal K is " . $chooseK_optimal . "\n");
  echo("Optimal K is " . $chooseK_optimal . "\n");



  // Step 4 - awk and sed to clean up files
  // TODO: RESTORE THIS
  tpps_job_logger_write("AWK AND SED adjustments");
  echo("AWK AND SED adjustments");
  $cmd_custom_cmds1 = "awk 'BEGIN { OFS = \"_\" } ;{print $1,$2}' " . $popstruct_temp_dir . '/' . $study_accession .  '_popstruct_plink.fam > ' . $popstruct_temp_dir . '/' . $study_accession .  "_popstruct_IDPanel.txt;";
  $cmd_custom_cmds1 .= "sed 's/_/\t/g' " . $popstruct_temp_dir . '/' . $study_accession . "_popstruct_IDPanel.txt > " . $popstruct_temp_dir . '/' . $study_accession . "_popstruct_IDPaneltab.txt;";
  $cmd_custom_cmds1 .= "awk '{print $1,$2}' " . $popstruct_temp_dir . '/' . $study_accession . "_popstruct_IDPaneltab.txt > " . $popstruct_temp_dir . '/' . $study_accession . "_popstruct_IDfamPanel.txt;";
  echo shell_exec($cmd_custom_cmds1);

  // // Step 5 - count the population
  $count_output = shell_exec("wc -l " . $popstruct_temp_dir . '/' . $study_accession . "_popstruct_IDPanel.txt");
  tpps_job_logger_write($count_output . "\n");
  echo($count_output . "\n");
  $count_output_parts = explode(' ', $count_output);
  $population_count = $count_output_parts[0];
  tpps_job_logger_write("Population count:" . $population_count . "\n");
  echo("Population count:" . $population_count . "\n");

  // Step 6 - Execute R script which generates popstruct from Panel using chooseK optimal value
  // TODO: RESTORE THIS
  tpps_job_logger_write("RScript popstruct_from_panel execution\n");
  echo("RScript popstruct_from_panel execution\n");
  $cmd_custom_r_code = "Rscript " . $tools_path .  "/popstruct_from_panel.R ";
  $cmd_custom_r_code .= $study_accession . " ";
  $cmd_custom_r_code .= $population_count . " ";
  $cmd_custom_r_code .= $popstruct_temp_dir . '/' . $study_accession . "_popstruct_plink." . $chooseK_optimal. ".meanQ ";
  $cmd_custom_r_code .= $popstruct_temp_dir . '/' . $study_accession . "_popstruct_IDfamPanel.txt ";
  $cmd_custom_r_code .= $popstruct_temp_dir . '/' . $study_accession . "_popstruct_PopPanel.txt";

  echo shell_exec($cmd_custom_r_code);

  // Step 7 - Cleaning up PopPanel columns...
  // TODO: RESTORE THIS
  $cmd_remove_column_code = "cut -d\\  -f2- " . $popstruct_temp_dir . '/' . $study_accession .  "_popstruct_PopPanel.txt > " . $popstruct_temp_dir . '/' . $study_accession . "_popstruct_PopPanel_final.txt";
  echo shell_exec($cmd_remove_column_code);


  // TODO: Push to postgres popstruct table
  // READ THE OUTPUT FILE, GET THE TREE_IDS AND LOCATIONS
  // THEN GO THROUGH THE OUTPUT FILE AND GET THE POPULATION GROUPS
  // THEN ADD THIS TO THE TABLE
  $file_handle = fopen($popstruct_temp_dir . '/' . $study_accession . "_popstruct_PopPanel_final.txt", "r");
  $tree_data = [];
  if ($file_handle) {
    while (($line = fgets($file_handle)) !== false) {
      // process the line read.
      $line_space_parts = explode(" ", $line);
      $tree_id = $study_accession . '-' . $line_space_parts[0];
      $tree_info = [
        'tree_id' => $tree_id,
        'population' => 0,
        'latitude' => 0,
        'longitude' => 0,
        'study_accession' => $study_accession
      ];
      if(count($line_space_parts) >= 4) {
        $population_group = $line_space_parts[3];
        if (strpos($population_group, 'e') !== FALSE) {
          $population_group = 1;
        }
        else {
          $population_group = intval(ceil($population_group)) + 1;
        }
        echo $population_group . ',';
        $tree_info['population'] = $population_group;
        $tree_data[$tree_id] = $tree_info;
      }
    }
    // echo "\n";
    fclose($file_handle);

    // Remove all records from the popstruct table for this study
    tpps_job_logger_write("Removing all popstruct data for accession $study_accession\n");
    echo("Removing all popstruct data for accession $study_accession\n");
    chado_query("DELETE FROM public.cartogratree_popstruct_layer WHERE study_accession = '" . $study_accession . "';");


    // Now query the locations of these tree_ids, so build an SQL statement
    $sql_locations = 'SELECT * FROM public.ct_trees WHERE uniquename IN (';
    $sql_tree_ids_list = '';
    $tree_id_count = 0;
    $sql_tree_ids_list = '';
    foreach($tree_data as $tree_info) {
      if($tree_id_count != 0) {
        $sql_tree_ids_list .= ',';
      }
      $sql_tree_ids_list .= "'" . $tree_info['tree_id'] . "'";
      $tree_id_count = $tree_id_count + 1;
    }
    $sql_locations .= $sql_tree_ids_list;
    // echo $sql_locations . "\n";
    $sql_locations .= ')';
    $results = chado_query($sql_locations);
    foreach($results as $row) {
      $tree_id = $row->uniquename;
      // echo $tree_id . "\n";
      $tree_data[$tree_id]['latitude'] = $row->latitude;
      $tree_data[$tree_id]['longitude'] = $row->longitude;
      $insert_sql = "INSERT INTO public.cartogratree_popstruct_layer (uniquename,population,study_accession,latitude,longitude) ";
      $insert_sql .= "VALUES (";
      $insert_sql .= "'" . $tree_id ."',". $tree_data[$tree_id]['population'] .",";
      $insert_sql .= "'" . $study_accession ."',". $tree_data[$tree_id]['latitude'] ."," . $tree_data[$tree_id]['latitude'] . "";
      $insert_sql .= ")";
      // echo $insert_sql . "\n";
      chado_query($insert_sql);
    }

    tpps_job_logger_write("POPSTRUCT completed.\n");
    echo("POPSTRUCT completed.\n");

  }

}

/**
 * Render genotype combination
 *
 * @param string $raw_value
 *   Tree ID genotype value from VCF file
 * @param string $ref
 *   REF value
 * @param string $alt
 *   ALT value
 */
function tpps_submit_vcf_render_genotype_combination($raw_value, $ref, $alt) {
  // $raw_value = $vcf_line[$j]; // format looks like this: 0/0:27,0:27:81:0,81,1065
  $raw_value_colon_parts = explode(':',$raw_value);
  // Sometimes, the format can look like 0|0 instead of 0/0
  if (stripos($raw_value_colon_parts[0], '|') !== FALSE) {
    $ref_alt_indices = explode('|', $raw_value_colon_parts[0]);
  }
  else {
    $ref_alt_indices = explode('/', $raw_value_colon_parts[0]); // eg 0/0
  }
  $genotype_combination = "";
  $count_indices = count($ref_alt_indices); // 2
  for($k = 0; $k < $count_indices; $k++) { // essentially generating A:G, A:A
    $index_tmp = $ref_alt_indices[$k]; // 0 or 1 (actual value)
    // Remove this code which used to add the :
    // if($k > 0) {
    //   $genotype_combination .= ':';
    // }
    if($index_tmp == 0) {
      $genotype_combination .= $ref;
    }
    else {
      // Index_tmp value is 1 or higher
      // We need to process $alt since alt could have comma separated values
      $alt_csv = explode(',', $alt);

      // Since index_tmp = 1 would be the first alt_csv[0], we can just make a new index_tmp_alt to know the index of alt_csv
      $index_tmp_alt = $index_tmp - 1;

      // OLD code that didn't cater for index_tmp being anything other than 1
      // $genotype_combination .= $alt;

      // NEW code 3/27/2023 which caters for alt being a csv
      $genotype_combination .= $alt_csv[$index_tmp_alt];

    }
  }
  return $genotype_combination;
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
function tpps_submit_environment(array &$form_state, $i, TripalJob &$job = NULL) {
  tpps_job_logger_write('[INFO] - Submitting environment data...');
  $job->logMessage('[INFO] - Submitting environment data...');
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $environment = $fourthpage["organism-$i"]['environment'] ?? NULL;
  if (empty($environment)) {
    return;
  }
  tpps_submission_add_tag($form_state['accession'], 'Environment');

  $env_layers = isset($environment['env_layers']) ? $environment['env_layers'] : FALSE;
  $env_params = isset($environment['env_params']) ? $environment['env_params'] : FALSE;
  $env_count = 0;

  $species_index = "species-$i";
  if (empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
    $species_index = "species-1";
  }
  $tree_accession = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index];
  $tree_acc_fid = $tree_accession['file'];
  if (!empty($form_state['revised_files'][$tree_acc_fid]) and (file_load($form_state['revised_files'][$tree_acc_fid]))) {
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
      if ($layer_name == 'other' or $layer_name == 'other_db' or $layer_name == 'other_name' or $layer_name == 'other_params') {
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
    tpps_job_logger_write('[INFO] - Processing environment_layers file data...');
    $job->logMessage('[INFO] - Processing environmental_layers file data...');
    tpps_file_iterator($tree_acc_fid, 'tpps_process_environment_layers', $options);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');

    tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
    $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
    tpps_chado_insert_multi($options['records']);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');
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
  $org_full_name = $row[$cols['org']] ?? "{$row[$cols['genus']]} {$row[$cols['species']]}";
  if ($search == $org_full_name) {
    $options['found'] = TRUE;
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
  global $tpps_job;
  $job = $tpps_job;
  $columns = $options['meta_columns'];
  $meta = &$options['meta'];

  // print_r("COLUMNS:\n");
  // print_r($columns);
  // print_r("ROW:\n");
  // print_r($row);

  $name = strtolower($row[$columns['name']]);
  $meta[$name] = array();
  $meta[$name]['attr'] = 'other';
  $meta[$name]['attr-other'] = $row[$columns['attr']];
  $meta[$name]['desc'] = $row[$columns['desc']];
  $meta[$name]['unit'] = 'other';
  $meta[$name]['unit-other'] = $row[$columns['unit']];
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
function tpps_refine_phenotype_meta(array &$meta, array $time_options = array(), TripalJob &$job = NULL) {
  $cvt_cache = array();
  $local_cv = chado_get_cv(array('name' => 'local'));
  $local_db = variable_get('tpps_local_db');
  $term_types = array(
    'attr' => array(
      'label' => 'Attribute',
      'ontology' => 'pato',
    ),
    'unit' => array(
      'label' => 'Unit',
      'ontology' => 'po',
    ),
    'struct' => array(
      'label' => 'Structure',
      'ontology' => 'po',
    ),
  );
  // print_r($meta);
  foreach ($meta as $name => $data) {
    // echo "Name: $name\n";
    // echo "Data:\n";
    // print_r ($data);
    // echo "\n";
    foreach ($term_types as $type => $info) {
      $meta[$name]["{$type}_id"] = $data["{$type}"];
      
      if ($data["{$type}"] == 'other') {
        $meta[$name]["{$type}_id"] = $cvt_cache[$data["{$type}-other"]] ?? NULL;
        if (empty($meta[$name]["{$type}_id"])) {
          $result = tpps_ols_install_term("{$info['ontology']}:{$data["{$type}-other"]}");
          if ($result !== FALSE) {
            $meta[$name]["{$type}_id"] = $result->cvterm_id;
            $job->logMessage("[INFO] New OLS Term {$info['ontology']}:{$data["{$type}-other"]} installed");
          }

          if (empty($meta[$name]["{$type}_id"])) {
            $term = chado_select_record('cvterm', array('cvterm_id'), array(
              'name' => array(
                'data' => $data["{$type}-other"],
                'op' => 'LIKE',
              ),
            ), array(
              'limit' => 1,
            ));
            $meta[$name]["{$type}_id"] = current($term)->cvterm_id ?? NULL;
          }

          if (empty($meta[$name]["{$type}_id"])) {
            $meta[$name]["{$type}_id"] = chado_insert_cvterm(array(
              'id' => "{$local_db->name}:{$data["{$type}-other"]}",
              // 'name' => $data["{$type}-other"],
              'name' => $data["{$type}"] . '-other',
              'definition' => '',
              'cv_name' => $local_cv->name,
            ))->cvterm_id;
            if (!empty($meta[$name]["{$type}_id"])) {
              $job->logMessage("[INFO] New Local {$info['label']} Term {$data["{$type}-other"]} installed");
            }
          }
          $cvt_cache[$data["{$type}-other"]] = $meta[$name]["{$type}_id"];
        }
      }
    }

    if (!empty($time_options['time_phenotypes'][strtolower($name)])) {
      $meta[$name]['time'] = $time_options['time_values'][strtolower($name)];
      if (empty($meta[$name]['time'])) {
        $meta[$name]['time'] = TRUE;
      }
    }
  }
  print_r($meta);
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
  global $tpps_job;
  $job = $tpps_job;
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
  $organism_name = &$options['organism_name'];
  $record_group = variable_get('tpps_record_group', 10000);
  // $record_group = 1;

  // Get genus and species from the organism name
  $organism_name_parts = explode(' ', $organism_name, 2);
  $genus = $organism_name_parts[0];
  $species = $organism_name_parts[1];

  // Ensure that we got the genus and species or error out
  if ($genus == "" || $species == "") {
    throw new Exception('Organism genus and species could not be processed. Please ensure you added an organism that exists within the chado.organism table!');
  }

  // Query the organism table to get the organism id
  $organism_id_results = chado_query('SELECT * FROM chado.organism WHERE genus = :genus and species = :species ORDER BY organism_id ASC LIMIT 1', array(
    ':genus' => $genus,
    ':species' => $species
  ));

  // Dummy value for organism_id until we get it from the sql results row
  $organism_id = -1;
  foreach($organism_id_results as $organism_id_row) {
    $organism_id = $organism_id_row->organism_id;
  }

  // Check that the organism id is valid
  if($organism_id == -1 || $organism_id == "") {
    throw new Exception('Could not find organism id for ' . $organism_name. '. This organism does not seem to exist in the chado.organism table!');
  }

  $cvterm_id_4lettercode = -1;
  // Get the cvterm_id (which is the type_id) for the organism 4 letter code
  $cvterm_results = chado_query('SELECT * FROM chado.cvterm WHERE name = :name LIMIT 1', array(
    ':name' => 'organism 4 letter code'
  ));
  foreach($cvterm_results as $cvterm_row) {
    $cvterm_id_4lettercode = $cvterm_row->cvterm_id;
  }
  if($cvterm_id_4lettercode == -1 || $cvterm_id_4lettercode == "") {
    throw new Exception('Could not find the cvterm id for organism 4 letter code within the chado.cvterm table. This is needed to generate the phenotype name.');
  }

  // We need to use the cvterm_id 4 letter code to find the actual code within the organismprop table (using the organism_id)
  $value_4lettercode = "";
  $organismprop_results = chado_query('SELECT * FROM chado.organismprop WHERE type_id = :type_id AND organism_id = :organism_id LIMIT 1', array(
    ':type_id' => $cvterm_id_4lettercode,
    ':organism_id' => $organism_id
  ));
  foreach ($organismprop_results as $organismprop_row) {
    $value_4lettercode = $organismprop_row->value;
  }

  if($value_4lettercode == "" || $value_4lettercode == null) {
    throw new Exception('4 letter code could not be found for ' . $organism_name . ' in the chado.organismprop table. This is needed to create the phenotype_name.');
  }

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
  if ($iso) {
    foreach ($row as $id => $value) {
      if (empty($tree_id)) {
        $tree_id = $value;
        continue;
      }
      $values[$id] = $file_headers[$id];
    }
  }

  if($tree_id == null || $tree_id == "") {
    throw new Exception('tree_id was null or empty - there might be a problem with the format of the phenotype data file or selected column options for the file via the user information, cannot continue until resolved.');
  }


  // print_r($values);
  // throw new Exception('DEBUG');
  $phenotype_name_previous = "<none set>";
  foreach ($values as $id => $name) {
    if($name == null || $name == "") {
      throw new Exception('Phenotype name was null or empty - there might be a problem with the format of the phenotype data file or selected column options for the file via the user information, cannot continue until resolved.');
    }
    $attr_id = $iso ? $meta['attr_id'] : $meta[strtolower($name)]['attr_id'];
    // throw new Exception('debug');
    if($attr_id == null || $attr_id == "") {
      print_r('$meta[attr_id]:' . $meta['attr_id'] . "\n");
      print_r('$name:' . $name . "\n");
      print_r('$meta[$name]:' . $meta[strtolower($name)]['attr_id'] . "\n");
      print_r('$attr_id:' . $attr_id . "\n");
      throw new Exception('Attribute id is null which causes phenotype data to not be added to database correctly.');
    }
    $value = $row[$id];
    $phenotype_name = "$accession-$tree_id-$name-$suffix";
    $phenotype_name .= '-' . $value_4lettercode;
    $options['data']["$tree_id-$name-$suffix"] = array(
      'uniquename' => "$tree_id-$name-$suffix",
      'name' => $name,
      'stock_id' => $tree_info[$tree_id]['stock_id'],
      'time' => NULL,
      'value' => $value,
    );

    $struct_id = NULL;
    if (isset($meta[strtolower($name)]['struct_id'])) {
      $struct_id = $meta[strtolower($name)]['struct_id'];
    }
    else if ($iso) { //override the value - likely for intensity / mass spectrometry
      $struct_id = $meta['struct_id'];
    }


    $records['phenotype'][$phenotype_name] = array(
      'uniquename' => $phenotype_name,
      'name' => $name,
      'attr_id' => $attr_id,
      // removed this old obserable_id to cater for mass spectrometry
      // 'observable_id' => $meta[strtolower($name)]['struct_id'] ?? NULL,
      // this is the new way of adding observable_id to cater for mass spectrometry as well
      'observable_id' => $struct_id,
      'value' => $value,
    );
    // print_r($records['phenotype'][$phenotype_name]);

    $records['stock_phenotype'][$phenotype_name] = array(
      'stock_id' => $tree_info[$tree_id]['stock_id'],
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );
    // print_r($records['stock_phenotype'][$phenotype_name]);

    if (isset($meta[strtolower($name)]['time'])) {
      $records['phenotypeprop']["$phenotype_name-time"] = array(
        'type_id' => $cvterms['time'],
        'value' => $meta[strtolower($name)]['time'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
      // print_r($records['phenotypeprop']["$phenotype_name-time"]);
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
      // print_r($records['phenotypeprop']["$phenotype_name-time"]);
      $options['data'][$phenotype_name]['time'] = $val;
    }

    // print_r($meta);
    $records['phenotypeprop']["$phenotype_name-desc"] = array(
      'type_id' => $cvterms['desc'],
      'value' => $iso ? $meta['desc'] : $meta[strtolower($name)]['desc'],
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );
    // print_r($phenotype_name-desc . "\n");
    // print_r($records['phenotypeprop']["$phenotype_name-desc"]);

    if ($iso) {
      $records['phenotypeprop']["$phenotype_name-unit"] = array(
        'type_id' => $cvterms['unit'],
        'value' => $meta['unit'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
      // print_r($records['phenotypeprop']["$phenotype_name-unit"]);
    }

    if (!$iso) {
      $records['phenotype_cvterm']["$phenotype_name-unit"] = array(
        'cvterm_id' => $meta[strtolower($name)]['unit_id'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
      // print_r($records['phenotype_cvterm']["$phenotype_name-unit"]);
    }

    if (isset($meta[strtolower($name)]['min'])) {
      $records['phenotypeprop']["$phenotype_name-min"] = array(
        'type_id' => $cvterms['min'],
        'value' => $meta[strtolower($name)]['min'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
      // print_r($records['phenotypeprop']["$phenotype_name-min"]);
    }

    if (isset($meta[strtolower($name)]['max'])) {
      $records['phenotypeprop']["$phenotype_name-max"] = array(
        'type_id' => $cvterms['max'],
        'value' => $meta[strtolower($name)]['max'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
      // print_r($records['phenotypeprop']["$phenotype_name-max"]);
    }

    if (!empty($meta[strtolower($name)]['env'])) {
      $records['phenotype_cvterm']["$phenotype_name-env"] = array(
        'cvterm_id' => $cvterms['environment'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
      // print_r($records['phenotype_cvterm']["$phenotype_name-env"]);
    }



    if ($phenotype_count >= $record_group) {
      // print_r($records);
      // print_r('------------' . "\n");
      tpps_job_logger_write('[INFO] -- Inserting data into database using insert_multi...');
      $job->logMessage('[INFO] -- Inserting data into database using insert_multi...');
      // print_r($records);
      tpps_chado_insert_multi($records);
      tpps_job_logger_write('[INFO] - Done.');
      $job->logMessage('[INFO] - Done.');

      // $temp_results = chado_query('SELECT * FROM chado.phenotype WHERE uniquename ILIKE :phenotype_name', array(
      //   ':phenotype_name' => $phenotype_name
      // ));
      // foreach($temp_results as $temp_row) {
      //   echo "Found phenotype saved: " . $temp_row->uniquename . "\n";
      // }

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
  global $tpps_job;
  $job = $tpps_job;
  $type = $options['type'];
  $organism_index = $options['organism_index'];
  $records = &$options['records'];
  $records2 = &$options['records2']; // hybrid / copy such as genotype_call records
  $headers = $options['headers'];
  $tree_info = &$options['tree_info'];
  $species_codes = $options['species_codes'];
  $genotype_count = &$options['genotype_count'];
  $project_id = $options['project_id'];
  $marker = $options['marker'];
  // Marker adjustment [RISH: 8/1/2023]
  if ($marker == 'SSRs') {
    $marker = 'SSR';
  }
  else if($marker == 'cpSSRs') {
    $marker = 'cpSSR';
  }

  $type_cvterm = $options['type_cvterm'];
  $seq_var_cvterm = $options['seq_var_cvterm'];
  $multi_insert_options = $options['multi_insert'];
  $associations = $options['associations'] ?? array();
  $vcf_processing_completed = $options['vcf_processing_completed'];
  $analysis_id = $options['analysis_id'];
  // echo "Analysis ID: $analysis_id\n";

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

    // RISH NOTES: This addition uses the organism_id based on the organism order
    // of the fourth page (we likely have to pass the i from previous function here)
    // THIS TECHNICALLY OVERRIDES PETER'S LOGIC ABOVE. TO BE DETERMINED IF RISH'S WAY IS CORRECT
    // OR NOT [6/22/2023]
    // THIS WAS AN ISSUE BROUGHT UP BY EMILY REGARDING SNPS NOT BEING ASSOCIATED WITH POP TRICH (665 STUDY)
    $species_code = null;
    $organism_id = null;
    $count_tmp = 0;
    foreach ($species_codes as $organism_id_tmp => $species_code_tmp) {
      $count_tmp = $count_tmp + 1; // increment
      // Check if count_tmp matches $organism_index
      if ($count_tmp == $organism_index) {
        $species_code = $species_code_tmp;
        $organism_id = $organism_id_tmp;
        break;
      }
    }

    $variant_name = $headers[$key];
    $marker_name = $variant_name . $marker;
    $genotype_name_without_call = "$marker-$variant_name-$species_code";
    $genotype_name = "$marker-$variant_name-$species_code-$val";

    // echo "Variant Name: $variant_name\n";
    // echo "Marker Name: $marker_name\n";
    // echo "Genotype name: $genotype_name\n";

    // THIS IS SUPER SLOW EVEN FOR TESTING PURPOSES
    // if (isset($options['test'])) {
    //   // DELETE marker_name feature if it already exists
    //   chado_query("DELETE FROM chado.feature WHERE uniquename = :marker_name", [
    //     ':marker_name' => $marker_name
    //   ]);

    //   // DELETE marker_name feature if it already exists
    //   chado_query("DELETE FROM chado.feature WHERE uniquename = :variant_name", [
    //     ':variant_name' => $variant_name
    //   ]);
    // }   

    // [RISH] 07/06/2023 - REMOVED SO WE CAN INSERT TO GET FEATURE ID
    // $records['feature'][$marker_name] = array(
    //   // 'organism_id' => $current_id, // PETER's original code
    //   'organism_id' => $organism_id, // RISH code override 6/22/2023
    //   'uniquename' => $marker_name,
    //   'type_id' => $seq_var_cvterm,
    // );
    chado_insert_record('feature', [
      'name' => $marker_name,
      'organism_id' => $organism_id,
      'uniquename' => $marker_name,
      'type_id' => $seq_var_cvterm
    ]);
    
    // Lookup the marker_name_id
    $results = chado_query("SELECT feature_id FROM chado.feature 
      WHERE uniquename = :uniquename AND organism_id = :organism_id", [
        ':uniquename' => $marker_name,
        ':organism_id' => $organism_id
    ]);
    $marker_name_id = NULL;
    foreach ($results as $row) {
      $marker_name_id = $row->feature_id;
    }

    // [RISH] 07/06/2023 - REMOVED SO WE CAN INSERT TO GET FEATURE ID
    // $records['feature'][$variant_name] = array(
    //   // 'organism_id' => $current_id, // PETER's original code
    //   'organism_id' => $organism_id, // RISH code override 6/22/2023
    //   'uniquename' => $variant_name,
    //   'type_id' => $seq_var_cvterm,
    // );

    chado_insert_record('feature', [
      'name' => $variant_name,
      'organism_id' => $organism_id,
      'uniquename' => $variant_name,
      'type_id' => $seq_var_cvterm
    ]);
    
    // Lookup the marker_name_id
    $results = chado_query("SELECT feature_id FROM chado.feature 
      WHERE uniquename = :uniquename AND organism_id = :organism_id", [
        ':uniquename' => $variant_name,
        ':organism_id' => $organism_id
    ]);
    $variant_name_id = NULL;
    foreach ($results as $row) {
      $variant_name_id = $row->feature_id;
    }    


    if (!empty($associations) and !empty($associations[$variant_name])) {
      $association = $associations[$variant_name];
      $assoc_feature_name = "{$variant_name}-{$options['associations_type']}-{$association['trait']}";

      echo "Association data for this row is being processed\n";
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

      // PETER's code - which doesn't connect to analysis
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

    // RISH 7/17/2023
    // Check if vcf was not processed, then make sure to 
    // add records for genotype and genotype call.
    // If however VCF is processed, we don't need to add these records.
    if ($vcf_processing_completed == true && $type == 'snp') {
      //skip performing genotype and genotype_call inserts
    }
    else {
      // [RISH] 07/06/2023 - REMOVED SO WE CAN INSERT TO GET ID
      // $records['genotype'][$genotype_name] = array(
      //   'name' => $genotype_name,
      //   'uniquename' => $genotype_name,
      //   'description' => $val,
      //   'type_id' => $type_cvterm,
      // );
      chado_insert_record('genotype', [
        'name' => $genotype_name_without_call,
        'uniquename' => $genotype_name,
        'description' => $val,
        'type_id' => $type_cvterm,   
      ]);

      $results = chado_query('SELECT genotype_id FROM chado.genotype WHERE uniquename = :uniquename', [
        ':uniquename' => $genotype_name
      ]);
      $genotype_id = NULL;
      foreach ($results as $row) {
        $genotype_id = $row->genotype_id;
      }     

      // [RISH] 07/06/2023 - REMOVED SO WE CAN USE HYBRID COPY SYSTEM
      // $records['genotype_call']["$stock_id-$genotype_name"] = array(
      //   'project_id' => $project_id,
      //   'stock_id' => $stock_id,
      //   '#fk' => array(
      //     'genotype' => $genotype_name,
      //     'variant' => $variant_name,
      //     'marker' => $marker_name,
      //   ),
      // );
      $records2['genotype_call']["$stock_id-$genotype_name"] = array(
        'project_id' => $project_id,
        'stock_id' => $stock_id,
        'genotype_id' => $genotype_id,
        'variant_id' => $variant_name_id,
        'marker_id' => $marker_name_id,
      );

      // $records['stock_genotype']["$stock_id-$genotype_name"] = array(
      //   'stock_id' => $stock_id,
      //   '#fk' => array(
      //     'genotype' => $genotype_name,
      //   ),
      // );

      $records['stock_genotype']["$stock_id-$genotype_name"] = array(
        'stock_id' => $stock_id,
        'genotype_id' => $genotype_id
      ); 
    }

    if ($genotype_count >= $record_group) {
      if ($vcf_processing_completed == true && $type == 'snp') {
        tpps_job_logger_write('[INFO] - Skipped genotype and genotype_call SNPs since VCF already loaded...');
        $job->logMessage('[[INFO] - Skipped genotype and genotype_call SNPs since VCF already loaded...');
      }
      tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
      $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
      tpps_chado_insert_multi($records, $multi_insert_options);
      tpps_job_logger_write('[INFO] - Inserting data into database using insert_hybrid...');
      $job->logMessage('[INFO] - Inserting data into database using insert_hybrid...');
      tpps_chado_insert_hybrid($records, $multi_insert_options);      
      tpps_job_logger_write('[INFO] - Done.');
      $job->logMessage('[INFO] - Done.');
      $records = array(
        'feature' => array(),
        'genotype' => array(),
        'genotype_call' => array(),
        'stock_genotype' => array(),
      );
      // Do this for the hybrid (COPY) command
      $records2 = array(
        'genotype_call' => array(),
      );
      if (!empty($associations)) {
        $records['featureloc'] = array();
        $records['featureprop'] = array();
      }
      $options['genotype_total'] += $genotype_count;
      tpps_job_logger_write('[INFO] - Genotypes inserted:' + $options['genotype_total']);
      $job->logMessage('[INFO] - Genotypes inserted:' + $options['genotype_total']);
      $genotype_count = 0;
    }
  }
}

/**
 * This function will process each line the genotype SNP design assay file
 * Initially done to get position data from the assay design file
 */
function tpps_process_genotype_snp_assay_design($row, array &$options = array()) {
  $line_rows = $row;
  $analysis_id = $options['analysis_id']; // needed to lookup source features
  $headers = &$options['headers'];
  $records = &$options['records'];
  $columns = $options['file_columns'];
  // print_r("File columns: ");
  // print_r($columns);
  // print_r("\n");
  // print_r("Data in row:");
  // print_r($line_rows);
  // print_r("\n");

  $chr_name = $line_rows[$columns['chr']];
  // // if the scaffold is only the number, we append scaffold_ to it (TGDR665)
  // if (is_numeric($chr_name)) {
  //   $chr_name = "scaffold_" . $chr_name;
  // }
  // // if TGDR665, replace chr with scaffold_
  // if (substr($chr_name,0,3) == 'chr') {
  //   $chr_name = str_replace('chr', 'scaffold_', $chr_name);
  // }

  $marker_name_raw = $line_rows[$columns['marker_name']];
  $position = intval($line_rows[$columns['position']]);
  $marker_type = $options['marker_type']; // we could force 'SNP' here

  $srcfeature_id = NULL;
  // Get the srcfeature_id
  $srcfeature_results = chado_query('select feature.feature_id from chado.feature
    join chado.analysisfeature on feature.feature_id = analysisfeature.feature_id
    where feature.name = :chr_name and analysisfeature.analysis_id = :analysis_id',
    [
      ':chr_name' => $chr_name,
      ':analysis_id' => $analysis_id
    ]
  );

  foreach ($srcfeature_results as $row) {
    $srcfeature_id = $row->feature_id;
  }

  if ($srcfeature_id != NULL) {
    echo "[GOOD] srcfeature_id for $chr_name: " . $srcfeature_id . "\n";

    $marker_name = $marker_name_raw . $marker_type;

    // We need to find the current marker_name in the feature table
    $results = chado_query("SELECT * FROM chado.feature WHERE uniquename = :uniquename", [
      ':uniquename' => $marker_name
    ]);
    $feature_id = NULL;
    foreach ($results as $feature) {
      $feature_id = $feature->feature_id;
    }

    if ($feature_id != NULL) {
      echo "[GOOD] Marker name $marker_name has feature_id: $feature_id\n";

      // Before we add a new featureloc record, check to make sure one does not already exist
      // in the featureloc table since we don't currently delete previous featurelocs on
      // study reloads
      $featureloc_results = chado_query('SELECT count(*) as c1 FROM chado.featureloc
        WHERE feature_id = :feature_id AND srcfeature_id = :srcfeature_id;', [
          ':feature_id' => $feature_id,
          ':srcfeature_id' => $srcfeature_id
        ]
      );
      $featureloc_count = 0;
      foreach ($featureloc_results as $row) {
        $featureloc_count = $row->c1;
      }
      // This means no featureloc exists, so insert it
      if ($featureloc_count == 0) {
        // Check for indels (longer than 1 reads)
        $snp = trim($line_rows[$columns['snp']]);
        $snp_possible_reads = explode('/', $snp);
        $read_length = 0; // default for non-indels
        foreach ($snp_possible_reads as $read) {
          $tmp_read_length = strlen($read);
          if ($tmp_read_length > 1) { // only for indels, we need a read_length of more than 0
            if ($tmp_read_length > $read_length) {
              $read_length = $tmp_read_length;
            }
          }
        }

        // TODO: if read_length is more than 1, it is an indel, change marker type
        // Also check code in tpps_process_genotype_sheet to check this or else
        // these 2 things will cause a submit failure - after conference (6/15/2023)

        $records['featureloc'][$marker_name] = [
          'fmin' => $position,
          'fmax' => ($position + $read_length),
          'srcfeature_id' => $srcfeature_id,
          'feature_id' => $feature_id,
        ];
        print_r($records['featureloc'][$marker_name]);
      }
      else {
        echo "[GOOD ALTERNATIVE] Featureloc record already exists, no need to add\n";
      }

      // Check if forward sequence information has been added
      $forward_sequence_cvterm_id = NULL;
      // Get cvterm_id (assuming it exists)
      $results = chado_query("SELECT * FROM chado.cvterm
        WHERE name = 'five_prime_flanking_region' LIMIT 1;", []);
      foreach ($results as $row) {
        $forward_sequence_cvterm_id = $row->cvterm_id;
      }

      // Check if record already exists
      $results = chado_query("SELECT count(*) as c1 FROM chado.featureprop
        WHERE feature_id = :feature_id AND type_id = :type_id;", [
          ':feature_id' => $feature_id,
          ':type_id' => $forward_sequence_cvterm_id
      ]);
      $count = $results->fetchObject()->c1;
      // If record not found
      if ($count == 0) {
        // add to record to featureprop table
        $records['featureprop'][$feature_id . $forward_sequence_cvterm_id] = [
          'feature_id' => $feature_id,
          'type_id' => $forward_sequence_cvterm_id,
          'value' => $line_rows[$columns['forward_sequence']]
        ];
      }


      // Check if reverse sequence information has been added
      $reverse_sequence_cvterm_id = NULL;
      // Get cvterm_id (assuming it exists)
      $results = chado_query("SELECT * FROM chado.cvterm
        WHERE name = 'three_prime_flanking_region' LIMIT 1;", []);
      foreach ($results as $row) {
        $reverse_sequence_cvterm_id = $row->cvterm_id;
      }

      // Check if record already exists
      $results = chado_query("SELECT count(*) as c1 FROM chado.featureprop
        WHERE feature_id = :feature_id AND type_id = :type_id;", [
          ':feature_id' => $feature_id,
          ':type_id' => $reverse_sequence_cvterm_id
      ]);
      $count = $results->fetchObject()->c1;
      // If record not found
      if ($count == 0) {
        // add record to featureprop table
        $records['featureprop'][$feature_id . $reverse_sequence_cvterm_id] = [
          'feature_id' => $feature_id,
          'type_id' => $reverse_sequence_cvterm_id,
          'value' => $line_rows[$columns['reverse_sequence']]
        ];
      }
    }
    else {
      echo "[ERROR] Marker name $marker_name feature_id could not be found\n";
    }
  }
  else {
    echo "[ERROR] srcfeature_id for $chr_name could not be found - we cannot add featureloc data.\n";
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
  global $tpps_job;
  $job = $tpps_job;
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

  foreach (array_keys($headers) as $key) {
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
          $marker_num++;
          break;
        }
        $results[$key] = "{$last}_$ploidy_suffix";
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
 * This will generate all genotype materialized views
 */
function tpps_generate_all_genotype_materialized_views() {
  // Get count of all studies
  $results = chado_query('SELECT COUNT(DISTINCT(accession)) as c1 FROM "public"."tpps_submission";',[]);
  $total = 0;
  foreach ($results as $row) {
    $total = $row->c1;
  }
  $current_count = 0;

  // Get all the study accessions from database
  $results = chado_query('SELECT DISTINCT(accession) as accession FROM "public"."tpps_submission" ORDER BY accession;',[]);
  foreach ($results as $row) {
    $current_count = $current_count+1;
    echo "Processing genotype materialized view: $current_count of $total\n";
    // Get the submission state
    $state = tpps_load_submission($row->accession);
    // Check if there's a project_id in the submission state
    $project_id = $state['ids']['project_id'] ?? NULL;
    // Once the project_id is not null, we're good
    if (isset($project_id)) {
      tpps_generate_genotype_materialized_view($project_id);
    }
  }
}

/**
 * This function will generate a genotype materialized view for
 * the specific project_id (which you must get from the state object).
 * This is used for the tpps/details genotypes tab
 *
 *
 * @param mixed $project_id
 *   The project ID of the study. NOT THE STUDY ACCESSION!
 *
 *
 */
function tpps_generate_genotype_materialized_view($project_id) {
  // Ensure the project_id is an integer
  $project_id = intval($project_id);
  if ($project_id <= 0) {
    return;
  }
  $view_name = 'chado.genotypes_' . $project_id;

  // Added to fix previous queries that used stock_genotype which we don't
  // want to keep using.
  echo "Attempting to drop materialized view (if exist): " . $view_name . "\n";
  chado_query("DROP MATERIALIZED VIEW IF EXISTS " . $view_name . ";");

  echo "Attempting to create materialized view (if it does not exist): " . $view_name . "\n";

  // @ DEPRECATED - Used stock_genotype which we want to avoid due to size.
  // We still use stock_genotype for Tripal Entities
  // chado_query('CREATE MATERIALIZED VIEW IF NOT EXISTS ' . $view_name . ' AS ' .
  //   "(SELECT g.genotype_id AS
  //   genotype_id,
  //   g.name AS name,
  //   g.uniquename AS uniquename,
  //   g.description AS description,
  //   g.type_id AS type_id,
  //   s.uniquename AS s_uniquename,
  //   s.stock_id AS stock_id
  //   FROM chado.genotype g
  //   INNER JOIN chado.stock_genotype sg ON sg.genotype_id = g.genotype_id
  //   INNER JOIN chado.project_stock ps ON ps.stock_id = sg.stock_id
  //   INNER JOIN chado.stock s ON s.stock_id = sg.stock_id
  //   WHERE (ps.project_id = '" . $project_id . "')" . ') ' .
  //   "WITH NO DATA"
  // ,[]);

  // @NEW - This code uses the genotype_call table only - less joins - more efficient
  // chado_query('CREATE MATERIALIZED VIEW ' . $view_name . ' AS ' .
  // "(SELECT g.genotype_id AS
  //   genotype_id,
  //   g.name AS name,
  //   g.uniquename AS uniquename,
  //   g.description AS description,
  //   g.type_id AS type_id,
  //   s.uniquename AS s_uniquename,
  //   s.stock_id AS stock_id
  //   FROM chado.genotype g
  //   INNER JOIN chado.genotype_call gc ON gc.genotype_id = g.genotype_id
  //   INNER JOIN chado.stock s ON s.stock_id = gc.stock_id
  //   WHERE (gc.project_id = '" . $project_id . "') ) " .
  //   "WITH NO DATA"
  // ,[]);

  // @NEW V2 - This adds an index_id column
  chado_query('CREATE MATERIALIZED VIEW ' . $view_name . ' AS ' .
  "(SELECT
    row_number() over (order by g.genotype_id, s.stock_id) as index_id,
    g.genotype_id AS genotype_id,
    g.name AS name,
    g.uniquename AS uniquename,
    g.description AS description,
    g.type_id AS type_id,
    s.uniquename AS s_uniquename,
    s.stock_id AS stock_id
    FROM chado.genotype g
    INNER JOIN chado.genotype_call gc ON gc.genotype_id = g.genotype_id
    INNER JOIN chado.stock s ON s.stock_id = gc.stock_id
    WHERE (gc.project_id = '" . $project_id . "') ) " .
    "WITH NO DATA"
  ,[]);



  // Generate the data / regenerate if necessary
  echo "Refreshing materialized view: " . $view_name . "\n";
  chado_query('REFRESH MATERIALIZED VIEW ' . $view_name, []);
  echo "Finished refresh of " . $view_name . "\n";

  // Add an index using the index_id column (IF NOT EXISTS)
  echo "Adding index_id index to $view_name\n";
  chado_query("CREATE UNIQUE INDEX " . str_replace('chado.', 'chado_' , $view_name) .
    "_id on " . $view_name . "(index_id);", []);
  echo "Index completed\n";

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
  global $tpps_job;
  $job = $tpps_job;
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
    'type_id' => tpps_load_cvterm('gps_latitude')->cvterm_id,
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

    foreach (array_keys($params) as $param_id) {
      $param_query = db_select('cartogratree_fields', 'f')
        ->fields('f', array('field_name'))
        ->condition('field_id', $param_id)
        ->execute();

      $param_name = $param_query->fetchObject()->field_name;
      $phenotype_name = "$accession-$tree_id-$layer_name-$param_name-$suffix";

      $value = tpps_get_environmental_layer_data($layer_id, $lat, $long, $param_name);
      $type = variable_get("tpps_param_{$param_id}_type", 'attr_id');

      $records['phenotype'][$phenotype_name] = array(
        'uniquename' => $phenotype_name,
        'name' => "$param_name",
        'value' => "$value",
      );

      $records['stock_phenotype'][$phenotype_name] = array(
        'stock_id' => $stock_id,
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );

      if ($type == 'attr_id') {
        $records['phenotype'][$phenotype_name]['attr_id'] = $env_cvterm;
      }
      if ($type != 'attr_id') {
        $records['phenotype_cvterm'][$phenotype_name] = array(
          'cvterm_id' => $env_cvterm,
          '#fk' => array(
            'phenotype' => $phenotype_name,
          ),
        );
      }

      $env_count++;
      if ($env_count >= $record_group) {
        tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
        $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
        tpps_chado_insert_multi($records);
        tpps_job_logger_write('[INFO] - Done.');
        $job->logMessage('[INFO] - Done.');
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
 * This function will add back genotype_call indexes
 */
function tpps_create_genotype_call_indexes($job) {
  echo "SET CONSTRAINTS ALL IMMEDIATE\n";
  chado_query("SET CONSTRAINTS ALL IMMEDIATE;",[]);
  
  // RECREATE INDEXES
  tpps_job_logger_write('[INFO] - Recreating indexes...');
  $job->logMessage('[INFO] - Recreating indexes...'); 
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_genotype_id_idx ON chado.genotype_call USING btree (genotype_id)");
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_project_id_idx ON chado.genotype_call USING btree (project_id)");
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_stock_id_idx ON chado.genotype_call USING btree (stock_id)");
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_marker_id_idx ON chado.genotype_call USING btree (marker_id)");
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_variant_id_idx ON chado.genotype_call USING btree (variant_id)");
  tpps_job_logger_write('[INFO] - Recreating INDEXES - Done.');
  $job->logMessage('[INFO] - Recreating INDEXES - Done.'); 
  chado_query("SET CONSTRAINTS ALL DEFERRED;",[]);


}

/**
 * This function will drop genotype_call indexes from the database
 */
function tpps_drop_genotype_call_indexes($job) {
  tpps_job_logger_write('[INFO] - Dropping indexes...');
  $job->logMessage('[INFO] - Dropping indexes...'); 
  chado_query("DROP INDEX IF EXISTS chado.genotype_call_genotype_id_idx;", []);
  chado_query("DROP INDEX IF EXISTS chado.genotype_call_project_id_idx;", []);
  chado_query("DROP INDEX IF EXISTS chado.genotype_call_stock_id_idx;", []);
  chado_query("DROP INDEX IF EXISTS chado.genotype_call_marker_id_idx;", []);
  chado_query("DROP INDEX IF EXISTS chado.genotype_call_variant_id_idx;", []);
  tpps_job_logger_write('[INFO] - Done.');
  $job->logMessage('[INFO] - Done.'); 
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
  $response = explode("\n", $response);
  if ($response) {
    $response = array_slice($response, 2, -2);
    foreach ($response as $line) {
      $item = explode("=", $line);
      if ($item and trim($item[0]) == $param) {
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
function tpps_process_accession($row, array &$options, $job = NULL) {
  global $tpps_job;
  $job = $tpps_job;
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
    $job->logMessage('[INFO] CV Terms Data' . print_r($cvterm, 1));
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

    if (isset($geo_api_key) and !array_key_exists($location, $options['locations'])) {
      $query = urlencode($location);
      $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
      $response = json_decode(file_get_contents($url));
      $options['locations'][$location] = $response->results[0]->geometry ?? NULL;

      if ($response->total_results and $response->total_results > 1 and !isset($cols['district']) and !isset($cols['county'])) {
        foreach ($response->results as $item) {
          if ($item->components->_type == 'state') {
            $options['locations'][$location] = $item->geometry;
            break;
          }
        }
      }
    }
    $lat = $options['locations'][$location]->lat ?? NULL;
    $lng = $options['locations'][$location]->lng ?? NULL;
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

    if (!$coord) {
      $records['stockprop']["$tree_id-location"] = array(
        'type_id' => $cvterm['loc'],
        'value' => $location,
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );

      $tree_info[$tree_id]['location'] = $location;

      if (isset($geo_api_key)) {
        $result = $options['locations'][$location] ?? NULL;
        if (empty($result)) {
          $query = urlencode($location);
          $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
          $response = json_decode(file_get_contents($url));
          $result = ($response->total_results) ? $response->results[0]->geometry : NULL;
          $options['locations'][$location] = $result;
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
    tpps_job_logger_write('[INFO] - Inserting data into database using insert_multi...');
    $job->logMessage('[INFO] - Inserting data into database using insert_multi...');
    $new_ids = tpps_chado_insert_multi($records, $multi_insert_options);
    tpps_job_logger_write('[INFO] - Done.');
    $job->logMessage('[INFO] - Done.');
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

/**
 * Lookup analysis_id from ref_genome string
 * The ref_genome string usually comes from TPPS form select
 */
function tpps_get_analysis_id_from_ref_genome($ref_genome) {
    // We need to find the analysis id from the ref genome
    $analysis_id = NULL;
    if (isset($ref_genome)) {
      // Get the species and version from the reference genome selected
      // if match occurs thats in index [0].
      // The group match index [1] is species, group match index [2] is version
      preg_match('/(.+) +v(\d*\.*\d*)/', $ref_genome, $matches);
      print_r($matches);
      print_r("\n");
      $ref_genome_species = NULL;
      $ref_genome_version = NULL;
      if (count($matches) > 0) {
        $ref_genome_species = $matches[1];
        $ref_genome_version = $matches[2];
      }
      echo "ref_genome_species: $ref_genome_species\n";
      echo "ref_genome_version: $ref_genome_version\n";

      if (isset($ref_genome_species) && isset($ref_genome_version)) {
        // Look up the analysis
        $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
          WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
          [
            ':name' => $ref_genome_species . '%',
            ':programversion' => $ref_genome_version
          ]
        );
        foreach ($analysis_results as $row) {
          print_r("analysis_row\n");
          print_r($row);
          print_r("\n");
          $analysis_id = $row->analysis_id;
        }
      }
      echo "analysis_id: $analysis_id\n";

      if($analysis_id == NULL) {
        // Look up the analysis
        $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
          WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
          [
            ':name' => $ref_genome_species . '%',
            ':programversion' => 'v' . $ref_genome_version
          ]
        );
        foreach ($analysis_results as $row) {
          print_r("analysis_row\n");
          print_r($row);
          print_r("\n");
          $analysis_id = $row->analysis_id;
        }
      }
      else {
        return $analysis_id;
      }


      // If an analysis_id still was not found, it's possibly from the db data source
      // instead of the genome directory. The genome directory code is in page_4_helper.php
      // New code to cater for new analysis checks via db - query given by Emily Grau (6/6/2023)
      if ($analysis_id == NULL) {
        $genome_query_results = chado_query("select * from chado.tpps_ref_genomes WHERE name LIKE :ref_genome;", [
          ':ref_genome' => $ref_genome
        ]);
        foreach ($genome_query_results as $genome_query_row) {
          print_r("genome_query_row\n");
          print_r($genome_query_row);
          print_r("\n");
          $analysis_id = $genome_query_row->analysis_id;
        }
      }
      else {
        return $analysis_id;
      }
      // $options['analysis_id'] = $analysis_id;
      echo "analysis_id: $analysis_id\n";
      // Once an analysis_id was found, try to get srcfeature_id

    }
    else {
      return NULL;
      echo "A reference genome could not be found in the TPPS page 4 form.\n";
      echo "Without this, we cannot find the analysis_id and thus the srcfeature_id.\n";
      echo "Featureloc data will not be recorded\n";
    }

}

/**
 * Generate all possible 4-letter TreeGenes organism codes.
 *
 * @param string $genus
 *   The genus of the organism.
 * @param string $species
 *   The species of the organism.
 *
 * @return Generator|array
 *   Yields each possible organism code in the desired order.
 */
function tpps_get_species_codes($genus, $species) {
  $codes = array();

  foreach (tpps_get_code_parts($genus) as $genus_part) {
    foreach (tpps_get_code_parts($species) as $species_part) {
      $code = ucfirst($genus_part . $species_part);
      if (!array_key_exists($code, $codes)) {
        yield $code;
        $codes[$code] = TRUE;
      }
    }
  }
}

/**
 * Helper function to determine if a VCF exists
 * in the form state
 * 
 * @param mixed $form_state
 * @param int $i
 *  The organism number example 1,2,3 etc.
 */
function tpps_vcf_exists($form_state, $i) {
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  if (!isset($genotype)) {
    return false;
  }
  if ($genotype['files']['file-type'] == 'VCF') {
    $vcf_fid = $genotype['files']['vcf'];
    // This means it was uploaded
    if ($vcf_fid > 0) {
      $vcf_file = file_load($vcf_fid);
      $location = tpps_get_location($vcf_file->uri);
    }
    else {
      $location = $genotype['files']['local_vcf'];
    }
    if (isset($location)) {
      return true;
    }
    else {
      return false;
    }
  }
}




function tpps_snps_assay_location($form_state, $i) {
  $results = [
    'status' => 'empty', // empty, exists, missing
    'location' => NULL,
    'fid' => 0,
  ];  
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  $snp_fid = $genotype['files']['snps-assay'];
  $results['fid'] = $snp_fid;
  if ($snp_fid > 0) {
    $results['status'] = 'exists';
    $snp_file = file_load($snp_fid);
    $location = tpps_get_location($snp_file->uri);
    $results['location'] = $location;
    if ($location == '' or $location == null) {
      $results['status'] = 'missing';
    }
  }
  return $results; 
}

/**
 * This helper function will do some checks on the data to determine
 * if everything looks good before allow the genotype processing 
 * to run. It's intent is to error out when there are errors before 
 * a long genotype load happens and then fails (wasting the teams time)
 */
function tpps_genotype_initial_checks($form_state, $i, $job) {
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  if (!isset($genotype)) {
    $str = "[INITIAL CHECK] Genotype data could not be found for this study.";
    tpps_job_logger_write($str);
    $job->logMessage($str);
    return;
  }

  // Look up location information for SNP Assay and VCF files
  $vcf_location = tpps_vcf_location($form_state, $i); // returns array['status','location'];
  $snps_assay_location = tpps_snps_assay_location($form_state, $i); // returns array['status', 'location'];


  // SNP Markers assessment
  $snps_assay_markers = NULL;
  if ($snps_assay_location['status'] == 'exists') {
    $memstart = memory_get_usage();
    $snps_assay_markers = tpps_genotype_get_snps_assay_markers($snps_assay_location['fid']);
    $snps_assay_markers['memusage'] = memory_get_usage() - $memstart;
    $str = "[INITIAL CHECK] SNPs Assay Markers (unique count): " . $snps_assay_markers['unique_count'];
    tpps_job_logger_write($str);
    $job->logMessage($str);
    $str = "[INITIAL CHECK] SNPs Assay Markers (memusage): " . $snps_assay_markers['memusage'];
    tpps_job_logger_write($str);
    $job->logMessage($str);    
  }

  $vcf_markers = NULL;
  print_r($vcf_location);
  if ($vcf_location['status'] == 'exists') {
    $memstart = memory_get_usage();
    $vcf_markers = tpps_genotype_get_vcf_markers($vcf_location['location']);
    $vcf_markers['memusage'] = memory_get_usage() - $memstart;
    $str = "[INITIAL CHECK] VCF Markers (unique count): " . $vcf_markers['unique_count'];
    tpps_job_logger_write($str);
    $job->logMessage($str);
    $str = "[INITIAL CHECK] VCF Markers (memusage): " . $vcf_markers['memusage'];
    tpps_job_logger_write($str);
    $job->logMessage($str);    
  }



  // If both a VCF file and a SNPS assay file exists
  if($vcf_location['status'] == 'exists' and $snps_assay_location['status'] == 'exists') {
    // [RISH] 7/18/2023 - TODO - check if they match or not, functions already built 
    // (check previous lines) where the arrays already exist - just need to run the 
    // array_diff function



  }
  // throw new Exception('DEBUG');
}

// TODO
function tpps_accession_file_get_tree_ids($form_state, $i) {
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];
  $fid = $thirdpage['tree-accession']['species-' . $i]['file'];
  $snp_assay_header = tpps_file_headers($fid);
}

function tpps_accession_file_location($form_state, $i) {
  $results = [
    'status' => 'empty', // empty, exists, missing
    'location' => NULL,
    'fid' => 0,
  ];  
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];
  $fid = $thirdpage['tree-accession']['species-' . $i]['file'];
  $results['fid'] = $fid;
  if ($fid > 0) {
    $results['status'] = 'exists';
    $file = file_load($fid);
    $location = tpps_get_location($file->uri);
    $results['location'] = $location;
    if ($location == '' or $location == null) {
      $results['status'] = 'missing';
    }
  }
  return $results; 
}

/**
 * Helper function to determine VCF location
 * from the form state
 * 
 * @param mixed $form_state
 * @param int $i
 *  The organism number example 1,2,3 etc.
 */
function tpps_vcf_location($form_state, $i) {
  $results = [
    'status' => 'empty', // empty, exists, missing
    'location' => NULL,
  ];
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  if (!isset($genotype)) {
    return false;
  }
  if ($genotype['files']['file-type'] == 'VCF') {
    $vcf_fid = $genotype['files']['vcf'];
    // This means it was uploaded
    if ($vcf_fid > 0) {
      $results['status'] = 'exists';
      $vcf_file = file_load($vcf_fid);
      $location = tpps_get_location($vcf_file->uri);
      $results['location'] = $location;
      if ($location == '' or $location == null) {
        $results['status'] = 'missing';
      }
    }
    else {
      $results['status'] = 'exists';
      $location = $genotype['files']['local_vcf'];
      $results['location'] = $location;
      if (!file_exists($location)) {
        $results['status'] = 'missing';
      }
    }
  }
  return $results;
}

/**
 * Helper function to return the VCF markers (called variants)
 * from a file specified by location.
 * @param string location (File location)
 * 
 * @return array values,unique_count,count
 */
function tpps_genotype_get_vcf_tree_ids($location) {
  $vcf_content = gzopen($location, 'r');
  $count = 0;
  $tree_ids = []; // tree_ids
  $duplicate_tree_ids = [];
  while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
    if (stripos($vcf_line,'#CHROM') !== FALSE && $vcf_line != "" && str_replace("\0", "", $vcf_line) != "") {
      $vcf_line = explode("\t", $vcf_line);
      $vcf_line_parts_count = count($vcf_line);
      for($i = 9; $i < $vcf_line_parts_count; $i++) {
        $tree_id = trim($vcf_line[$i]);
        if ($tree_id != "") {
          if (isset($tree_ids[$tree_id])) {
            $duplicate_tree_ids[$tree_id] = 1;
          }
          else {
            $tree_ids[$tree_id] = 1; // arbitrary value of 1 just save as an array
          }
          $count++;

        }
      }
    }
  }
  ksort($tree_ids);
  ksort($duplicate_tree_ids);
  $result_arr = [
    'values' => array_keys($tree_ids),
    'duplicate_values' => array_keys($duplicate_tree_ids),
    'unique_count' => count(array_keys($tree_ids)),
    'count' => $count
  ];
  return $result_arr;

}


/**
 * Helper function to return the VCF markers (called variants)
 * from a file specified by location.
 * @param string location (File location)
 * 
 * @return array values,unique_count,count
 */
function tpps_genotype_get_vcf_markers($location) {
  $vcf_content = gzopen($location, 'r');
  $variants = []; // markers basically
  $duplicate_variants = []; // record duplicates
  $count = 0;
  while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
    if ($vcf_line[0] != '#' && stripos($vcf_line,'.vcf') === FALSE && trim($vcf_line) != "" && str_replace("\0", "", $vcf_line) != "") {
      $vcf_line = explode("\t", $vcf_line);
      $variant_name = &$vcf_line[2];
      if (isset($variants[$variant_name])) {
        $duplicate_variants[$variant_name] = 1;
      }
      else {
        $variants[$variant_name] = 1; // arbitrary value of 1 just save as an array
      }
      $count++;
    }
  }
  ksort($variants);
  ksort($duplicate_variants);
  $result_arr = [
    'values' => array_keys($variants),
    'duplicate_values' => array_keys($duplicate_variants),
    'unique_count' => count(array_keys($variants)),
    'count' => $count
  ];
  return $result_arr;

}

/**
 * Helper function to return the snps assay markers
 * from a file specified by the FILE ID.
 * @param int FID (File ID)
 * 
 * @return array values,unique_count,count
 */
function tpps_genotype_get_snps_assay_markers($fid) {
  $snp_assay_header = tpps_file_headers($fid);
  // Ignore the first column which contains the tree_id
  $results = [];
  $duplicates = [];
  $count = 0;
  foreach ($snp_assay_header as $column_letters => $column_value) {
    $column_value = trim($column_value);
    if ($count == 0) {
      // ignore (this is the first column which is the tree id header)
    }
    else {
      if (isset($results[$column_value])) {
        // duplicate detected
        $duplicates[$column_value] = 1;
      }
      else {
        $results[$column_value] = 1; // 1 is just an arbitrary value
      }
      
    }
    $count++;
  }
  if ($count > 0) {
    $count--; // since we needed to ignore the first line which is a header
  } 
  ksort($results);
  ksort($duplicates);
  $result_arr = [
    'values' => array_keys($results),
    'duplicate_values' => array_keys($duplicates),
    'unique_count' => count(array_keys($results)),
    'count' => $count
  ];
  return $result_arr;
}

/**
 * Helper function for tpps_get_species_codes().
 *
 * Generate all possible 2-letter organism code parts.
 *
 * @param string $part
 *   The part of the organism name, either genus or species.
 *
 * @return Generator|array
 *   Yields each possible code part in the desired order.
 */
function tpps_get_code_parts($part) {
  for ($char1 = 0; $char1 <= strlen($part) - 2; $char1++) {
    for ($char2 = $char1 + 1; $char2 <= strlen($part) - 1; $char2++) {
      // Code parts should not repeat letters.
      if ($part[$char1] == $part[$char2]) {
        continue;
      }

      yield strtolower($part[$char1] . $part[$char2]);
    }
  }
}
