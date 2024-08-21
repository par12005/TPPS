<?php

/**
 * @file
 * Defines function tpps_submit_all and its helper functions.
 *
 * The functions defined in this file do not actually submit the genotype,
 * phenotype, or environmental data collected from page 4. That data is instead
 * submitted through a Tripal job due to the size of the data.
 */

// Global variables.
$tpps_job_logger = NULL;
$tpps_job = NULL;


/**
 * Initialized the job logger which handles writing to job logs
 * and also outputting Tripal Job log messages at the same time.
 * RISH 8/20/2024 - Code moved to this new function for re-use.
 *
 * @param string $accession
 *   The accession number of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_initialize_job_logger($accession, TripalJob $job = NULL) {
  global $tpps_job;
  global $tpps_job_logger;
  // Get public path.
  $log_path = tpps_realpath('public://') . '/tpps_job_logs/';

  tpps_log('[INFO] Initializing log path: ' . $log_path);

  if (!is_dir($log_path)) {
    mkdir($log_path);
  }

  // Update the global $tpps_job_logger variable.
  $tpps_job_logger = [];
  $tpps_job_logger['job_object'] = $job;
  $tpps_job_logger['log_file_path'] = $log_path . $accession . '_'
    . $tpps_job_logger['job_object']->getJobID() . '.txt';
  $tpps_job_logger['log_file_handle'] = fopen($tpps_job_logger['log_file_path'], "w+");
}

/**
 * Creates a record for the project and calls the submission helper functions.
 *
 * @param string $accession
 *   The accession number of the form being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_all($accession, TripalJob $job = NULL) {
  error_reporting(E_ALL);
  global $tpps_job;
  global $tpps_job_logger;

  // Get Submission Shared State (3S).
  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    // This could happen when study was removed but someone clicks 'Re-run'
    // Tripal job at page /admin/tripal/tripal_jobs'.
    $message = t("Submission @accession doesn't exist.",
      ['@accession' => $accession]);
    throw new Exception($message);
  }

  $tpps_job = $job;
  tpps_initialize_job_logger($accession, $job);



  tpps_log('[INFO] Setting up...');
  $job->setInterval(1);

  // PATCH to check if VCF exists, then remove the assay design.
  // Advised by Meghan 7/6/2023.

  // Do check for missing files, this is a bit crude but it works.
  $display_results = strip_tags(tpps_table_display($submission->sharedState));
  // Find 'missing file' string.
  echo $display_results;
  if (stripos($display_results, 'file missing') !== FALSE) {
    $display_results_lines = explode("\n", $display_results);
    foreach ($display_results_lines as $line) {
      if (stripos($line, 'file missing') !== FALSE) {
        tpps_log('[FATAL] ' . $line . "\n");
      }
    }
    $message = t('Detected a missing file, please ensure missing files '
      . 'are resolved in the tpps-admin-panel and then rerun the study');
    throw new Exception($message);
  }

  // Update 'updated' field with current time and 'status' field.
  $submission->save(TPPS_SUBMISSION_STATUS_SUBMISSION_JOB_RUNNING);
  $transaction = db_transaction();
  try {

    // RISH 7/18/2023 - Run some checks before going through most of the genotype
    // processing (this will error out if an issue is found to avoid long failing loads)
    $page1_values = &$submission->sharedState['saved_values'][TPPS_PAGE_1];
    $page4_values = &$submission->sharedState['saved_values'][TPPS_PAGE_4];
    $organism_number = $page1_values['organism']['number'];
    for ($i = 1; $i <= $organism_number; $i++) {
      tpps_genotype_initial_checks($submission->sharedState, $i, $job);
    }

    tpps_log('[INFO] Clearing any previous data for this study from the database...');
    tpps_submission_clear_db($accession);
    tpps_log('[INFO] Database cleared');


    tpps_submission_clear_default_tags($accession);
    $submission->sharedState['file_rank'] = 0;
    $submission->sharedState['ids'] = [];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Create project and get project_id.
    tpps_log('[INFO] Creating project record...');
    $project_id = $submission->sharedState['ids']['project_id'] ?? NULL;
    // When 'publication status' is 'In press' we have no 'title'
    // and no 'abstract' yet so default values must be used.
    $publication_title = $page1_values['publication']['title'] ?? 'No title';
    $publication_abstract = $page1_values['publication']['abstract'] ?? 'No abstract';
    $submission->sharedState['title'] = $publication_title;
    $submission->sharedState['abstract'] = $publication_abstract;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Process 'project_id'.
    if (!empty($project_id)) {
      // Reuse existing.
      // @TODO Should we check if record really exists in db table?
      $submission->sharedState['ids']['project_id']
        = $submission->state['ids']['project_id']
          = $project_id;
    }
    else {
      $project_record = [
        'name' => $publication_title,
        'description' => $publication_abstract,
      ];
      // Check if project with the same 'name' exists.
      if (count(chado_select_record('project', ['name'], $project_record)) > 0) {
        // Record exists and we need to make 'name' unique.
        $project_record['name'] .= ' (' . $accession . ')';
        $record = chado_select_record('project', ['project_id'], $project_record);
        if (!empty($record[0]->project_id)) {
          $submission->sharedState['ids']['project_id']
            = $submission->state['ids']['project_id']
              = $record[0]->project_id;
        }
        else {
          $submission->sharedState['ids']['project_id']
            = $submission->state['ids']['project_id']
              = chado_insert_record('project', $project_record)['project_id'];
        }
      }
      else {
        // Note: tpps_chado_insert_record() returns 'project_id' not whole record.
        $submission->sharedState['ids']['project_id']
          = $submission->state['ids']['project_id']
          = tpps_chado_insert_record('project', $project_record);
      }
    }
    tpps_log(
      '[INFO] Project record created. project_id: @pid.' . PHP_EOL,
      ['@pid' => $submission->sharedState['ids']['project_id']]
    );
    tpps_tripal_entity_publish('Project',
      [
        $submission->sharedState['title'],
        $submission->sharedState['ids']['project_id'],
      ]
    );
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    tpps_log("[INFO] Submitting Publication/Species information...");
    tpps_submit_page_1($submission->sharedState, $job);
    tpps_log("[INFO] Publication/Species information submitted!\n");

    tpps_log("[INFO] Submitting Study Details...");
    tpps_submit_page_2($submission->sharedState, $job);
    tpps_log("[INFO] Study Details sumbitted!\n");

    tpps_log("[INFO] Submitting Accession information...");
    tpps_submit_page_3($submission->sharedState, $job);
    tpps_log("[INFO] Accession information submitted!\n");

    tpps_log("[INFO] Submitting Genotype (Raw) data...");
    tpps_submit_page_4($submission->sharedState, $job);
    tpps_log("[INFO] Genotype (Raw) data submitted!\n");

    tpps_log("[INFO] Submitting Summary information...");
    tpps_submit_summary($submission->sharedState);
    tpps_log("[INFO] Summary information submitted!\n");

    tpps_log("[INFO] Renaming files...");
    tpps_submission_rename_files($accession);
    tpps_log("[INFO] Files renamed!\n");

    tpps_log("[INFO] Finishing up...");
    // Functions starting from tpps_submit_page_1() update $shared_state array
    // with new data so now we are going to update db record.
    $submission->sharedState['loaded'] = time();
    $submission->save(TPPS_SUBMISSION_STATUS_APPROVED);
    tpps_log("[INFO] Complete!");

    fclose($tpps_job_logger['log_file_handle']);

  }
  catch (Exception $e) {
    $transaction->rollback();
    // Restore status of study because processing failed.
    $submission = new Submission($accession);
    $submission->save(TPPS_SUBMISSION_STATUS_PENDING_APPROVAL);

    tpps_log('[ERROR] Job failed', [], TRIPAL_ERROR);
    tpps_log('[ERROR] Error message: @msg', ['@msg' => $e->getMessage()], TRIPAL_ERROR);
    tpps_log("[ERROR] Trace: \n!trace", ['!trace' => $e->getTraceAsString()], TRIPAL_ERROR);

    fclose($tpps_job_logger['log_file_handle']);
    watchdog_exception('tpps', $e);
    throw new Exception('Job failed.');
  }
}

/**
 * Writes data to the tpps_job_logger_handle.
 *
 * @param string $string
 *   Write string to the job log file using the tpps_job_logger object.
 * @param mixed $replacements
 *   List of tokens to be replaced.
 */
function tpps_job_logger_write($string, $replacements = []) {
  global $tpps_job_logger;
  try {
    foreach ($replacements as $key_string => $replace_string) {
      $string = str_replace($key_string, $replace_string, $string);
    }

    // Add timestamp.
    $time_now = time();
    $timestamp_now = date('m/d/y g:i:s A', $time_now);

    $string = "\n" . $timestamp_now . " " . $string;

    @fwrite($tpps_job_logger['log_file_handle'], $string);
    @fflush($tpps_job_logger['log_file_handle']);
  }
  catch (Exception $e) {
    print_r($e->getMessage());
  }
}

/**
 * Submits Publication and Species data to the database.
 *
 * @param array $shared_state
 *   Submission Shared State. See class Submission.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_page_1(array &$shared_state, TripalJob &$job = NULL) {

  $dbxref_id = $shared_state['dbxref_id'];
  $page1_values = $shared_state['saved_values'][TPPS_PAGE_1];
  $page3_values = $shared_state['saved_values'][TPPS_PAGE_3];

  $primaryAuthor = check_plain($page1_values['primaryAuthor']);
  $seconds = $page1_values['publication']['secondaryAuthors'];

  //tpps_log(print_r($shared_state, 1));

  tpps_chado_insert_record('project_dbxref', array(
    'project_id' => $shared_state['ids']['project_id'],
    'dbxref_id' => $dbxref_id,
    'is_current' => TRUE,
  ));

  // Save DOI fields.
  if (($shared_state['tpps_type'] ?? NULL) == 'tppsc') {
    // Required 'Publication DOI' Field.
    if (!empty($page1_values['doi'])) {
      $dryad_db = chado_get_db(['name' => 'dryad']);
      $dryad_dbxref = chado_insert_dbxref([
        'db_id' => $dryad_db->db_id,
        'accession' => $page1_values['doi'],
      ])->dbxref_id;
      tpps_chado_insert_record('project_dbxref', [
        'project_id' => $shared_state['ids']['project_id'],
        'dbxref_id' => $dryad_dbxref,
        'is_current' => TRUE,
      ]);
    }
    // Optional 'Dataset DOI' Field.
    if (!empty($page1_values['dataset_doi'])) {
      $dryad_db = chado_get_db(['name' => 'dryad']);
      $dryad_dbxref = chado_insert_dbxref([
        'db_id' => $dryad_db->db_id,
        'accession' => $page1_values['dataset_doi'],
      ])->dbxref_id;
      tpps_chado_insert_record('project_dbxref', [
        'project_id' => $shared_state['ids']['project_id'],
        'dbxref_id' => $dryad_dbxref,
        'is_current' => TRUE,
      ]);
    }
  }

  if (!empty($page1_values['photo'])) {
    tpps_add_project_file($shared_state, $page1_values['photo']);
  }

  $primary_author_id = tpps_chado_insert_record('contact', array(
    'name' => $primaryAuthor,
    'type_id' => tpps_load_cvterm('person')->cvterm_id,
  ));

  tpps_chado_insert_record('project_contact', array(
    'project_id' => $shared_state['ids']['project_id'],
    'contact_id' => $primary_author_id,
  ));

  $authors = [$primaryAuthor];
  if ($seconds['number'] != 0) {
    for ($i = 1; $i <= $seconds['number']; $i++) {
      if (!empty($seconds[$i]) || $seconds[$i] != "") {
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
        tpps_log('[INFO] - Secondary publishers error - found an empty secondary publisher name. Ignoring this input.');
        // throw new Exception("Seconds[$i]" . $seconds[$i]);
      }
    }
  }

  $publication_title = $page1_values['publication']['title'] ?? 'No title';
  $publication_journal = $page1_values['publication']['journal'] ?? 'No journal';
  $publication_year = $page1_values['publication']['year'] ?? NULL;
  $publication_id = tpps_chado_insert_record('pub', [
    'title' => $publication_title,
    'series_name' => $publication_journal,
    'type_id' => tpps_load_cvterm('article')->cvterm_id,
    'pyear' => $publication_year,
    'uniquename' => implode('; ', $authors)
      . " $publication_title. $publication_journal; $publication_year",
  ]);
  $shared_state['ids']['pub_id'] = $publication_id;
  tpps_tripal_entity_publish('Publication', [$publication_title, $publication_id]);

  // @TODO Check why this data dupliated to the 1st level of $shared_state.
  $shared_state['pyear'] = $page1_values['publication']['year'];
  $shared_state['journal'] = $page1_values['publication']['journal'];

  if (!empty($page1_values['publication']['abstract'])) {
    tpps_chado_insert_record('pubprop', array(
      'pub_id' => $publication_id,
      'type_id' => tpps_load_cvterm('abstract')->cvterm_id,
      'value' => $page1_values['publication']['abstract'],
    ));
  }

  tpps_chado_insert_record('pubprop', array(
    'pub_id' => $publication_id,
    'type_id' => tpps_load_cvterm('authors')->cvterm_id,
    'value' => implode(', ', $authors),
  ));
  $shared_state['authors'] = $authors;

  tpps_chado_insert_record('project_pub', array(
    'project_id' => $shared_state['ids']['project_id'],
    'pub_id' => $publication_id,
  ));

  $names = explode(" ", $primaryAuthor);
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

  $shared_state['ids']['organism_ids'] = [];
  $organism_number = $page1_values['organism']['number'];

  for ($i = 1; $i <= $organism_number; $i++) {
    $raw_name = trim($page1_values['organism'][$i]['name']);
    $parts = explode(" ", $raw_name);
    $genus = trim($parts[0]);
    $species = trim(implode(" ", array_slice($parts, 1)));
    if ($genus == '' || $genus == NULL) {
      throw new Exception("Genus is empty - this isn't good so we're terminating the job.");
    }
    if ($species == '' || $species == NULL) {
      throw new Exception("Species is empty - this isn't good so we're terminating the job.");
    }
    $infra = NULL;
    $parts_count = count($parts);
    $organism_type_id = NULL;
    if (isset($parts[2]) and (
        $parts[2] == 'subsp.' or 
        $parts[2] == 'spp.' or 
        $parts[2] == 'sp.' or 
        $parts[2] == 'subspecies' or 
        $parts[2] == 'var.' or 
        $parts[2] == 'varieta' or 
        $parts[2] == 'variety' or 
        $parts[2] == 'subvar.' or 
        $parts[2] == 'subvarieta' or 
        $parts[2] == 'subvariety' or 
        $parts[2] == 'f.' or 
        $parts[2] == 'forma' or 
        $parts[2] == 'form'
      )
    ) {
      $infra = implode(" ", array_slice($parts, 2));
      $species = $parts[1]; // Based on Emily's suggestion 7/25/2024
    }
    else if (isset($parts[2]) and $parts_count <= 3) {
      // cater for examples like Taxus baccata L or Taxus baccata L.
      // where we want to remove the L or L.

      // Set infra to NULL
      $infra = NULL;
      // Set the species to the second part which is in $parts[1];
      $species = $parts[1];
    }
    else if (isset($parts[2]) and $parts_count > 3) {
      // lookup type_id
      if ($parts[2] == 'x') {
        $results_organism_type_id_results = chado_query('SELECT * FROM chado.cvterm WHERE name = :name', [
          ':name' => 'speciesaggregate'
        ]);
        foreach ($results_organism_type_id_results as $organism_type_id_row) {
          $organism_type_id = $organism_type_id_row->type_id;
        }
      }
      else if ($parts[2] == 'subsp.' or $parts[2] == 'spp.' or $parts[2] == 'sp.' or $parts[2] == 'subspecies') {
        $results_organism_type_id_results = chado_query('SELECT * FROM chado.cvterm WHERE name = :name', [
          ':name' => 'subspecies'
        ]);
        foreach ($results_organism_type_id_results as $organism_type_id_row) {
          $organism_type_id = $organism_type_id_row->type_id;
        }
      }
      else if ($parts[2] == 'var.' or $parts[2] == 'varieta' or $parts[2] == 'variety') {
        $results_organism_type_id_results = chado_query('SELECT * FROM chado.cvterm WHERE name = :name', [
          ':name' => 'varietas'
        ]);
        foreach ($results_organism_type_id_results as $organism_type_id_row) {
          $organism_type_id = $organism_type_id_row->type_id;
        }
      } 
      else if ($parts[2] == 'f.' or $parts[2] == 'forma' or $parts[2] == 'form') {
        $results_organism_type_id_results = chado_query('SELECT * FROM chado.cvterm WHERE name = :name', [
          ':name' => 'forma'
        ]);
        foreach ($results_organism_type_id_results as $organism_type_id_row) {
          $organism_type_id = $organism_type_id_row->type_id;
        }
      }    
    }

    $record = [
      'genus' => $genus,
      'species' => $species,
      'infraspecific_name' => $infra,
      'type_id' => $organism_type_id
    ];

    echo "This is the record data to check for OR ELSE insert this data into the db\n";
    print_r($record);

    if (preg_match('/ x /', $species)) {
      $record['type_id'] = tpps_load_cvterm('speciesaggregate')->cvterm_id;
    }

    echo "Checking to see if records exist for genus $genus, species $species, infr $infra\n";
    // Let's check to see if genus and species match, if so, get the id
    // if it does not return any rows, then create organism
    $organism_results = chado_query('SELECT * FROM chado.organism WHERE genus = :genus AND species = :species
      AND infraspecific_name = :infra', [
      ':genus' => $genus,
      ':species' => $species,
      ':infra' => $infra
    ]);
    $organism_results_id = -1;
    // Check if an id exists in the database for this organism and remember it in ($organism_results_id)
    foreach ($organism_results as $organism_row) {
      $organism_results_id = $organism_row->organism_id;
    }
    echo "Found organism results id: " . $organism_results_id . "\n";
    // throw new Exception('DEBUG');


    // If no organism id was found in database, perform an insert

    // TEST CODE @TODO, ADD THIS TO WHEN $organism_results_id == -1
    if ($infra != "" and $infra != NULL and $organism_results_id == -1) {
      // Lookup to see if this species exists on NCBI
      $taxons = tpps_ncbi_get_taxon_id($raw_name, TRUE);
      // print_r($taxons);
      $taxons = json_decode(json_encode($taxons))->Id;
      // print_r($taxons);

      if (empty($taxons) || count($taxons) === 0) {
        throw new Exception("This study contains a variation-type species in which we could not find a matching record on NCBI: " . $raw_name);
      }
    }


    if ($organism_results_id == -1) {
      $shared_state['ids']['organism_ids'][$i] = tpps_chado_insert_record('organism', $record);
    }
    // If organism id was found in database, use it
    else {
      $shared_state['ids']['organism_ids'][$i] = $organism_results_id;
    }

    if (!empty(tpps_load_cvterm('Type'))) {
      tpps_chado_insert_record('organismprop', [
        'organism_id' => $shared_state['ids']['organism_ids'][$i],
        'type_id' => tpps_load_cvterm('Type')->cvterm_id,
        'value' => $page1_values['organism'][$i]['is_tree'] ? 'Tree' : 'Non-tree',
      ]);
    }

    if ($organism_number != 1) {
      if (
        !empty($page3_values['tree-accession']['check'])
        && empty($page3_values['tree-accession']["species-$i"]['file'])
      ) {
        continue;
      }

      if (empty($page3_values['tree-accession']['check'])) {
        $options = [
          'cols' => [],
          'search' => $page1_values['organism'][$i]['name'],
          'found' => FALSE,
        ];
        $tree_accession = $page3_values['tree-accession']["species-1"];
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

    $code_exists = tpps_chado_prop_exists(
      'organism',
      $shared_state['ids']['organism_ids'][$i],
      'organism 4 letter code'
    );

    if (!$code_exists) {
      foreach (tpps_get_species_codes($genus, $species) as $trial_code) {
        $new_code_query = chado_select_record('organismprop', ['value'], [
          'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
          'value' => $trial_code,
        ]);

        if (empty($new_code_query)) {
          break;
        }
      }

      tpps_chado_insert_record('organismprop', array(
        'organism_id' => $shared_state['ids']['organism_ids'][$i],
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
      $exists = tpps_chado_prop_exists(
        'organism', $shared_state['ids']['organism_ids'][$i], $rank
      );
      if (!$exists) {
        $taxon = tpps_get_taxon($page1_values['organism'][$i]['name'], $rank);
        if ($taxon) {
          tpps_chado_insert_record('organismprop', array(
            'organism_id' => $shared_state['ids']['organism_ids'][$i],
            'type_id' => tpps_load_cvterm($rank)->cvterm_id,
            'value' => $taxon,
          ));
        }
      }
    }

    tpps_chado_insert_record('project_organism', array(
      'organism_id' => $shared_state['ids']['organism_ids'][$i],
      'project_id' => $shared_state['ids']['project_id'],
    ));

    tpps_chado_insert_record('pub_organism', array(
      'organism_id' => $shared_state['ids']['organism_ids'][$i],
      'pub_id' => $publication_id,
    ));

    tpps_tripal_entity_publish('Organism', array(
      "$genus $species",
      $shared_state['ids']['organism_ids'][$i],
    ));
  }
}

/**
 * Submits Study Design data to the database.
 *
 * @param array $shared_state
 *   Submission Shared State. See class Submission.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_page_2(array &$shared_state, TripalJob &$job = NULL) {

  $page2_values = $shared_state['saved_values'][TPPS_PAGE_2];

  if (!empty($page2_values['StartingDate'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $shared_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('study_start')->cvterm_id,
      'value' => $page2_values['StartingDate']['month'] . " " . $page2_values['StartingDate']['year'],
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $shared_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('study_end')->cvterm_id,
      'value' => $page2_values['EndingDate']['month'] . " " . $page2_values['EndingDate']['year'],
    ));
  }

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $shared_state['ids']['project_id'],
    'type_id' => tpps_load_cvterm('association_results_type')->cvterm_id,
    'value' => $page2_values['data_type'],
  ));

  module_load_include('inc', 'tpps', 'includes/form');
  tpps_chado_insert_record('projectprop', [
    'project_id' => $shared_state['ids']['project_id'],
    'type_id' => tpps_load_cvterm('study_type')->cvterm_id,
    'value' => tpps_form_get_study_type($page2_values['study_type']),
  ]);

  if (!empty($page2_values['study_info']['season'])) {
    $seasons = implode($page2_values['study_info']['season']);

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $shared_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('assession_season')->cvterm_id,
      'value' => $seasons,
    ));
  }

  if (!empty($page2_values['study_info']['assessions'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $shared_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('assession_number')->cvterm_id,
      'value' => $page2_values['study_info']['assessions'],
    ));
  }

  if (!empty($page2_values['study_info']['temp'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $shared_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('temperature_high')->cvterm_id,
      'value' => $page2_values['study_info']['temp']['high'],
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $shared_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('temperature_low')->cvterm_id,
      'value' => $page2_values['study_info']['temp']['low'],
    ));
  }

  $types = array(
    'co2',
    'humidity',
    'light',
    'salinity',
  );

  foreach ($types as $type) {
    if (!empty($page2_values['study_info'][$type])) {
      $set = $page2_values['study_info'][$type];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $shared_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm("{$type}_control")->cvterm_id,
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $shared_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm("{$type}_level")->cvterm_id,
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $shared_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm("{$type}_level")->cvterm_id,
          'value' => $set['uncontrolled'],
        ));
      }
    }
  }

  if (!empty($page2_values['study_info']['rooting'])) {
    $root = $page2_values['study_info']['rooting'];

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $shared_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('rooting_type')->cvterm_id,
      'value' => $root['option'],
    ));

    if ($root['option'] == 'Soil') {
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $shared_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('soil_type')->cvterm_id,
        'value' => ($root['soil']['type'] == 'Other') ? $root['soil']['other'] : $root['soil']['type'],
      ));

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $shared_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('soil_container')->cvterm_id,
        'value' => $root['soil']['container'],
      ));
    }

    if (!empty($page2_values['study_info']['rooting']['ph'])) {
      $set = $page2_values['study_info']['rooting']['ph'];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $shared_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('pH_control')->cvterm_id,
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $shared_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('pH_level')->cvterm_id,
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $shared_state['ids']['project_id'],
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
          'project_id' => $shared_state['ids']['project_id'],
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
 * @param array $shared_state
 *   Submission Shared State. See class Submission.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_page_3(array &$shared_state, TripalJob &$job = NULL) {
  $page1_values = $shared_state['saved_values'][TPPS_PAGE_1];
  $page3_values = $shared_state['saved_values'][TPPS_PAGE_3];
  $organism_number = $page1_values['organism']['number'];
  $shared_state['locations'] = [];
  $shared_state['tree_info'] = [];
  $stock_count = 0;
  $loc_name = 'Location (latitude/longitude or country/state or population group)';

  if (!empty($page3_values['study_location'])) {
    $type = $page3_values['study_location']['type'];
    $locs = $page3_values['study_location']['locations'];
    $geo_api_key = variable_get('tpps_geocode_api_key', NULL);

    for ($i = 1; $i <= $locs['number']; $i++) {
      if ($type !== '2') {
        $standard_coordinate = explode(',', tpps_standard_coord($locs[$i]));
        $latitude = $standard_coordinate[0];
        $longitude = $standard_coordinate[1];

        tpps_chado_insert_record('projectprop', array(
          'project_id' => $shared_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('gps_latitude')->cvterm_id,
          'value' => $latitude,
          'rank' => $i,
        ));

        tpps_chado_insert_record('projectprop', array(
          'project_id' => $shared_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('gps_longitude')->cvterm_id,
          'value' => $longitude,
          'rank' => $i,
        ));
        continue;
      }
      $loc = $locs[$i];
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $shared_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('experiment_location')->cvterm_id,
        'value' => $loc,
        'rank' => $i,
      ));

      if (variable_get('tpps_submitall_skip_gps_request') && isset($geo_api_key)) {
        $query = urlencode($loc);
        $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
        $response = json_decode(file_get_contents($url));

        if ($response->total_results) {
          $result = $response->results[0]->geometry;
          $shared_state['locations'][$loc] = $result;

          tpps_chado_insert_record('projectprop', array(
            'project_id' => $shared_state['ids']['project_id'],
            'type_id' => tpps_load_cvterm('gps_latitude')->cvterm_id,
            'value' => $result->lat,
            'rank' => $i,
          ));

          tpps_chado_insert_record('projectprop', array(
            'project_id' => $shared_state['ids']['project_id'],
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
      'prefix' => $shared_state['accession'] . '-',
    ),
  );

  $names = array();
  for ($i = 1; $i <= $organism_number; $i++) {
    $names[$i] = $page1_values['organism'][$i]['name'];
  }
  $names['number'] = $page1_values['organism']['number'];
  $options = array(
    'cvterms' => $cvterms,
    'records' => $records,
    'overrides' => $overrides,
    'locations' => &$shared_state['locations'],
    'accession' => $shared_state['accession'],
    'single_file' => empty($page3_values['tree-accession']['check']),
    'org_names' => $names,
    'saved_ids' => &$shared_state['ids'],
    'stock_count' => &$stock_count,
    'multi_insert' => $multi_insert_options,
    'tree_info' => &$shared_state['tree_info'],
    'job' => &$job,
  );

  for ($i = 1; $i <= $organism_number; $i++) {
    $tree_accession = $page3_values['tree-accession']["species-$i"];
    $fid = $tree_accession['file'];

    tpps_add_project_file($shared_state, $fid);

    $column_vals = $tree_accession['file-columns'];
    $groups = $tree_accession['file-groups'];

    $options['org_num'] = $i;
    $options['no_header'] = !empty($tree_accession['file-no-header']);
    $options['empty'] = $tree_accession['file-empty'];
    $options['pop_group'] = $tree_accession['pop-group'];

    // [VS] #8669py308
    switch ($tree_accession['location_accuracy']) {
      case 'exact':
        $options['exact'] = TRUE;
        $options['precision'] = NULL;
        break;

      case 'approximate':
        $options['exact'] = NULL;
        $options['precision'] = $tree_accession['coord_precision'] ?? NULL;
        if (
          !empty($tag_id = tpps_get_tag_id('No Location Information'))
          && !array_key_exists($tag_id, tpps_submission_get_tags($shared_state['accession']))
        ) {
          tpps_submission_add_tag($shared_state['accession'], 'Approximate Coordinates');
        }
        break;

      case 'descriptive_place':
        // @TODO Major. Store value in database.
        $options['exact'] = NULL;
        $options['precision'] = $tree_accession['descriptive_place'] ?? NULL;
        break;
    }
    // [/VS] #8669py308

    $county = array_search(TPPS_COLUMN_COUNTY, $column_vals);
    $district = array_search(TPPS_COLUMN_DISTRICT, $column_vals);
    $clone = array_search(TPPS_COLUMN_CLONE_NUMBER, $column_vals);
    $options['column_ids'] = [
      'id' => $groups['Tree Id'][TPPS_COLUMN_PLANT_IDENTIFIER],
      'lat' => $groups[$loc_name][TPPS_COLUMN_LATITUDE] ?? NULL,
      'lng' => $groups[$loc_name][TPPS_COLUMN_LONGITUDE] ?? NULL,
      'country' => $groups[$loc_name][TPPS_COLUMN_COUNTRY] ?? NULL,
      'state' => $groups[$loc_name][TPPS_COLUMN_STATE] ?? NULL,
      'county' => ($county !== FALSE) ? $county : NULL,
      'district' => ($district !== FALSE) ? $district : NULL,
      'clone' => ($clone !== FALSE) ? $clone : NULL,
      'pop_group' => $groups[$loc_name][TPPS_COLUMN_POPULATION_GROUP] ?? NULL,
    ];

    if ($organism_number != 1 and empty($page3_values['tree-accession']['check'])) {
      if ($groups['Genus and Species']['#type'] == 'separate') {
        $options['column_ids']['genus'] = $groups['Genus and Species']['6'];
        $options['column_ids']['species'] = $groups['Genus and Species']['7'];
      }
      if ($groups['Genus and Species']['#type'] != 'separate') {
        $options['column_ids']['org'] = $groups['Genus and Species']['10'];
      }
    }
    tpps_log('[INFO] - Processing accession file data...');
    tpps_file_iterator($fid, 'tpps_process_accession', $options);
    tpps_log('[INFO] - Done.');

    tpps_log('[INFO] - Inserting data into database using insert_multi...');
    $new_ids = tpps_chado_insert_multi($options['records'], $multi_insert_options);
    tpps_log('[INFO] - Done.');
    foreach ($new_ids as $t_id => $stock_id) {
      $shared_state['tree_info'][$t_id]['stock_id'] = $stock_id;
    }
    unset($options['records']);
    $stock_count = 0;
    if (empty($page3_values['tree-accession']['check'])) {
      break;
    }
  }

  if (!empty($page3_values['existing_trees'])) {
    tpps_matching_trees($shared_state['ids']['project_id']);
  }
}

/**
 * Submits Tripal FASTAImporter job for reference genome.
 *
 * The remaining data for the fourth page is submitted during the TPPS File
 * Parsing Tripal Job due to its size.
 *
 * @param array $shared_state
 *   Submission Shared State. See class Submission.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_page_4(array &$shared_state, TripalJob &$job = NULL) {
  $page1_values = $shared_state['saved_values'][TPPS_PAGE_1];
  $page4_values = $shared_state['saved_values'][TPPS_PAGE_4];
  $organism_number = $page1_values['organism']['number'];
  // RISH 8/12/2024 - New function to generate species_codes array from shared_state
  $species_codes = tpps_generate_species_codes_array_from_shared_state($shared_state);


  for ($i = 1; $i <= $organism_number; $i++) {
    // Get species codes.
    // DEPRECATED 8/12/2024 due to breaking changes made by Vlad
    // $species_codes[$shared_state['ids']['organism_ids'][$i]] = current(
    //   chado_select_record(
    //     'organismprop',
    //     ['value'],
    //     [
    //       'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
    //       'organism_id' => $shared_state['ids']['organism_ids'][$i],
    //     ],
    //     [
    //       'limit' => 1,
    //     ]
    //   )
    // )->value;

    // Submit importer jobs.
    if (isset($page4_values["organism-$i"]['genotype'])) {
      $ref_genome = $page4_values["organism-$i"]['genotype']['ref-genome'];

      if ($ref_genome === 'url' or $ref_genome === 'manual' or $ref_genome === 'manual2') {
        // Create job for tripal fasta importer.
        $class = 'FASTAImporter';
        tripal_load_include_importer_class($class);

        $fasta = $page4_values["organism-$i"]['genotype']['tripal_fasta'];

        $file_upload = isset($fasta['file']['file_upload']) ? trim($fasta['file']['file_upload']) : 0;
        $file_existing = isset($fasta['file']['file_upload_existing']) ? trim($fasta['file']['file_upload_existing']) : 0;
        $file_remote = isset($fasta['file']['file_remote']) ? trim($fasta['file']['file_remote']) : 0;
        $analysis_id = $fasta['analysis_id'];
        $seqtype = $fasta['seqtype'];
        $organism_id = $shared_state['ids']['organism_ids'][$i];
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
          $importer->formSubmit($form, $shared_state);

          $importer->create($run_args, $file_details);

          $importer->submitJob();

        }
        catch (Exception $ex) {
          drupal_set_message(t('Cannot submit import: @msg', array('@msg' => $ex->getMessage())), 'error');
        }
      }
      elseif ($ref_genome === 'bio') {
        $eutils = $page4_values["organism-$i"]['genotype']['tripal_eutils'];
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

  $shared_state['data']['phenotype'] = array();
  $shared_state['data']['phenotype_meta'] = array();

  // Submit raw data.
  for ($i = 1; $i <= $organism_number; $i++) {
    tpps_submit_phenotype($shared_state, $i, $job);

    // Since $i is an organism order number and $species_codes are using
    // species id (cvterm id) they are not matches and this code assumes
    // that there is no code for species but $species_codes is not empty
    // and filled correctly.
    // Disable this message.
    // See task: https://app.clickup.com/t/86az7u2xr.
    //if (empty($species_codes[$i])) {
    //  // Not sure if it's a blocker for phenotype and environement.
    //  // Seems it's used for 'genotype' only.
    //  echo t("[WARNING] Species code for i = @count is empty.\n", ['@count' => $i]);
    //}
    //echo t("[DEBUG] Processing genotype data for species code '@code' and i = @count\n",
    //  ['@code' => $species_codes[$i], '@count' => $i]);

    tpps_submit_genotype($shared_state, $species_codes, $i, $job);
    tpps_submit_environment($shared_state, $i, $job);
  }
  // Generate genotype view.
  $test = FALSE;
  $project_id = $shared_state['ids']['project_id'];
  if (isset($project_id) && $test != TRUE) {
    tpps_generate_genotype_materialized_view($project_id);
  }
}

/**
 * Submits phenotype information for one species.
 *
 * @param array $shared_state
 *   Submission Shared State. See class Submission.
 * @param int $i
 *   The organism number we are submitting.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_phenotype(array &$shared_state, $i, TripalJob &$job = NULL) {
  // Debug Mode allows to call this function from browser for testing.
  $debug_mode = variable_get('tpps_submitall_phenotype_debug_mode', FALSE);
  tpps_log('[INFO] - Submitting phenotype data...');
  $page1_values = $shared_state['saved_values'][TPPS_PAGE_1];
  $page4_values = $shared_state['saved_values'][TPPS_PAGE_4];
  $phenotype = $page4_values["organism-$i"]['phenotype'] ?? NULL;
  $organism_name = $page1_values['organism'][$i]['name'];
  if (empty($phenotype)) {
    return;
  }
  tpps_submission_add_tag($shared_state['accession'], 'Phenotype');

  // Get appropriate cvterms.
  $phenotype_cvterms = array(
    'time' => tpps_load_cvterm('time')->cvterm_id,
    'desc' => tpps_load_cvterm('description')->cvterm_id,
    // @TODO [VS] Not sure this is needed since units are in separate table
    // in database and 'min/max not used for units.
    'unit' => tpps_load_cvterm('unit')->cvterm_id,
    'min' => tpps_load_cvterm('minimum')->cvterm_id,
    'max' => tpps_load_cvterm('maximum')->cvterm_id,
    // [/VS]
    'environment' => tpps_load_cvterm('environment')->cvterm_id,
    'intensity' => tpps_load_cvterm('intensity')->cvterm_id,
  );

  $records = [
    'phenotype' => [],
    'phenotypeprop' => [],
    'stock_phenotype' => [],
    'phenotype_cvterm' => [],
  ];
  $phenotype_count = 0;

  $options = [
    'records' => $records,
    'cvterms' => $phenotype_cvterms,
    'accession' => $shared_state['accession'],
    'tree_info' => $shared_state['tree_info'],
    'suffix' => 0,
    'phenotype_count' => $phenotype_count,
    'data' => &$shared_state['data']['phenotype'],
    'job' => &$job,
  ];

  $phenotype_number = $phenotype['phenotypes-meta']['number'];
  if (!empty($phenotype['normal-check'])) {
    $phenotypes_meta = [];
    $data_fid = $phenotype['file'];
    // Get all phenotype data provided by admin to override submitted data.
    if (!($debug_mode ?? NULL)) {
      tpps_add_project_file($shared_state, $data_fid);
    }
    $env_phenotypes = FALSE;
    // Populate $phenotypes_meta with manually entered metadata.
    for ($j = 1; $j <= $phenotype_number; $j++) {
      $name = strtolower($phenotype['phenotypes-meta'][$j]['name']);
      $phenotypes_meta[$name] = [];
      $phenotypes_meta[$name]['desc'] = $phenotype['phenotypes-meta'][$j]['description'];
      $phenotypes_meta[$name]['attr'] = $phenotype['phenotypes-meta'][$j]['attribute'];
      if ($phenotype['phenotypes-meta'][$j]['attribute'] == 'other') {
        $phenotypes_meta[$name]['attr-other'] = $phenotype['phenotypes-meta'][$j]['attr-other'];
      }
      // [VS] #8669rmrw5
      // Store casesensitive Phenotype name to use for synonym save.
      $phenotypes_meta[$name]['name'] = $phenotype['phenotypes-meta'][$j]['name'];
      $phenotypes_meta[$name]['synonym_id'] = $phenotype['phenotypes-meta'][$j]['synonym_id'];
      $phenotypes_meta[$name]['unit'] = $phenotype['phenotypes-meta'][$j]['unit'];
      if ($phenotype['phenotypes-meta'][$j]['unit'] == 'other') {
        $phenotypes_meta[$name]['unit-other'] = $phenotype['phenotypes-meta'][$j]['unit-other'];
      }
      // [/VS] #8669rmrw5
      $phenotypes_meta[$name]['struct'] = $phenotype['phenotypes-meta'][$j]['structure'];
      if ($phenotype['phenotypes-meta'][$j]['structure'] == 'other') {
        $phenotypes_meta[$name]['struct-other'] = $phenotype['phenotypes-meta'][$j]['struct-other'];
      }
      $phenotypes_meta[$name]['env'] = !empty($phenotype['phenotypes-meta'][$j]['env-check']);
      if ($phenotypes_meta[$name]['env']) {
        $env_phenotypes = TRUE;
      }
    }
    if ($env_phenotypes) {
      tpps_submission_add_tag($shared_state['accession'], 'Environment');
    }

    if ($phenotype['check'] == '1' || $phenotype['check'] == 'upload_file') {
      // @todo Check Phenotype Data file at validation stage. Check if it's
      // actually integer and not zero.
      $meta_fid = intval($phenotype['metadata']);
      // Added because TGDR009 META FID was 0 which caused failures
      if ($meta_fid > 0) {
        if (!($debug_mode ?? NULL)) {
          tpps_add_project_file($shared_state, $meta_fid);
        }
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
          'unit' => $groups['Unit']['4'],
          'struct' => !empty($struct) ? $struct : NULL,
          'min' => !empty($min) ? $min : NULL,
          'max' => !empty($max) ? $max : NULL,
        );
        // print_r($columns);

        $meta_options = array(
          'no_header' => $phenotype['metadata-no-header'],
          'meta_columns' => $columns,
          // [VS] $phenotypes_meta seems empty when metadata file used.
          // But later tpps_process_phenotype_meta() will fill 'meta' element
          // with data from phenotype metadata file. Keys will be phenotype
          // names from file in lowercase.
          'meta' => &$phenotypes_meta,
        );

        tpps_log('[INFO] - Processing phenotype_meta file data...');
        tpps_file_iterator($meta_fid, 'tpps_process_phenotype_meta', $meta_options);
        tpps_log('[INFO] - Done.');
      }
      else {
        tpps_job_logger_write('[WARNING] - phenotype_meta file id looks '
          . 'incorrect but the UI checkbox was selected. '
          . 'Need to double check this!');
      }
    }

    if (($debug_mode ?? NULL) || 0) {
      print_r("Phenotypes Meta:\n");
      print_r($phenotypes_meta);
      print_r("\n");
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
    // [VS]
    if ($phenotype['format'] == 0) {
      $file_headers = tpps_file_headers($data_fid, $phenotype['file-no-header']);
      $data_columns = [];
      if (
        is_array($groups['Phenotype Data']['0'])
        && !empty($groups['Phenotype Data']['0'])
      ) {
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
    tpps_log('[INFO] - Processing phenotype_data file data...');
    tpps_file_iterator($data_fid, 'tpps_process_phenotype_data', $options);
    $shared_state['data']['phenotype_meta'] += $phenotypes_meta;
    tpps_log('[INFO] - Inserting data into database using insert_multi...');

    $id_list = tpps_chado_insert_multi($options['records'], ['fks' => 'phenotype']);
    tpps_log('[INFO] - Done.');
  }

  if (!empty($phenotype['iso-check'])) {
    $iso_fid = $phenotype['iso'];
    tpps_add_project_file($shared_state, $iso_fid);

    $options['iso'] = TRUE;
    $options['records'] = $records;
    $options['cvterms'] = $phenotype_cvterms;
    $options['file_headers'] = tpps_file_headers($iso_fid);
    $options['organism_name'] = $organism_name;
    $options['meta'] = array(
      'desc' => "Mass Spectrometry",
      // Unit name replaced with Unit Id (cvterm_id for 'chemical substance').
      // Outdated: 'unit' => "intensity (arbitrary nits)".
      'unit' => 139527,
      'unit_id' => 139527,
      'attr_id' => tpps_load_cvterm('intensity')->cvterm_id,
      // Manual term for MASS Spec.
      'struct_id' => tpps_load_cvterm('whole plant')->cvterm_id,
    );

    print_r('ISO_FID:' . $iso_fid . "\n");
    tpps_log('[INFO] - Processing phenotype_data file data...');
    tpps_file_iterator($iso_fid, 'tpps_process_phenotype_data', $options);
    tpps_log('[INFO] - Inserting phenotype_data into database using insert_multi...');
    // [VS]
    $id_list = tpps_chado_insert_multi($options['records'], ['fks' => 'phenotype']);
    tpps_log('[INFO] - Done.');
  }
  // Store relations between Phenotype, Synonym, Unit.
  if ($id_list) {
    tpps_synonym_save($phenotypes_meta, $id_list);
  }
  // [/VS].
}

/**
 * Submits genotype information for one species.
 *
 * @param array $shared_state
 *   Submission Shared State. See class Submission.
 * @param array $species_codes
 *   An array of 4-letter species codes associated with the submission.
 * @param int $i
 *   The organism number we are submitting.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_submit_genotype(array &$shared_state, array $species_codes, $i, TripalJob &$job = NULL) {
  tpps_log('[INFO] - Submitting genotype data...');
  // Pages data.
  $page1_values = $shared_state['saved_values'][TPPS_PAGE_1];
  $page4_values = $shared_state['saved_values'][TPPS_PAGE_4];
  $genotype = $page4_values["organism-$i"]['genotype'] ?? NULL;
  // Project id is how this study is recorded in chado tables instead of TGDRXXXX
  $project_id = $shared_state['ids']['project_id'];
  // This record_group variable is the number of records in a batch
  // This can be set within the TPPS admin panel
  $record_group = variable_get('tpps_record_group', 10000);

  // VCF needed variables.
  // Remember if VCF is already processed or not.
  $vcf_processing_completed = FALSE;
  $vcf_import_mode = $page1_values['vcf_import_mode'] ?? 'hybrid';

  // If no genotype data, don't continue running this code.
  if (empty($genotype)) {
    return;
  }

  // Add tag genotype to this study.
  tpps_submission_add_tag($shared_state['accession'], 'Genotype');

  $genotype_count = 0;
  $genotype_total = 0;
  // 1491.
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
    'tree_info' => $shared_state['tree_info'],
    'species_codes' => $species_codes,
    'genotype_count' => &$genotype_count,
    'genotype_total' => &$genotype_total,
    'organism_index' => $i, // RISH [6/22/2023] to remember which organism this genotype data is connected to based on page 4
    'project_id' => $project_id,
    'seq_var_cvterm' => $seq_var_cvterm,
    'multi_insert' => &$multi_insert_options,
    'job' => &$job,
    'study_accession' => $shared_state['saved_values'][1]['accession']
  );
  $options['vcf_processing_completed'] = $vcf_processing_completed;

  // 2/29/2024 Add reference genome more consitently for all scenarios
  if ($shared_state['file_rank'] == NULL) {
    $shared_state['file_rank'] = 0;
  }

  if (in_array($genotype['ref-genome'], ['manual', 'manual2', 'url'])) {
    if ($genotype['tripal_fasta']['file_upload']) {
      // Uploaded new file.
      $assembly_user = $genotype['tripal_fasta']['file_upload'];
      tpps_add_project_file($shared_state, $assembly_user);
      tpps_chado_insert_record('projectprop', [
        'project_id' => $project_id,
        'type_id' => tpps_load_cvterm('file_path')->cvterm_id,
        'value' => $assembly_user,
        'rank' => $shared_state['file_rank'],
      ]);
    }
    if ($genotype['tripal_fasta']['file_upload_existing']) {
      // Uploaded existing file.
      $assembly_user = $genotype['tripal_fasta']['file_upload_existing'];
      tpps_add_project_file($shared_state, $assembly_user);
      tpps_chado_insert_record('projectprop', [
        'project_id' => $project_id,
        'type_id' => tpps_load_cvterm('file_path')->cvterm_id,
        'value' => $assembly_user,
        'rank' => $shared_state['file_rank'],
      ]);
    }
    if ($genotype['tripal_fasta']['file_remote']) {
      // Provided url to file.
      $assembly_user = $genotype['tripal_fasta']['file_remote'];
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
        'type_id' => tpps_load_cvterm('file_path')->cvterm_id,
        'value' => $assembly_user,
        'rank' => $shared_state['file_rank'],
      ));
      $shared_state['file_rank']++;
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
    // $vcf_exists = tpps_vcf_exists($shared_state, $i);
    // if ($vcf_exists) {
    //   // process vcf first to insert genotype and genotype_calls
    //   tpps_genotype_vcf_processing($shared_state, $species_codes, $i, $job, $vcf_import_mode);
    //   $vcf_processing_completed = true;
    // }

    $snp_fid = $genotype['files']['snps-assay'];
    tpps_add_project_file($shared_state, $snp_fid);

    $options['type'] = 'snp';
    $options['headers'] = tpps_file_headers($snp_fid);
    $options['marker'] = 'SNP';
    $options['type_cvterm'] = tpps_load_cvterm('snp')->cvterm_id;
    $options['ref-genome'] = $genotype['ref-genome'];

    // This variable is used to determine whether to import
    // genotype and genotype calls from assay or not
    // If the VCF was loaded, we don't need to import the SNPs Assay
    // genotypes and genotype_calls
    $ref_genome = $genotype['ref-genome'];
    echo "Ref-genome: $ref_genome\n";



    // Lookup analysis id from reference genome and add it to options array
    $options['analysis_id'] = tpps_get_analysis_id_from_ref_genome($ref_genome);

    if (!empty($genotype['files']['snps-association'])) {
      $assoc_fid = $genotype['files']['snps-association'];
      print_r("Association file ID: " . $assoc_fid . "\n");
      tpps_add_project_file($shared_state, $assoc_fid);

      $options['records']['featureloc'] = array();
      $options['records']['featureprop'] = array();
      $options['records']['feature_relationship'] = array();
      $options['records']['feature_cvterm'] = array();
      $options['records']['feature_cvtermprop'] = array();

      $options['associations'] = array();
      $options['associations_tool'] = $genotype['files']['snps-association-tool'];
      $options['associations_groups'] = $genotype['files']['snps-association-groups'];
      $options['scaffold_cvterm'] = tpps_load_cvterm('scaffold')->cvterm_id;
      $options['phenotype_meta'] = $shared_state['data']['phenotype_meta'];
      $options['pub_id'] = $shared_state['ids']['pub_id'];

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
      tpps_log('[INFO] - Processing snp_association file data...');
      tpps_file_iterator($assoc_fid, 'tpps_process_snp_association', $options);
      tpps_log('[INFO] - Done.');

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

      // Files are optional:
      if ($pop_struct_fid = ($genotype['files']['snps-pop-struct'] ?? NULL)) {
        tpps_add_project_file($shared_state, $pop_struct_fid);
      }
      if ($kinship_fid = ($genotype['files']['snps-kinship'] ?? NULL)) {
        tpps_add_project_file($shared_state, $kinship_fid);
      }
    }
    // DROP INDEXES FROM GENOTYPE_CALL TABLE.
    // tpps_drop_genotype_call_indexes($job);
    tpps_log('[INFO] - Processing SNP genotype_spreadsheet file data...');
    echo "trace 1\n";
    echo "Species codes:\n";
    print_r($options['species_codes']);
    echo "Tree Info:\n";
    print_r($options['tree_info']);
    tpps_file_iterator($snp_fid, 'tpps_process_genotype_spreadsheet', $options);
    tpps_log('[INFO] - Done.');

    tpps_log('[INFO] - Inserting SNP genotype_spreadsheet data into database using insert_multi...');
    tpps_chado_insert_multi($options['records'], $multi_insert_options);
    tpps_log('[INFO] - Inserting SNP genotype_spreadsheet data into database using insert_hybrid...');
    tpps_chado_insert_hybrid($options['records2'], $multi_insert_options);
    tpps_log('[INFO] - Done');

    // RECREATE INDEXES FROM GENOTYPE_CALL TABLE.
    // tpps_create_genotype_call_indexes();
    $options['records'] = $records;
    $genotype_total += $genotype_count;
    tpps_log('[INFO] - Genotype count:' . $genotype_count);
    $genotype_count = 0;
  }

  // This if statement caters for the Genotype Assay Design file
  // (which holds extra data like positions etc)
  // This is usually also accompanied with the Genotype SNP Assay
  // (which holds the snps)
  // We want to insert location data into the database
  // if both are found and the marker-type is snp
  // The previous step took care of the SNPs insertion via the Genotype
  // SNP Assay (not to be confused with genotype SNP assay design file)
  if (!empty($genotype['files']['assay-design'])) {
    $design_fid = $genotype['files']['assay-design'];
    tpps_add_project_file($shared_state, $design_fid);

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
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // 'SSRs' and 'cpSSR' fields.
  foreach (['ssrs', 'ssrs_extra'] as $ssr_field_name) {
    if (!empty($ssr_fid = $genotype['files'][$ssr_field_name])) {
      $options['type'] = 'ssrs';
      if ($ssr_field_name == 'ssrs') {
        $options['marker'] = 'SSR';
      }
      elseif ($ssr_field_name == 'ssrs_extra') {
        $options['marker'] = 'cpSSR';
      }
      // CV Term Id for 'ssr': 764.
      $options['type_cvterm'] = tpps_load_cvterm('ssr')->cvterm_id;
      $options['headers'] = tpps_ssrs_headers($ssr_fid, $genotype['files']['ploidy']);
      $options['ploidy'] = $genotype['files']['ploidy'];
      $options['empty'] = $genotype['files'][$ssr_field_name]['empty'] ?? 'NA';
      tpps_log('[SSR FID] ' . $ssr_fid);
      tpps_ssr_process(
        $shared_state,
        $ssr_fid,
        $options,
        $job,
        $multi_insert_options
      );
      $options['records'] = $records;
      $genotype_count = 0;
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Other.
  if (!empty($genotype['files']['other'])) {
    $other_fid = $genotype['files']['other'];
    tpps_add_project_file($shared_state, $other_fid);

    $options['headers'] = tpps_file_headers($other_fid);
    if (!empty($genotype['files']['other-groups'])) {
      $groups = $genotype['files']['other-groups'];
      $options['headers'] = tpps_other_marker_headers($other_fid, $groups['Genotype Data'][0]);
      $options['tree_id'] = $groups['Tree Id'][1];
    }

    // DROP INDEXES FROM GENOTYPE_CALL TABLE
    // tpps_drop_genotype_call_indexes($job);

    $options['type'] = 'other';
    $options['marker'] = $genotype['other-marker'];
    $options['type_cvterm'] = tpps_load_cvterm('genetic_marker')->cvterm_id;

    tpps_log('[INFO] - Processing OTHER MARKER genotype_spreadsheet file data...');
    echo "trace 5\n";
    tpps_file_iterator($other_fid, 'tpps_process_genotype_spreadsheet', $options);
    tpps_log('[INFO] - Done.');

    tpps_log('[INFO] - Inserting data into database using insert_multi...');
    tpps_chado_insert_multi($options['records'], $multi_insert_options);

    tpps_log('[INFO] - Inserting data into database using insert_hybrid...');
    tpps_chado_insert_hybrid($options['records2'], $multi_insert_options);
    tpps_log('[INFO] - Done.');

    // CREATE INDEXES FROM GENOTYPE_CALL TABLE.
    // tpps_create_genotype_call_indexes();

    $options['records'] = $records;
    $genotype_count = 0;
  }

  tpps_log('[INFO] - VCF IMPORT MODE is ' . $vcf_import_mode);
  if ($vcf_processing_completed == FALSE) {
    tpps_genotype_vcf_processing($shared_state, $species_codes, $i, $job, $vcf_import_mode, $options);
  }
  else {
    tpps_log('[INFO] - VCF was already processed!');
  }
}

/**
 * [RISH] [8/12/2024] Function to generate species_codes array from shared_state
 */
function tpps_generate_species_codes_array_from_shared_state($shared_state) {
  $species_codes = array();
  $organism_number = $shared_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
  for ($i = 1; $i <= $organism_number; $i++) {
    // Get the organism id from the organism name due to breaking changed made by Vlad
    $organism_name = $shared_state['saved_values'][1]['organism'][$i]['name'];
    $organism_name_parts = explode(" ", $organism_name, 2);
    print_r($organism_name_parts);
    $organism_name_genus = $organism_name_parts[0];
    $organism_name_species = $organism_name_parts[1];
    $organism_lookup_results = chado_query('SELECT organism_id FROM chado.organism WHERE genus ILIKE :genus AND species ILIKE :species',[
      ':genus' => $organism_name_genus,
      ':species' => $organism_name_species,
    ]);
    $organism_id = NULL;
    foreach ($organism_lookup_results as $organism_lookup_results_row) {
      $organism_id = $organism_lookup_results_row->organism_id;
      print_r("ORGANISM ID ($organism_name): " . $organism_id . "\n");
    }

    // Use the organism_id to lookup the 4 letter code
    $four_letter_code_results = chado_query('SELECT value FROM chado.organismprop WHERE type_id = :type_id AND organism_id = :organism_id', [
      ':organism_id' => $organism_id,
      'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id
    ]);
    $four_letter_code = NULL;
    foreach ($four_letter_code_results as $four_letter_code_results_row) {
      $four_letter_code = $four_letter_code_results_row->value;
      print_r("FOUR LETTER CODE ($organism_name): " . $four_letter_code . "\n");
    }

    if ($organism_id != NULL && $four_letter_code != NULL) {
      $species_codes[$organism_id] = $four_letter_code;
    }
    else {
      throw new Exception("Could not find the organism by name ($organism_name) or the species code for this organism");
    }
    
    // OLD CODE BEFORE 8/12/2024
    // $species_codes[$shared_state['ids']['organism_ids'][$i]] = current(chado_select_record('organismprop', array('value'), array(
    //   'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
    //   'organism_id' => $shared_state['ids']['organism_ids'][$i],
    // ), array(
    //   'limit' => 1,
    // )))->value;
  }
  return $species_codes;
}

/**
 * Tpps genotype_vcf_to_flat_files (CSV).
 *
 * This function will process all vcf files per organism (from genotype section) genotypic information
 * and store it within files.
 */
function tpps_genotypes_to_flat_files_and_find_studies_overlaps($form_state, $shared_state, $regenerate_all = TRUE) {
  
  
  $project_id = $shared_state['ids']['project_id'];
  $accession = $form_state['accession'];
  $dest_folder = tpps_realpath('public://tpps_vcf_flat_files');
  
  // print_r($form_state);
  // Generate species codes which is needed later on
  $organism_number = $shared_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
  echo "Organism Number: $organism_number\n";
  tpps_log("Organism Number: $organism_number\n");
  echo "Shared state Organism IDS:\n";
  print_r($shared_state['ids']['organism_ids']);
  echo "Form State Organism IDS:\n";
  print_r($form_state['ids']['organism_ids']);

  $species_codes = tpps_generate_species_codes_array_from_shared_state($shared_state);
  print_r("Species code\n");
  print_r($species_codes);
  // throw new Exception('debug');

  // Run genotype_vcf_to_flat_file per organism_index for the original study
  // @TODO Re-enable this
  for ($i = 1; $i <= $organism_number; $i++) {
    $snps_flat_file_location = $dest_folder . '/' . $accession . '-' . $i . '-snps.csv';
    echo "Checking if file exists: $snps_flat_file_location\n";
    // If file does not exist, we need to generate this
    if (!file_exists($snps_flat_file_location) || $regenerate_all == true) {
      echo "Generating: $snps_flat_file_location\n";
      tpps_genotypes_to_flat_file($form_state, $shared_state, $species_codes, $i);
    }
  }

  // GOAL: Look for similar studies that contain similar variant_ids
  global $study_accessions_with_potential_overlaps;
  $study_accessions_with_potential_overlaps = [];
  $accession_results = chado_query("select distinct accession from
    (select accession, unnest(markers) as marker from chado.studies_with_markers)x
    where marker in
    (select unnest(markers) as marker from chado.studies_with_markers where accession = '" . $accession . "');");
  foreach ($accession_results as $row) {
    if ($row->accession != strtoupper($accession)) {
      $study_accessions_with_potential_overlaps[] = $row->accession;
    }
  }
  // print_r($study_accessions_with_potential_overlaps);



  // Go through each additional study and run genotypes to flat file.
  foreach ($study_accessions_with_potential_overlaps as $study_accession) {
    $submission = new Submission($study_accession);
    $study_state = $submission->sharedState;

    $study_organism_number = $study_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
    echo "Study organism number for $study_accession is $study_organism_number\n";
    // Check if each study has overlap files
    for($i = 1; $i <= $study_organism_number; $i++) {
      $snps_flat_file_location = $dest_folder . '/' . $study_accession . '-' . $i . '-snps.csv';
      echo "Checking if file exists: $snps_flat_file_location\n";
      if (!file_exists($snps_flat_file_location) || $regenerate_all == true) {
        // GOAL: We need to generate flat files for this study
        // Step 1: Get species_codes for this study
        
        // New code by Rish to get the species code array
        $study_species_codes = tpps_generate_species_codes_array_from_shared_state($shared_state);

        // DEPRECATED 8/12/2024 due to Vlad's breaking changes code
        //$study_species_codes = array();
        // for ($j = 1; $j <= $organism_number; $j++) {
        //   $study_species_codes[$study_state['ids']['organism_ids'][$j]] = current(chado_select_record('organismprop', array('value'), array(
        //     'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
        //     'organism_id' => $study_state['ids']['organism_ids'][$j],
        //   ), array(
        //     'limit' => 1,
        //   )))->value;
        // }

        // Generate the flat files and necessary DB inserts for features, genotypes etc
        echo "Running tpps_genotypes_to_flat_file $study_accession $i\n";
        tpps_genotypes_to_flat_file($study_state, $shared_state, $study_species_codes, $i);
      }
      else {
        // Flat file exists so we can perform comparison operations to check between original study
        // and this particular study
      }
    }
  }

  // @TODO after running tests
  // exit;

  // Go through all studies and generate sorted list of the csv files
  $accession_results = chado_query("select distinct accession from
    (select accession, unnest(markers) as marker from chado.studies_with_markers)x
    where marker in
    (select unnest(markers) as marker from chado.studies_with_markers where accession = '" . $accession . "');");
  foreach ($accession_results as $row) {
    $study_accession = $row->accession;
    $submission = new Submission($study_accession);
    $study_state = $submission->sharedState;
    $study_organism_number = $study_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
    for ($i = 1; $i <= $study_organism_number; $i++) {
      $snps_flat_file_location = $dest_folder . '/' . $study_accession . '-' . $i . '-snps.csv';
      if (!is_file($snps_flat_file_location) || $regenerate_all == true) {
        $snps_sorted = $dest_folder . '/' . $study_accession . '-' . $i . '-snps-sorted.csv';
        $time_start = time();
        exec("cat $snps_flat_file_location | sort > $snps_sorted");
        $time_end = time();
        $time_elapsed = $time_end - $time_start;
        echo "[$time_elapsed s] File generated: $snps_sorted\n";
      }
      else {

      }
      echo "[SORTED LIST]: ". $snps_flat_file_location . "\n";
    }
  }

  $all_studies_array = [];
  $accession_results = chado_query("select distinct accession from
  (select accession, unnest(markers) as marker from chado.studies_with_markers)x
  where marker in
  (select unnest(markers) as marker from chado.studies_with_markers where accession = '" . $accession . "');");
  foreach ($accession_results as $row) {
    $all_studies_array[] = $row->accession;
  }
  print_r("All Studies Array:\n");
  print_r($all_studies_array);

  // // Code modified from https://r.je/php-find-every-combination


  // $words = array('red', 'blue', 'green');
  $all_combinations = [];
  $combination = [];
  $num = count($all_studies_array);
  // The total number of possible combinations.
  $total = pow(2, $num);
  // Loop through each possible combination.
  for ($i = 0; $i < $total; $i++) {
    // For each combination check if each bit is set.
    for ($j = 0; $j < $num; $j++) {
      // Is bit $j set in $i?
      if (pow(2, $j) & $i) {
        // echo $all_studies_array[$j] . ' ';
        $combination[] = $all_studies_array[$j];
      }
    }
    $all_combinations[] = json_decode(json_encode($combination));
    $combination = [];
    // echo '<br />';
  }

  // Remove every combo that isn't 2.
  $count_combinations = count($all_combinations);
  $unique_pairs = [];
  for ($i = 0; $i < $count_combinations; $i++) {
    if (count($all_combinations[$i]) == 2) {
      $unique_pairs[] = $all_combinations[$i];
    }
  }
  // print_r($unique_pairs);
  // return;

  // Go through each study other than the original and get repeats
  foreach ($unique_pairs as $pair) {
    $snps_flat_file_location_1 = $dest_folder . '/' . $pair[0] . '-1-snps-sorted.csv';
    $snps_flat_file_location_2 = $dest_folder . '/' . $pair[1] . '-1-snps-sorted.csv';
    $repeats_location = $dest_folder . '/' . $pair[0] . "-" . $pair[1] . "-repeats.csv";
    // unlink($repeats_location);
    $cmd = "comm $snps_flat_file_location_1 $snps_flat_file_location_2 | awk -F'\\t' '{print $3}' | sed '/^$/d' > $repeats_location";
    // echo "CMD: $cmd\n";
    if (!is_file($repeats_location) || $regenerate_all == true) {
      exec($cmd);
      // echo "Repeats Location: $repeats_location\n";
    }
    else {

    }

    echo "[Repeats Location]: $repeats_location\n";
  }

  // Remove repeats from corresponding studies.
  foreach ($unique_pairs as $pair) {
    $snps_flat_file_location_1 = $dest_folder . '/' . $pair[0] . '-1-snps-sorted.csv';
    $snps_flat_file_location_2 = $dest_folder . '/' . $pair[1] . '-1-snps-sorted.csv';
    $repeats_location = $dest_folder . '/' . $pair[0] . "-" . $pair[1] . "-repeats.csv";

    $repeats_removed_location_1 = $dest_folder . '/' . $pair[0] . '-1-snps-repeats-removed.csv';
    $repeats_removed_location_2 = $dest_folder . '/' . $pair[1] . '-1-snps-repeats-removed.csv';

    if (!is_file($repeats_removed_location_1) || $regenerate_all == TRUE) {
      // exec("grep -v -x -f $repeats_location $snps_flat_file_location_1 > $repeats_removed_location_1");
      exec("awk 'NR==FNR{a[$0]=1;next}!a[$0]' $repeats_location $snps_flat_file_location_1 > $repeats_removed_location_1");
    }
    echo "[Repeats removed]: $repeats_removed_location_1\n";

    if (!is_file($repeats_removed_location_2) || $regenerate_all == TRUE) {
      // exec("grep -v -x -f $repeats_location $snps_flat_file_location_2 > $repeats_removed_location_2");
      exec("awk 'NR==FNR{a[$0]=1;next}!a[$0]' $repeats_location $snps_flat_file_location_2 > $repeats_removed_location_2");
    }
    echo "[Repeats removed]: $repeats_removed_location_2\n";
  }

  // Distinct repeats_removed.
  foreach ($unique_pairs as $pair) {
    $repeats_removed_location_1 = $dest_folder . '/' . $pair[0] . '-1-snps-repeats-removed.csv';
    $repeats_removed_location_2 = $dest_folder . '/' . $pair[1] . '-1-snps-repeats-removed.csv';

    $distinct_repeats_removed_location_1 = $dest_folder . '/' . $pair[0] . '-1-snps-repeats-removed-distinct-snps.csv';
    $distinct_repeats_removed_location_2 = $dest_folder . '/' . $pair[1] . '-1-snps-repeats-removed-distinct-snps.csv';

    // unlink($distinct_repeats_removed_location_1);
    // unlink($distinct_repeats_removed_location_2);

    if (!is_file($distinct_repeats_removed_location_1) || $regenerate_all == true) {
      exec("awk -F',' '{print $2}' $repeats_removed_location_1 | sort | uniq > $distinct_repeats_removed_location_1");
    }
    echo "[Distinct repeats removed snps]: $distinct_repeats_removed_location_1\n";

    if (!is_file($distinct_repeats_removed_location_2) || $regenerate_all == true) {
      exec("awk -F',' '{print $2}' $repeats_removed_location_2 | sort | uniq > $distinct_repeats_removed_location_2");
    }
    echo "[Distinct repeats removed snps]: $distinct_repeats_removed_location_2\n";
  }

  // Check for repeats between distinct_snps
  foreach ($unique_pairs as $pair) {
    $distinct_repeats_removed_location_1 = $dest_folder . '/' . $pair[0] . '-1-snps-repeats-removed-distinct-snps.csv';
    $distinct_repeats_removed_location_2 = $dest_folder . '/' . $pair[1] . '-1-snps-repeats-removed-distinct-snps.csv';

    $overlapping_snps_location = $dest_folder . '/' . $pair[0] . '-' . $pair[1] . '-1-snps-overlapping.txt';
    if (!is_file($overlapping_snps_location) || $regenerate_all == true) {
      exec("comm $distinct_repeats_removed_location_1 $distinct_repeats_removed_location_2 | awk -F'\\t' '{print $3}' | sed '/^$/d' > $overlapping_snps_location");
    }
    echo "[OVERLAPPING SNPS LOCATION]: $overlapping_snps_location\n";
  }

  // Now we need to add this data to the database.
  foreach ($unique_pairs as $pair) {
    $overlapping_snps_location = $dest_folder . '/' . $pair[0] . '-' . $pair[1] . '-1-snps-overlapping.txt';
    // Try to do this efficiently.
    $snps = [];
    $handle = fopen($overlapping_snps_location, "r");
    if ($handle) {
      while (($line = fgets($handle)) !== FALSE) {
        // Process the line read.
        if (trim($line) != '' || trim($line) == ' ') {
          $snps[] = "'" . trim($line) . "'";
        }
      }
    }
    fclose($handle);
    $snps_count = count($snps);
    echo "SNPs count $snps_count between " . $pair[0] . " and " . $pair[1] . "\n";
    // print_r($snps);
    // print_r(implode(',', $snps));

    if ($snps_count > 0) {
      // Check if this data exists, if it doesn't insert, else update.
      $results = chado_query("SELECT count(*) as c1 FROM chado.studies_marker_overlaps
        WHERE '" . $pair[0] . "'= ANY(accession)
        AND '" . $pair[1] . "'= ANY(accession)");
      $count = $results->fetchObject()->c1;
      // Row exists, delete it before inserting new row.
      if ($count == 0) {
        chado_query("DELETE FROM chado.studies_marker_overlaps
        WHERE '" . $pair[0] . "'= ANY(accession)
        AND '" . $pair[1] . "'= ANY(accession)");
      }
      chado_query('INSERT INTO chado.studies_marker_overlaps (accession,overlap) VALUES (' .
        'ARRAY[\'' . $pair[0] . '\',\'' . $pair[1] . '\'], ARRAY[' . implode(',', $snps) . ']' .
      ')');
    }
    else {
      echo "No SNPs overlaps found between " . $pair[0] . " and " . $pair[1] . "\n";
    }
  }
  echo "ALL COMPLETED!\n";
}

/**
 * Tpps genotype_vcf_to_flat_file (CSV).
 *
 * This function will process a vcf file's genotypic information
 * and store it within the a file.
 *
 * @param array $form_state
 * @param array $species_codes
 * @param mixed $i
 * @param TripalJob $job
 * @access public
 *
 * @return void
 */
function tpps_genotypes_to_flat_file($form_state, $shared_state, array $species_codes, $i, $is_primary_study = true, TripalJob &$job = NULL, $insert_mode = 'hybrid') {
  $project_id = $shared_state['ids']['project_id'];
  $organism_index = $i;
  // Some initial variables previously inherited from the parent function code. So we're reusing it to avoid
  // missing any important variables if we rewrote it.
  $page1_values = $shared_state['saved_values'][TPPS_PAGE_1];
  $page4_values = $shared_state['saved_values'][TPPS_PAGE_4];
  // print_r("Page 4 values\n");
  // print_r($page4_values);
  $genotype = $page4_values["organism-$i"]['genotype'] ?? NULL;

  if ($insert_mode == '') {
    throw new Exception('VCF processing insert mode was empty - it should have a value of either hybrid or inserts.');
  }



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
    'accession' => $shared_state['accession'],
    'tree_info' => $shared_state['tree_info'],
    'species_codes' => $species_codes,
    'genotype_count' => &$genotype_count,
    'genotype_total' => &$genotype_total,
    'project_id' => $project_id,
    'seq_var_cvterm' => $seq_var_cvterm,
    'multi_insert' => &$multi_insert_options,
    'job' => &$job,
    'study_accession' => $shared_state['saved_values'][1]['accession']
  );

  // check to make sure admin has not set disable_vcf_importing.
  $disable_vcf_import = 0;
  if (isset($page1_values['disable_vcf_import'])) {
    $disable_vcf_import = $page1_values['disable_vcf_import'];
  }
  echo "VCF Importer Disabled: " . $disable_vcf_import . "\n";
  tpps_job_logger_write('[INFO] Disable VCF Import is set to ' . $disable_vcf_import . ' (0 means allow vcf import, 1 ignore vcf import)');
  // RISH: 12/6/2023
  $accession = $shared_state['accession'];
  if ($genotype['files']['file-type'] == TPPS_GENOTYPING_FILE_TYPE_VCF) {
    // @TODO Comment out after testing
    // echo "Skipping VCF processing during debug\n";
    // return;
    echo "Detecting VCF file type: VCF\n";

    $transaction = db_transaction();
    try {
    //if ($disable_vcf_import == 0) {
      // tpps_drop_genotype_call_indexes($job);

      // @todo we probably want to use tpps_file_iterator to parse vcf files.
      $vcf_fid = $genotype['files']['vcf'];
      
      // check project already exists
      $results_project_file = chado_query("SELECT count(*) AS c1 FROM public.tpps_project_file_managed 
        WHERE project_id = :project_id
        AND fid = :fid", 
      [
        ':project_id' => $project_id,
        ':fid' => $vcf_fid
      ]);
      $project_file_count = 0;
      foreach ($results_project_file as $results_project_file_row) {
        $project_file_count = $results_project_file_row->c1;
      }

      if ($project_file_count == 0) {
        // Add this file to the project
        tpps_add_project_file($shared_state, $vcf_fid);
      }


      $records['genotypeprop'] = array();

      $snp_cvterm = tpps_load_cvterm('snp')->cvterm_id;
      $format_cvterm = tpps_load_cvterm('format')->cvterm_id;
      $qual_cvterm = tpps_load_cvterm('quality_value')->cvterm_id;
      $filter_cvterm = tpps_load_cvterm('filter')->cvterm_id;
      $freq_cvterm = tpps_load_cvterm('allelic_frequency')->cvterm_id;
      $depth_cvterm = tpps_load_cvterm('read_depth')->cvterm_id;
      $n_sample_cvterm = tpps_load_cvterm('number_samples')->cvterm_id;

      // This means it was uploaded
      print_r("VCF_FID: $vcf_id\n");
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

      // [RISH] [12/14/2023] [STEP 1] Perform one liner related code for flat files and db insertions
      // [STEP 1 A - Run a check for duplicate sample names]
      $cmd_output = [];
      $found_duplicate_sample_ids = false;
      $pathinfo = pathinfo($location);
      $duplicate_warnings_location = $pathinfo['dirname'] . '/' .  $pathinfo['filename'] . '-duplicate-warnings.txt';
      $cmd = 'bcftools query -l ' . $location . ' &> ' . $duplicate_warnings_location;
      exec($cmd);
      echo "Duplicate sample names checked\n";
      // print_r($cmd);
      $cmd_output = file($duplicate_warnings_location);
      // print_r($cmd_output);
      foreach ($cmd_output as $line) {
        if (stripos($line, 'Duplicated sample name')) {
          echo "Found duplicated sample names\n";
          $found_duplicate_sample_ids = true;
          break;
        }
      }
      $cmd_output = []; // reset

      // [STEP 1 - B - If duplicates were found, we need to fix the VCF file]
      $vcf_fixed_location = NULL;
      if ($found_duplicate_sample_ids == true) {
        // We should generate a location to push the rename list to
        $pathinfo = pathinfo($location);
        $sample_rename_location = $pathinfo['dirname'] . '/' . $pathinfo['filename'] . '-sample-rename-list.txt';
        @unlink($sample_rename_location);
        echo "Sample Rename Location: $sample_rename_location\n";
        // Get renamed sample list
        $cmd = '
          awk \'
          BEGIN { OFS="\t" }  # Set output field separator to tab
          /^#CHROM/ {
              for (i = 10; i <= NF; i++) {
                  original = $i
                  if ($i in seen) { $i = $i "_" ++seen[$i] }
                  else { seen[$i] = 1 }
                  print $i
              }
              exit  # Exit after processing the header line
          }
          \' ' . $location . ' > "' . $sample_rename_location . '"
        ';
        // echo $cmd . "\n";
        $cmd_output = []; // reset
        exec($cmd, $cmd_output);


        // Regenerate the VCF with the corrected samples
        $vcf_fixed_location = $pathinfo['dirname'] . '/' . $pathinfo['filename'] . '-fixed-header.vcf';
        echo "VCF Fixed location: $vcf_fixed_location\n";
          $cmd = 'bcftools reheader -s '. $sample_rename_location . ' ' . $location . ' > ' . $vcf_fixed_location;
        $cmd_output = []; // reset
        exec($cmd, $cmd_output);
      }


      // STEP 1 C - GZIP THE VCF IF IT IS NOT ALREADY GZIPPED
      $valid_vcf = NULL;
      $valid_vcf_gz = NULL;
      if ($vcf_fixed_location != NULL) {
        $valid_vcf = $vcf_fixed_location;
      }
      else {
        $valid_vcf = $location;
      }
      echo "Valid VCF location: $valid_vcf\n";


      $pathinfo_valid_vcf = pathinfo($valid_vcf);

      $cmd_output = [];
      if (strtolower($pathinfo_valid_vcf['extension']) != 'gz') {
        $valid_vcf_gz = $pathinfo_valid_vcf['dirname'] . '/' . $pathinfo_valid_vcf['basename'] . '.gz';
        // perform compression
        $cmd_output = []; // reset
        echo "Gzipping VCF file\n";
        exec('gzip -c ' . $valid_vcf . ' > ' . $valid_vcf_gz, $cmd_output);
      }
      else {
        $valid_vcf_gz = $valid_vcf;
      }
      echo "Valid VCF GZ location: $valid_vcf_gz\n";

      // STEP 1 D - GET ALL TREES AND MARKERS
      $pathinfo = pathinfo($location);
      $trees_markers_location = $pathinfo['dirname'] . '/' . $pathinfo['filename'] . '-trees-markers.txt';
      // $cmd = "bcftools query -f '[%ID\t%SAMPLE\t%GT ]\\n' " . $valid_vcf_gz . " | tr ' ' '\\n' | awk -F '\\t' '{if ($3 != \"./.\") print $1,$2}' > " . $trees_markers_location;
      $cmd = "bcftools query -f '[%ID\\t%SAMPLE\\t%GT ]\\n' " . $valid_vcf_gz . " | tr ' ' '\\n' | awk -F '\\t' '{if ($3 != \"./.\") print " . '$1,$2' . "}' > $trees_markers_location";
      // echo $cmd . "\n";
      $cmd_output = [];
      exec($cmd, $cmd_output);
      echo "Trees Markers Location: $trees_markers_location\n";

      // STEP 1 E - GET ALL UNIQUE TREES
      $trees_unique_location = $pathinfo['dirname'] . '/' . $pathinfo['filename'] . '-trees-unique.txt';
      $cmd_output = [];
      $cmd = "bcftools query -l $valid_vcf_gz";
      exec($cmd, $cmd_output);
      // print_r($cmd_output);

      $unique_tree_ids = $cmd_output;
      $count = 0;
      $accession = $options['accession'];
      chado_query("DELETE FROM chado.markers_and_study_accession_per_individual_tree WHERE accession = '$accession'");
      foreach ($unique_tree_ids as $unique_tree_id) {
        $count++;
        // search for all markers ($trees_markers_location)
        $unique_tree_id = trim($unique_tree_id);
        if ($unique_tree_id != "") {
          $cmd2 = "awk '$2 == \"" . $unique_tree_id . "\" {print $1}' $trees_markers_location";
          // print_r($cmd . "\n");
          $cmd2_output = [];
          exec($cmd2, $cmd2_output);
          // print_r($cmd2_output);
          // print_r("\n");

          $markers_array = [];
          $markers_array_count = 0;
          foreach ($cmd2_output as $line) {
            $markers_array_count++;
            $line = trim($line);
            $markers_array[] = "'" . $line . "'";
          }
          $cmd2_output = []; // reset
          // INSERT THE MARKERS
          echo "Inserting markers ($markers_array_count) for $accession TREE_ID $accession-$unique_tree_id\n";
          try {
            chado_query("INSERT INTO chado.markers_and_study_accession_per_individual_tree (accession, tree_id, markers)
              VALUES (
                '$accession','$accession-$unique_tree_id', ARRAY[" . implode(',',$markers_array) . "]
              )
            ");
          }
          catch (Exception $ex) {
            print_r($ex);
          }
          $markers_array = []; // reset
        }
      }


      // [RISH] [Original code] [STEP 2] - Continue with processing VCF files
      $vcf_content = gzopen($location, 'r');
      $stocks = array();
      $format = "";
      // This was done by Peter
      // $current_id = $shared_state['ids']['organism_ids'][$i];
      // $species_code = $species_codes[$current_id];

      $tree_ids = [];
      $variant_ids = [];
      // We need to create a file to write SNPS data to
      $dest_folder = tpps_realpath('public://tpps_vcf_flat_files');
      @mkdir($dest_folder);
      $snps_flat_file_location = $dest_folder . '/' . $accession . '-' . $i . '-snps.csv';
      echo '[FILE_LOCATION][SNPs FLAT FILE CSV] ' . $dest_folder . '/' . $accession . '-' . $i . '-snps.csv' . "\n";
      $fhandle = fopen($snps_flat_file_location, 'w');

      // Override the above code done by Rish to use organism_id from page 4
      // RISH NOTES: This addition uses the organism_id based on the organism order
      // of the fourth page (we likely have to pass the i from previous function here)
      // THIS TECHNICALLY OVERRIDES PETER'S LOGIC ABOVE. TO BE DETERMINED IF RISH'S WAY IS CORRECT
      // OR NOT [7/1/2023]
      // THIS WAS AN ISSUE BROUGHT UP BY EMILY REGARDING SNPS NOT BEING ASSOCIATED WITH POP TRICH (665 STUDY)
      $species_code = null;
      $current_id = null;
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
        // DEPRECATED 8/12/2024 in favour of chado.tpps_ref_assembly_view created by Emily
        // // Get the species and version from the reference genome selected
        // // if match occurs thats in index [0].
        // // The group match index [1] is species, group match index [2] is version
        // preg_match('/(.+) +v(\d*\.*\d*)/', $ref_genome, $matches);
        // $ref_genome_species = NULL;
        // $ref_genome_version = NULL;
        // if (count($matches) > 0) {
        //   $ref_genome_species = $matches[1];
        //   $ref_genome_version = $matches[2];
        // }

        // if (isset($ref_genome_species) && isset($ref_genome_version)) {
        //   // Look up the analysis
        //   $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
        //     WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
        //     [
        //       ':name' => $ref_genome_species . '%',
        //       ':programversion' => $ref_genome_version
        //     ]
        //   );
        //   foreach ($analysis_results as $row) {
        //     $analysis_id = $row->analysis_id;
        //   }
        // }

        // if($analysis_id == NULL) {
        //   // Look up the analysis
        //   $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
        //     WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
        //     [
        //       ':name' => $ref_genome_species . '%',
        //       ':programversion' => 'v' . $ref_genome_version
        //     ]
        //   );
        //   foreach ($analysis_results as $row) {
        //     print_r("analysis_row\n");
        //     print_r($row);
        //     print_r("\n");
        //     $analysis_id = $row->analysis_id;
        //   }
        // }

        // // If an analysis_id still was not found, it's possibly from the db data source
        // // instead of the genome directory. The genome directory code is in page_4*.php
        // // New code to cater for new analysis checks via db - query given by Emily Grau (6/6/2023)
        // if ($analysis_id == NULL) {
        //   $genome_query_results = chado_query("select * from chado.tpps_ref_assembly_view WHERE name LIKE :ref_genome;", [
        //     ':ref_genome' => $ref_genome
        //   ]);
        //   foreach ($genome_query_results as $genome_query_row) {
        //     $analysis_id = $genome_query_row->analysis_id;
        //   }
        // }

        $analysis_id = tpps_get_analysis_id_from_ref_genome($ref_genome);

        // Once an analysis_id was found, try to get srcfeature_id

      }
      else {
       $error_line = [
          "A reference genome could not be found in the TPPS page 4 form.",
          "Without this, we cannot find the analysis_id and thus the srcfeature_id.",
          "Featureloc data will not be recorded",
        ];
        tpps_log('[REF GENOME NOT FOUND] - ' . implode("\n", $error_line) . "\n");
      }

      tpps_log("Analysis ID Found: $analysis_id\n");
      print_r("Analysis ID Found: $analysis_id\n");

      echo "[INFO] Processing Genotype VCF file\n";
      // throw new Exception("DEBUG");
      $file_progress_line_count = 0;
      $record_count = 0;
      while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
        $file_progress_line_count++;
        if($file_progress_line_count % 10000 == 0 && $file_progress_line_count != 0) {
          echo '[INFO] [VCF PROCESSING STATUS] ' . $file_progress_line_count . " lines done\n";
        }
        // If not a header line, perform processings
        if ($vcf_line[0] != '#'
          && stripos($vcf_line, '.vcf') === FALSE
          && trim($vcf_line) != ""
          && str_replace("\0", "", $vcf_line) != ""
        ) {
          $line_process_start_time = microtime(true);
          $record_count = $record_count + 1;
          // DEBUG - TEST ONLY 100 records
          // if ($record_count > 100) {
          //   break;
          // }
          print_r('Record count:' . $record_count . "\n");
          $genotype_count += count($stocks);
          $vcf_line = explode("\t", $vcf_line);
          $scaffold_id = explode(" ",$vcf_line[0])[0]; // take the first part if it is space delimited (8/13/2024 RISH discussion with Emily)
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
          // Emily updated suggestion on Tuesday August 9th 2022
          $marker_name = $scaffold_id . '_' . $position . '-' . $species_code;
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

          // Check to see if marker exists in the feature's table
          $feature_check_results = chado_query('SELECT count(*) as c1 FROM chado.feature WHERE uniquename = :marker_name',[
            ':marker_name' => $marker_name
          ]);
          $feature_check_count = $feature_check_results->fetchObject()->c1;
          // If marker does not exist, insert it into the feature table
          if ($feature_check_count <= 0) {
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
          }
          // get the marker_id <- feature_id column value
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

          // Check if variant_id already exists in the feature's table
          $feature_check_results = chado_query('SELECT count(*) as c1 FROM chado.feature WHERE uniquename = :variant_name',[
            ':variant_name' => $variant_name
          ]);
          $feature_check_count = $feature_check_results->fetchObject()->c1;
          // If marker does not exist, insert it into the feature table
          $feature_exists = NULL;
          if ($feature_check_count <= 0) {
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
          }
          else {
            $feature_exists = true; // remember it already exists (this is used further down to block featureloc)
          }

          // get the feature_id
          $results = chado_query('SELECT feature_id FROM chado.feature WHERE uniquename = :uniquename', [
            ':uniquename' => $variant_name
          ]);
          $row_object = $results->fetchObject();
          $variant_id = $row_object->feature_id;

          // // Lookup whether marker is already inserted into the features table.
          // $result = chado_query(
          //   "SELECT * FROM chado.feature WHERE uniquename = :marker_name",
          //   // Column 3 of VCF.
          //   [':marker_name' => $variant_name]
          // );

          if ($feature_exists) {
            tpps_log("Feature (variant): $variant_name exists already, featureloc will not be added\n");
            echo("Feature (variant): $variant_name exists, featureloc will not be added\n");
          }
          else {
            tpps_log("Feature (variant): $variant_name not found so it was created\n");
            echo("Feature (variant): $variant_name not found so it was created\n");
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
                'feature_id' => $variant_id,
                'srcfeature_id' => $srcfeature_id,
                'fmin' => $position,
                'fmax' => $fmax // ALPHA code above now caters for INDELS
              ];

              // Since we haven't catered for deletion of these featureloc records
              // there may already exist, we have to make sure the record doesn't already exist
              $featureloc_results = chado_query('SELECT count(*) as c1 FROM chado.featureloc
                WHERE feature_id = :feature_id AND srcfeature_id = :srcfeature_id;', [
                  ':feature_id' => $variant_id,
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
                $records['featureloc'][$variant_name] = $featureloc_values;
                echo "Featureloc for $variant_name will be created\n";
              }
            }
          }
          // throw New Exception('DEBUG');

          // Rish 12/08/2022: So we have multiple genotypes created
          // So I adjusted some of this code into a for statement
          // since the genotype_desc seems important and so I modified
          // it to be unique and based on the genotype_name.
          $genotype_names = array_keys($detected_genotypes);

          // print_r($detected_genotypes);
          // echo "\n";
          // echo "line#$file_progress_line_count ";
          // print_r('genotypes per line: ' . count($genotype_names) . " ");

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

            // echo "name: $marker_type . '-' . $genotype_name_without_combination, uniquename: $genotype_desc\n";
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
            // get the feature_id.
            $results = chado_query('SELECT genotype_id FROM chado.genotype WHERE uniquename = :uniquename', [
              ':uniquename' => $genotype_desc
            ]);
            $row_object = $results->fetchObject();
            $genotype_id = $row_object->genotype_id;
            // $debug_info = "Uniquename: $genotype_desc Type_id:$format_cvterm Value:$format Genotype_id:$genotype_id Variant_id:$variant_id Marker_id:$marker_id\n";
            // $debug_info = "Variant_name: $variant_name, Variant_id: $variant_id\n";
            // echo("DEBUG INFO: $debug_info");

            // 3/27/2023 Meeting - FORMAT: REVIEW THIS IN TERMS OF IF WE NEED IT.
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
            
            // echo "gen_name_index:$genotype_name_progress_count colcount:$vcf_cols_count ";
            for ($j = 9; $j < $vcf_cols_count; $j++) {
              // Rish: This was added on 09/12/2022
              // This gets the name of the current genotype for the tree_id column
              // being checked.

              $j_column_data = $vcf_line[$j];
              // @TODO We need to cater for extra metadata eg. 1/1:0,98:98:99:3055,289,0 <-- the data after the : is metadata
              $j_read = explode(':',$j_column_data)[0]; // gets the 1/1 part

              $val_combination = tpps_submit_vcf_render_genotype_combination($j_read, $ref, $alt);
              $column_genotype_name = $marker_type . '-' . $marker_name . '-' . tpps_submit_vcf_render_genotype_combination($vcf_line[$j], $ref, $alt);
              // echo 'Column Genotype Name: ' . $column_genotype_name . " Genotype Name: $genotype_name\n";
              if($column_genotype_name == $genotype_name) {
                // Found a match between the tree_id genotype and the genotype_name from records
                // echo "Found match (and using variant_name $variant_name ($variant_id) to add to genotype call\n";

                // [RISH] 02/26/2024
                // Insert genotype reads into chado.genotype_reads_per_plant
                // We need plant name, study, marker_name
                // $options['tree_id'], $options['study_accession'], $marker_name
                // Check if a record already exists, if not, create initial record

                $study_accession = $options['study_accession'];
                $tree_id = $study_accession . '-' . $tree_ids[$j - 9];
                $per_plant_results = chado_query('
                  SELECT COUNT(*) as c1 FROM chado.genotype_reads_per_plant
                  WHERE tree_acc = :tree_id AND study_accession = :study_accession
                ', [
                  ':tree_id' => $tree_id,
                  ':study_accession' => $study_accession
                ]);
                $per_plant_records_count = $per_plant_results->fetchObject()->c1;
                if ($per_plant_records_count == 0) {
                  // CREATE AN EMPTY RECORD IN TABLE
                  chado_query("
                    INSERT INTO chado.genotype_reads_per_plant
                    (tree_acc, study_accession, marker_array, read_array)
                    VALUES
                    ('$tree_id', '$study_accession', ARRAY[]::text[], ARRAY[]::text[])
                  ");
                }
                // So now we have a record in the table for the plant, so append the new values
                chado_query("
                  UPDATE chado.genotype_reads_per_plant
                  set marker_array = array_append(marker_array, '$marker_name')
                  WHERE tree_acc = '$tree_id' AND study_accession = '$study_accession'
                ");
                chado_query("
                  UPDATE chado.genotype_reads_per_plant
                  set read_array = array_append(read_array, '$val_combination')
                  WHERE tree_acc = '$tree_id' AND study_accession = '$study_accession'
                ");

                // It is in use for genotype materialized views and Emily's function to generate tables
                // which is used for tpps/details page
                $records['stock_genotype']["{$stocks[$j - 9]}-$genotype_name"] = array(
                  'stock_id' => $stocks[$j - 9],
                  // PETER
                  // '#fk' => array(
                  //   'genotype' => $genotype_desc,
                  // ),
                  // RISH
                  'genotype_id' => $genotype_id,
                );


                // ob_start();
                chado_insert_record('feature_genotype', [
                  'feature_id' => $variant_id,
                  'genotype_id' => $genotype_id,
                  'chromosome_id' => NULL,
                  'rank' => 0,
                  'cgroup' => 0,
                  'cvterm_id' => $snp_cvterm,
                ]);
                // ob_end_clean();

                $variant_ids[$variant_id] = true;

                $csv_line = $tree_ids[$j] . ',' . $variant_name . "\n";
                // echo $csv_line;
                fwrite($fhandle, $csv_line);

                // 26/02/2024 So we need to insert genotype reads into table genotype_reads_per_plant

                // RISH: Removed on 12/6/2023 to avoid genotype_call inserts which are slow
                // print_r('[genotype_call insert]: ' . "{$stocks[$j - 9]}-$genotype_name" . "\n");
                // $records['genotype_call']["{$stocks[$j - 9]}-$genotype_name"] = array(
                //   'project_id' => $project_id,
                //   'stock_id' => $stocks[$j - 9],
                //   // PETER
                //   // '#fk' => array(
                //   //   'genotype' => $genotype_desc,
                //   //   'variant' => $variant_name,
                //   //   'marker' => $marker_name,
                //   // ),
                //   // RISH
                //   'genotype_id' => $genotype_id,
                //   'variant_id' => $variant_id,
                //   'marker_id' => $marker_id,
                // );

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
          print_r("VCF_LINE\n");
          print_r($vcf_line);
          echo "\nGenotype call records to insert (LINE:$file_progress_line_count): " . count($records['genotype_call']) . "\n";
          // echo "\nrecord group threshold: $record_group ";
          // throw new Exception('DEBUG');
          // Tripal Job has issues when all submissions are made at the same
          // time, so break them up into groups of 10,000 genotypes along with
          // their relevant genotypeprops.
          if ($genotype_count > $record_group) {
            tpps_log('[INFO] - Last bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            tpps_log('[INFO] - Last insert cumulative time: ' . $insert_cumulative_time . ' seconds');
            $genotype_count = 0;
            $insert_start_time = microtime(true);
            if ($insert_mode == 'multi') {
              tpps_log('[INFO] - Inserting data into database using insert_multi...');
              tpps_chado_insert_multi($records, $multi_insert_options);
            }
            else if ($insert_mode == 'hybrid') {
              // THIS WILL DO SOME MULTI INSERTS BUT ALSO USE COPY FOR GENOTYPE_CALL TABLE
              tpps_log('[INFO] - Inserting data into database using insert_hybrid...');
              tpps_chado_insert_hybrid($records, $multi_insert_options);
            }

            tpps_log('[INFO] - Done.');
            $insert_end_time = microtime(true);
            $insert_elapsed_time = $insert_end_time - $insert_start_time;
            tpps_log('[INFO] - Bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            if(!isset($insert_cumulative_time)) {
              $insert_cumulative_time = 0;
            }
            $insert_cumulative_time += $insert_elapsed_time;
            tpps_log('[INFO] - Insert cumulative time: ' . $insert_cumulative_time . ' seconds');
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
            $stocks[] = $shared_state['tree_info'][trim($vcf_line[$j])]['stock_id'];
            $tree_ids[$j] = trim($vcf_line[$j]);
          }
        }
      }
      // Insert the last set of values.
      if ($insert_mode == 'multi') {
        tpps_log('[INFO] - Inserting data into database using insert_multi...');
        tpps_chado_insert_multi($records, $multi_insert_options);
      }
      elseif ($insert_mode == 'hybrid') {
        tpps_log('[INFO] - Inserting data into database using insert_hybrid...');
        tpps_chado_insert_hybrid($records, $multi_insert_options);
      }

      // Recreate the indexes.
      // tpps_create_genotype_call_indexes();
      tpps_log('[INFO] - Done.');
      unset($records);
      $genotype_count = 0;
      fclose($fhandle);
      echo "SNPs Flat file generation completed\n";


      // GOAL: Push unique variants to chado.studies_with_markers
      $unique_variant_ids_string = implode(',',array_keys($variant_ids));
      echo "Unique_variant_ids_string: $unique_variant_ids_string \n";
      unset($variant_ids);

      // UNIQUE VARIANTS Step 1 - check to see if there's already a record
      $accession_count_results = chado_query('SELECT count(*) as c1 FROM chado.studies_with_markers WHERE accession ILIKE :accession', [
        ':accession' => $accession
      ]);
      $accession_count = $accession_count_results->fetchObject()->c1;
      

      // UNIQUE VARIANTS Step 2 - INSERT if no record exists or UPDATE if record exists
      if ($accession_count > 0) {
        // UPDATE
        echo "UPDATE chado.studies_with_markers for $accession\n";
        chado_query("UPDATE chado.studies_with_markers SET markers = ARRAY[" . $unique_variant_ids_string . "] WHERE accession ILIKE '" . $accession . "';");
      }
      else {
        // INSERT
        echo "INSERT chado.studies_with_markers for $accession\n";
        chado_query("insert into chado.studies_with_markers (accession, markers) values ('" . $accession . "', ARRAY[" . $unique_variant_ids_string . "]);");
      }
      // Clean up memory
      unset($unique_variant_ids_string);



      // Generating unique treeids flat file
      //echo "Generate distinct Tree IDs flat file\n";
      $snps_distinct_tree_ids_flat_file_location = $dest_folder . '/' . $accession . '-' . $i . '-distinct-treeids.txt';
      echo "[FILE_LOCATION][DISTINCT TREE IDS] " . $snps_distinct_tree_ids_flat_file_location . "\n";
      $fhandle_distinct_treeids = fopen($snps_distinct_tree_ids_flat_file_location, 'w');
      sort($tree_ids);
      foreach ($tree_ids as $tree_id) {
        fwrite($fhandle_distinct_treeids, $tree_id . "\n");
      }
      fclose($fhandle_distinct_treeids);
      // Clear up memory
      unset($tree_ids);
      // echo "Distinct Tree IDs completed\n";

      //echo "Generate distinct variant names...";
      // awk -F',' 'NR>1 && !a[$2]++{print $2}' /var/www/Drupal/sites/default/files/tpps_vcf_flat_files/TGDR674-1-snps.csv
      $snps_distinct_flat_file_location = $dest_folder . '/' . $accession . '-' . $i . '-distinct-snps.txt';
      echo "[FILE_LOCATION][DISTINCT VARIANT NAMES] " . $snps_distinct_flat_file_location . "\n";
      exec("awk -F',' 'NR>1 && !a[$2]++{print $2}' $snps_flat_file_location | sort -u > $snps_distinct_flat_file_location");
      // echo "Distinct variant names completed\n";
      echo "Finished\n";

    // }
    } catch (Exception $ex) {
      print_r($ex);
      $transaction->rollback();
      exit;
    }
  }
  else if (isset($genotype['files']['snps-assay'])) {
    // @TODO Remove after testing
    // echo "SKIPPING ASSAY DURING TEST\n";
    // return;

    echo "Processing SNPS assay into flat files...\n";
    // We need to create a file to write SNPS data to
    $dest_folder = tpps_realpath('public://tpps_vcf_flat_files');
    @mkdir($dest_folder);
    $snps_flat_file_location = $dest_folder . '/' . $accession . '-' . $i . '-snps.csv';
    // echo '[FILE_LOCATION][SNPs FLAT FILE CSV] ' . $dest_folder . '/' . $accession . '-' . $i . '-snps.csv' . "\n";
    $fhandle = fopen($snps_flat_file_location, 'w');
    $options['fhandle'] = $fhandle;

    $snp_fid = $genotype['files']['snps-assay'];
    $ref_genome = $genotype['ref-genome'];
    echo "Ref-genome: $ref_genome\n";
    $options['organism_index'] = $i;
    $options['species_codes'] = $species_codes;
    $options['type'] = 'snp';
    $options['headers'] = tpps_file_headers($snp_fid);
    $options['marker'] = 'SNP';
    $options['type_cvterm'] = tpps_load_cvterm('snp')->cvterm_id;
    $options['ref-genome'] = $genotype['ref-genome'];

    $options['records']['featureloc'] = array();
    $options['records']['featureprop'] = array();
    $options['records']['feature_relationship'] = array();
    $options['records']['feature_cvterm'] = array();
    $options['records']['feature_cvtermprop'] = array();

    $options['associations'] = array();
    $options['associations_tool'] = $genotype['files']['snps-association-tool'];
    $options['associations_groups'] = $genotype['files']['snps-association-groups'];
    $options['scaffold_cvterm'] = tpps_load_cvterm('scaffold')->cvterm_id;
    $options['phenotype_meta'] = $shared_state['data']['phenotype_meta'];
    $options['pub_id'] = $shared_state['ids']['pub_id'];
    $options['all_variants'] = [];

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

    // Lookup analysis id from reference genome and add it to options array
    $options['analysis_id'] = tpps_get_analysis_id_from_ref_genome($ref_genome);
    $time_start = time();
    $transaction = db_transaction();
    try {
      // Remove all accession tree_id markers before running tpps_file_iterator to insert back data
      echo "Remove all markers_and_study_accession_per_individual for accession $accession\n";
      chado_query("DELETE FROM chado.markers_and_study_accession_per_individual_tree WHERE accession = '$accession'");
      // Run the file_iterator which will populate flat files and also populate chado.markers_and_study_accession_per_individual_tree
      tpps_file_iterator($snp_fid, 'tpps_process_genotype_spreadsheet_flat_file', $options);
      tpps_log('[INFO] - Done.');

      tpps_log('[INFO] - Inserting SNP genotype_spreadsheet data into database using insert_multi...');
      tpps_chado_insert_multi($options['records'], $multi_insert_options);

      // Add all variants (markers) to the chado.studies_with_markers table
      // GOAL: Push unique variants to chado.studies_with_markers
      $unique_variant_ids_string = implode(',',array_keys($options['all_variant_ids']));

      // UNIQUE VARIANTS Step 1 - check to see if there's already a record
      $accession_count_results = chado_query('SELECT count(*) as c1 FROM chado.studies_with_markers WHERE accession ILIKE :accession', [
        ':accession' => $accession
      ]);
      $accession_count = $accession_count_results->fetchObject()->c1;

      if ($accession_count > 0) {
        // UPDATE
        echo "UPDATE chado.studies_with_markers for $accession\n";
        chado_query("UPDATE chado.studies_with_markers SET markers = ARRAY[" . $unique_variant_ids_string . "] WHERE accession ILIKE '" . $accession . "';");
      }
      else {
        // INSERT
        echo "INSERT chado.studies_with_markers for $accession\n";
        chado_query("insert into chado.studies_with_markers (accession, markers) values ('" . $accession . "', ARRAY[" . $unique_variant_ids_string . "]);");
      }
      // Clean up memory
      unset($unique_variant_ids_string);


    } catch (Exception $ex) {
      echo "An exception occurred.\n";
      print_r($ex);
      $transaction->rollback();
    }
    // tpps_log('[INFO] - Inserting SNP genotype_spreadsheet data into database using insert_hybrid...');
    // tpps_chado_insert_hybrid($options['records2'], $multi_insert_options);
    fclose($fhandle);
    tpps_log('[INFO] - Done');
    $time_end = time();
    echo "Time elapsed for SNP genotype spreadsheet processing: " . (($time_end - $time_start) / 60) . " mins.\n";
    echo '[FILE_LOCATION][SNPs FLAT FILE CSV] ' . $dest_folder . '/' . $accession . '-' . $i . '-snps.csv' . "\n";
  }
}

function tpps_process_genotype_spreadsheet_flat_file($row, array &$options = array()) {
  // print_r($row);
  global $tpps_job;
  $job = $tpps_job;
  $type = $options['type'];
  $organism_index = $options['organism_index'];
  $records = &$options['records'];
  $records2 = &$options['records2']; // hybrid / copy such as genotype_call records
  $headers = $options['headers'];
  $tree_info = &$options['tree_info'];
  $species_codes = $options['species_codes'];
  // print_r("Species codes\n");
  // print_r($species_codes);
  $genotype_count = &$options['genotype_count'];
  $project_id = $options['project_id'];
  $marker = $options['marker'];
  // print_r("row\n");
  // print_r($row);
  // print_r("tree_info\n");
  // print_r($tree_info);
  // print_r("\n");
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
  $tree_id = "";

  $keys = array_keys($row);
  $key_index = -1;
  $markers_array = [];
  foreach ($row as $key => $val) {
    $key_index++;
    // echo "ROW key:$key, val:$val\n";
    if (empty($headers[$key])) {
      continue;
    }


    // This $val is different from the $val later on
    // so order is important
    if (!isset($stock_id)) {
      $stock_id = $tree_info[trim($val)]['stock_id'];
      $current_id = $tree_info[trim($val)]['organism_id'];
      $species_code = $species_codes[$current_id];
      $tree_id = trim($val);
      echo "TREE_ID: $tree_id\n";
      continue;
    }
    $genotype_count++;

    echo "Stock ID: $stock_id, Current ID: $current_id, Genotype_count: $genotype_count\n";


    // echo "Header before alterations:" . $headers[$key] . "\n";

    $header_length = strlen($headers[$key]);
    // Cater for Diploids [Rish: 8/3/2023]
    if($options['ploidy'] == 'Diploid' && substr($headers[$key], $header_length - 2, 2) == "_A") {
      // Remove the _A from the first diploid header
      // and allow the below code to continue to be processed so the SSR can be imported in
      $headers[$key] = substr($headers[$key], 0, $header_length - 3);
      $options['diploid_header'] = $headers[$key];
      $options['diploid_val'] = $val;
      // Save this header for use in a later iteration when _B gets called
      // This reason for this is we want _A and _B values recorded
      echo "Diploid first header reset to: " . $headers[$key] . "\n";
      // This will skip processing iteration by ONE iteration if _A (SSR diploid detected)
      continue;
    }

    // [RISH] This is a minor adjustment for diploid done on 8/3/2023
    if($options['ploidy'] == 'Diploid' && substr($headers[$key], $header_length - 2, 2) == "_B") {
      $options['diploid_val'] .= ',' . $val;

      // Reset to these new values for insertion into the database later on
      $headers[$key] = $options['diploid_header'];
      $val = $options['diploid_val'];
      echo "Diploid val: $val\n";
    }
    // End of cater for diploids

    // Cater for Polyploids [RISH: 8/7/2023]
    // Get header without the trailing _X (_1,_2,_3 etc)
    $header_parts = explode("_", $headers[$key]);
    $header_parts_length = count($header_parts);
    $header_without_polyploid_index = "";
    for ($j = 0; $j < $header_parts_length - 1; $j++) {
      if ($j > 0) {
        $header_without_polyploid_index .= "_";
      }
      $header_without_polyploid_index .= $header_parts[$j];
    }

    if($options['ploidy'] == 'Polyploid' && $options['polyploid_header'] != $header_without_polyploid_index) {
      // Remove the _1 from the first diploid header
      // and allow the below code to continue to be processed so the SSR can be imported in
      $headers[$key] = $header_without_polyploid_index;
      $options['polyploid_header'] = $headers[$key];
      $options['polyploid_val'] = $val;
      // Save this header for use in a later iteration when _B gets called
      // This reason for this is we want _A and _B values recorded
      echo "Polyploid first header reset to: " . $headers[$key] . "\n";
      // This will skip processing iteration by ONE iteration if _1 (SSR diploid detected)
      continue;
    }

    // [RISH] This is a minor adjustment for polyploid done on 8/7/2023
    // Look forward to see if the next headers_key
    $header_next = $headers[$keys[$key_index + 1]];
    // Get next header without the trailing _X (_1,_2,_3 etc)
    $header_next_parts = explode("_", $header_next);
    $header_next_parts_length = count($header_next_parts);
    $header_next_without_polyploid_index = "";
    for ($j = 0; $j < $header_next_parts_length - 1; $j++) {
      if ($j > 0) {
        $header_next_without_polyploid_index .= "_";
      }
      $header_next_without_polyploid_index .= $header_next_parts[$j];
    }

    if($options['ploidy'] == 'Polyploid' && $options['polyploid_header'] == $header_without_polyploid_index) {
      $options['polyploid_val'] .= ',' . $val; // append the new value to what was already there

      // Check if the next header does not match current header (this would mean next header starts a new SSR polyploid) OR
      // if the next header is NULL (this means end of headers of the file)
      // so we need to allow the rest of code below to happen to insert this current SRR polyploid
      if (($header_without_polyploid_index != $header_next_without_polyploid_index) || $header_next == NULL) { // NULL happens if end of all headers
        // we have found that $headers[$key] is the last polyploid column for the current SSR
        // so reset to these new values for insertion into the database later on
        $headers[$key] = $options['polyploid_header'];
        $val = $options['polyploid_val'];
        echo "Polyploid val: $val for insertion using SSR marker" . $headers[$key] . "\n";
      }
      else {
        echo "Skipping insert\n";
        continue; // this will skip insertion (below code) until all values for the current SSR polyploid is found
      }
    }
    // End of catering for polyploids
    echo "Processing the insert\n";

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

    // This will add a non NA marker to markers_array which will get pushed to the database further
    // down in code
    if ($val != 'NA') {
      $markers_array[] = "'" . $variant_name . "'";
    }

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
    ob_start();
    chado_insert_record('feature', [
      'name' => $marker_name,
      'organism_id' => $organism_id,
      'uniquename' => $marker_name,
      'type_id' => $seq_var_cvterm,
    ]);
    ob_end_clean();
    // Lookup the marker_name_id.
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

    ob_start();
    chado_insert_record('feature', [
      'name' => $variant_name,
      'organism_id' => $organism_id,
      'uniquename' => $variant_name,
      'type_id' => $seq_var_cvterm
    ]);
    ob_end_clean();


    // Lookup the marker_name_id
    $results = chado_query("SELECT feature_id FROM chado.feature
      WHERE uniquename = :uniquename AND organism_id = :organism_id", [
        ':uniquename' => $variant_name,
        ':organism_id' => $organism_id
    ]);

    // echo "variant_name: $variant_name\n";
    $variant_name_id = NULL;
    foreach ($results as $row) {
      $variant_name_id = $row->feature_id;
    }


    // this will at the end of everything be used to insert / update studies_with_markers
    $options['all_variant_ids'][$variant_name_id] = 1;


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
    // if ($vcf_processing_completed == true && $type == 'snp') {
    //   //skip performing genotype and genotype_call inserts
    // }
    // else {
      // [RISH] 07/06/2023 - REMOVED SO WE CAN INSERT TO GET ID
      // $records['genotype'][$genotype_name] = array(
      //   'name' => $genotype_name,
      //   'uniquename' => $genotype_name,
      //   'description' => $val,
      //   'type_id' => $type_cvterm,
      // );
      ob_start();
      chado_insert_record('genotype', [
        'name' => $genotype_name_without_call,
        'uniquename' => $genotype_name,
        'description' => $val,
        'type_id' => $type_cvterm,
      ]);
      ob_end_clean();
      // echo "name: $genotype_name_without_call, uniquename: $genotype_name, $val, $type_cvterm\n";


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

      // RISH - 12/18/2023 - Requested by Emily
      ob_start();
      chado_insert_record('feature_genotype', [
        'feature_id' => $variant_name_id,
        'genotype_id' => $genotype_id,
        'chromosome_id' => NULL,
        'rank' => 0,
        'cgroup' => 0,
        'cvterm_id' => $type_cvterm,
      ]);
      // echo "feature_id: $variant_name_id, genotype_id: $genotype_id\n";
      ob_end_clean();

      fwrite($options['fhandle'], "$tree_id,$variant_name\n");


      // echo "Genotype_call key: $stock_id-$genotype_name\n";
      // if (isset($records2['genotype_call']["$stock_id-$genotype_name"])) {
      //   echo "This genotype_call key is already set (so uniqueness is maybe broken?\n";
      // }
      // $records2['genotype_call']["$stock_id-$genotype_name"] = array(
      //   'project_id' => $project_id,
      //   'stock_id' => $stock_id,
      //   'genotype_id' => $genotype_id,
      //   'variant_id' => $variant_name_id,
      //   'marker_id' => $marker_name_id,
      // );

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
    // }

    if ($genotype_count >= $record_group) {
      if ($vcf_processing_completed == TRUE && $type == 'snp') {
        tpps_log('[INFO] - Skipped genotype and genotype_call SNPs since VCF already loaded...');
      }
      tpps_log('[INFO] - Inserting data into database using insert_multi...');
      tpps_chado_insert_multi($records, $multi_insert_options);
      // tpps_log('[INFO] - Inserting data into database using insert_hybrid...');
      // tpps_chado_insert_hybrid($records2, $multi_insert_options);
      tpps_log('[INFO] - Done.');
      $records = array(
        'feature' => array(),
        'genotype' => array(),
        'genotype_call' => array(),
        'stock_genotype' => array(),
      );
      // Do this for the hybrid (COPY) command.
      $records2 = array(
        'genotype_call' => array(),
      );
      if (!empty($associations)) {
        $records['featureloc'] = array();
        $records['featureprop'] = array();
      }
      $options['genotype_total'] += $genotype_count;
      tpps_log('[INFO] - Genotypes inserted:' . $options['genotype_total']);
      $genotype_count = 0;
    }
  }

  $accession = $options['accession'];
  $markers_array_count = count($markers_array);
  echo "Found $markers_array_count markers for $tree_id\n";
  if ($markers_array_count > 0) {
    // Check whether table already contains
    echo "Inserting markers for $accession TREE_ID $accession-$tree_id\n";
    chado_query("INSERT INTO chado.markers_and_study_accession_per_individual_tree (accession, tree_id, markers)
      VALUES (
        '$accession','$accession-$tree_id', ARRAY[" . implode(',',$markers_array) . "]
      )
    ");
    echo "Insert successful\n";
  }
}

/**
 * Tpps genotype_vcf_processing.
 *
 * This function will process a vcf file's genotypic information
 * and store it within the db. Most importantly is the $import_mode option
 * which can be set to 'multi' or 'hybrid'.
 * Hybrid requires the the db user be granted SUPERUSER access
 * to the Postgresql db in order to utilize the COPY keyword for
 * inserting data which is dramatically faster.
 *
 * @param array $form_state
 * @param array $species_codes
 * @param mixed $i
 * @param TripalJob $job
 * @param string $insert_mode
 * @access public
 *
 * @return void
 */
function tpps_genotype_vcf_processing(array &$form_state, array $species_codes, $i, TripalJob &$job = NULL, $insert_mode = 'hybrid', array &$options) {
  $organism_index = $i;
  // Some initial variables previously inherited from the parent function code. So we're reusing it to avoid
  // missing any important variables if we rewrote it.
  $page1_values = $form_state['saved_values'][TPPS_PAGE_1];
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $page4_values["organism-$i"]['genotype'] ?? NULL;

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
    'study_accession' => $form_state['saved_values'][1]['accession']
  );

  // check to make sure admin has not set disable_vcf_importing.
  $disable_vcf_import = 0;
  if (isset($page1_values['disable_vcf_import'])) {
    $disable_vcf_import = $page1_values['disable_vcf_import'];
  }
  tpps_job_logger_write('[INFO] Disable VCF Import is set to ' . $disable_vcf_import . ' (0 means allow vcf import, 1 ignore vcf import)');

  if ($genotype['files']['file-type'] == TPPS_GENOTYPING_FILE_TYPE_VCF) {
    if ($disable_vcf_import == 0) {
      // tpps_drop_genotype_call_indexes($job);

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
      $tree_ids = array();
      $format = "";

      // This was done by Peter
      // DEPRECATED 8/13/2024
      // $current_id = $form_state['ids']['organism_ids'][$i];
      // $species_code = $species_codes[$current_id];


      // Override the above code done by Rish to use organism_id from page 4
      // RISH NOTES: This addition uses the organism_id based on the organism order
      // of the fourth page (we likely have to pass the i from previous function here)
      // THIS TECHNICALLY OVERRIDES PETER'S LOGIC ABOVE. TO BE DETERMINED IF RISH'S WAY IS CORRECT
      // OR NOT [7/1/2023]
      // THIS WAS AN ISSUE BROUGHT UP BY EMILY REGARDING SNPS NOT BEING ASSOCIATED WITH POP TRICH (665 STUDY)
      $species_code = null;
      $current_id = null;
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
        // DEPRECATED 8/12/2024 in favour of chado.tpps_ref_assembly_view created by Emily
        // // Get the species and version from the reference genome selected
        // // if match occurs thats in index [0].
        // // The group match index [1] is species, group match index [2] is version
        // preg_match('/(.+) +v(\d*\.*\d*)/', $ref_genome, $matches);
        // $ref_genome_species = NULL;
        // $ref_genome_version = NULL;
        // if (count($matches) > 0) {
        //   $ref_genome_species = $matches[1];
        //   $ref_genome_version = $matches[2];
        // }

        // if (isset($ref_genome_species) && isset($ref_genome_version)) {
        //   // Look up the analysis
        //   $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
        //     WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
        //     [
        //       ':name' => $ref_genome_species . '%',
        //       ':programversion' => $ref_genome_version
        //     ]
        //   );
        //   foreach ($analysis_results as $row) {
        //     $analysis_id = $row->analysis_id;
        //   }
        // }

        // if($analysis_id == NULL) {
        //   // Look up the analysis
        //   $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
        //     WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
        //     [
        //       ':name' => $ref_genome_species . '%',
        //       ':programversion' => 'v' . $ref_genome_version
        //     ]
        //   );
        //   foreach ($analysis_results as $row) {
        //     print_r("analysis_row\n");
        //     print_r($row);
        //     print_r("\n");
        //     $analysis_id = $row->analysis_id;
        //   }
        // }

        // // If an analysis_id still was not found, it's possibly from the db data source
        // // instead of the genome directory. The genome directory code is in page_4*.php
        // // New code to cater for new analysis checks via db - query given by Emily Grau (6/6/2023)
        // if ($analysis_id == NULL) {
        //   $genome_query_results = chado_query("select * from chado.tpps_ref_assembly_view WHERE name LIKE :ref_genome;", [
        //     ':ref_genome' => $ref_genome
        //   ]);
        //   foreach ($genome_query_results as $genome_query_row) {
        //     $analysis_id = $genome_query_row->analysis_id;
        //   }
        // }

        $analysis_id = tpps_get_analysis_id_from_ref_genome($ref_genome);

        // Once an analysis_id was found, try to get srcfeature_id

      }
      else {
       $error_line = [
          "A reference genome could not be found in the TPPS page 4 form.",
          "Without this, we cannot find the analysis_id and thus the srcfeature_id.",
          "Featureloc data will not be recorded",
        ];
        tpps_log('[REF GENOME NOT FOUND] - ' . implode("\n", $error_line) . "\n");
      }

      tpps_log("Analysis ID Found: $analysis_id\n");
      print_r("Analysis ID Found: $analysis_id\n");
      // throw new Exception("DEBUG");

      echo "[INFO] Processing Genotype VCF file\n";
      $file_progress_line_count = 0;
      $record_count = 0;
      while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
        $file_progress_line_count++;
        if($file_progress_line_count % 10000 == 0 && $file_progress_line_count != 0) {
          echo '[INFO] [VCF PROCESSING STATUS] ' . $file_progress_line_count . " lines done\n";
        }
        if ($vcf_line[0] != '#'
          && stripos($vcf_line,'.vcf') === FALSE
          && trim($vcf_line) != ""
          && str_replace("\0", "", $vcf_line) != ""
        ) {
          $line_process_start_time = microtime(true);
          $record_count = $record_count + 1;
          // DEBUG - TEST ONLY 100 records
          // if ($record_count > 100) {
          //   break;
          // }
          print_r('Record count:' . $record_count . "\n");
          $genotype_count += count($stocks);
          $vcf_line = explode("\t", $vcf_line);
          $scaffold_id = explode(" ",$vcf_line[0])[0]; // take the first part if it is space delimited (8/13/2024 RISH discussion with Emily)
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
          // Emily updated suggestion on Tuesday August 9th 2022
          $marker_name = $scaffold_id . '_' . $position . '-' . $species_code;

          // $description = "$ref:$alt"; // Replaced with genotype_combination within $detected_genotypes array (5/31/2023)

          // $genotype_name = "$marker-$species_code-$scaffold_id-$position"; // Original by Peter

          // Instead, we have multiple genotypes we need to generate, so lets do a key val array
          $detected_genotypes = array();
          $first_genotypes = array(); // used to save the first genotype in each row of the VCF (used for genotype_call table)
          $count_columns = count($vcf_line);
          for ($j = 9; $j < $count_columns; $j++) {

            $j_column_data = $vcf_line[$j];
            // @TODO We need to cater for extra metadata eg. 1/1:0,98:98:99:3055,289,0 <-- the data after the : is metadata
            $j_read = explode(':',$j_column_data)[0]; // gets the 1/1 part

            $genotype_combination = tpps_submit_vcf_render_genotype_combination($j_read, $ref, $alt); // eg AG (removed the : part of code on 5/31/2023)

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

          // Check to see if marker exists in the feature's table
          $feature_check_results = chado_query('SELECT count(*) as c1 FROM chado.feature WHERE uniquename = :marker_name',[
            ':marker_name' => $marker_name
          ]);
          $feature_check_count = $feature_check_results->fetchObject()->c1;
          // If marker does not exist, insert it into the feature table
          if ($feature_check_count <= 0) {
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
          }
          // get the marker_id <- feature_id column value
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

          // Check if variant_id already exists in the feature's table
          $feature_check_results = chado_query('SELECT count(*) as c1 FROM chado.feature WHERE uniquename = :variant_name',[
            ':variant_name' => $variant_name
          ]);
          $feature_check_count = $feature_check_results->fetchObject()->c1;
          // If variant does not exist, add it to the feature table
          $feature_exists = NULL;
          if ($feature_check_count <= 0) {
            $feature_exists = false; // remember that it didn't exist at first (this is used further down to add featureloc)
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
          }
          else {
            $feature_exists = true; // remember it already exists (this is used further down to block featureloc)
          }

          // get the feature_id
          $results = chado_query('SELECT feature_id FROM chado.feature WHERE uniquename = :uniquename', [
            ':uniquename' => $variant_name
          ]);
          $row_object = $results->fetchObject();
          $variant_id = $row_object->feature_id;

          // // Lookup whether marker is already inserted into the features table.
          // $result = chado_query(
          //   "SELECT * FROM chado.feature WHERE uniquename = :marker_name",
          //   // Column 3 of VCF.
          //   [':marker_name' => $variant_name]
          // );

          if ($feature_exists) {
            tpps_log("Feature (variant): $variant_name exists already, featureloc will not be added\n");
            echo("Feature (variant): $variant_name exists, featureloc will not be added\n");
          }
          else {
            tpps_log("Feature (variant): $variant_name not found so it was created\n");
            echo("Feature (variant): $variant_name not found so it was created\n");
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

            if ($srcfeature_id) {
              tpps_log("SRC Feature: $srcfeature_id found\n");
              echo("SRC Feature: $srcfeature_id exists\n");
            }
            else {
              tpps_log("SRC Feature not found\n");
              echo("SRC Feature not found\n");
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
                'feature_id' => $variant_id,
                'srcfeature_id' => $srcfeature_id,
                'fmin' => $position,
                'fmax' => $fmax // ALPHA code above now caters for INDELS
              ];

              // Since we haven't catered for deletion of these featureloc records
              // there may already exist, we have to make sure the record doesn't already exist
              $featureloc_results = chado_query('SELECT count(*) as c1 FROM chado.featureloc
                WHERE feature_id = :feature_id AND srcfeature_id = :srcfeature_id;', [
                  ':feature_id' => $variant_id,
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
                // The marker_name doesn't mean much here because it is only used a key
                $records['featureloc'][$variant_name] = $featureloc_values;
                echo "Featureloc for $variant_name will be created\n";
              }
            }
          }
          // throw New Exception('DEBUG');

          // Rish 12/08/2022: So we have multiple genotypes created
          // So I adjusted some of this code into a for statement
          // since the genotype_desc seems important and so I modified
          // it to be unique and based on the genotype_name.
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
            // get the feature_id.
            $results = chado_query('SELECT genotype_id FROM chado.genotype WHERE uniquename = :uniquename', [
              ':uniquename' => $genotype_desc
            ]);
            $row_object = $results->fetchObject();
            $genotype_id = $row_object->genotype_id;
            // $debug_info = "Uniquename: $genotype_desc Type_id:$format_cvterm Value:$format Genotype_id:$genotype_id Variant_id:$variant_id Marker_id:$marker_id\n";
            // $debug_info = "Variant_name: $variant_name, Variant_id: $variant_id\n";
            // echo("DEBUG INFO: $debug_info");

            // 3/27/2023 Meeting - FORMAT: REVIEW THIS IN TERMS OF IF WE NEED IT.
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

              $j_column_data = $vcf_line[$j];
              // @TODO We need to cater for extra metadata eg. 1/1:0,98:98:99:3055,289,0 <-- the data after the : is metadata
              $j_read = explode(':',$j_column_data)[0]; // gets the 1/1 part

              $val_combination = tpps_submit_vcf_render_genotype_combination($j_read, $ref, $alt);
              $column_genotype_name = $marker_type . '-' . $marker_name . '-' . $val_combination;
              // echo 'Column Genotype Name: ' . $column_genotype_name . " Genotype Name: $genotype_name\n";
              if($column_genotype_name == $genotype_name) {

                // [RISH] 02/26/2024
                // Insert genotype reads into chado.genotype_reads_per_plant
                // We need plant name, study, marker_name
                // $options['tree_id'], $options['study_accession'], $marker_name
                // Check if a record already exists, if not, create initial record

                $study_accession = $options['study_accession'];
                $tree_id = $study_accession . '-' . $tree_ids[$j - 9];
                $per_plant_results = chado_query('
                  SELECT COUNT(*) as c1 FROM chado.genotype_reads_per_plant
                  WHERE tree_acc = :tree_id AND study_accession = :study_accession
                ', [
                  ':tree_id' => $tree_id,
                  ':study_accession' => $study_accession
                ]);
                $per_plant_records_count = $per_plant_results->fetchObject()->c1;
                if ($per_plant_records_count == 0) {
                  // CREATE AN EMPTY RECORD IN TABLE
                  chado_query("
                    INSERT INTO chado.genotype_reads_per_plant
                    (tree_acc, study_accession, marker_array, read_array)
                    VALUES
                    ('$tree_id', '$study_accession', ARRAY[]::text[], ARRAY[]::text[])
                  ");
                }
                // So now we have a record in the table for the plant, so append the new values
                chado_query("
                  UPDATE chado.genotype_reads_per_plant
                  set marker_array = array_append(marker_array, '$marker_name')
                  WHERE tree_acc = '$tree_id' AND study_accession = '$study_accession'
                ");
                chado_query("
                  UPDATE chado.genotype_reads_per_plant
                  set read_array = array_append(read_array, '$val_combination')
                  WHERE tree_acc = '$tree_id' AND study_accession = '$study_accession'
                ");

                // Found a match between the tree_id genotype and the genotype_name from records
                // echo "Found match (and using variant_name $variant_name ($variant_id) to add to genotype call\n";

                // print_r('[genotype_call insert]: ' . "{$stocks[$j - 9]}-$genotype_name" . "\n");
                // $records['genotype_call']["{$stocks[$j - 9]}-$genotype_name"] = array(
                //   'project_id' => $project_id,
                //   'stock_id' => $stocks[$j - 9],
                //   // PETER
                //   // '#fk' => array(
                //   //   'genotype' => $genotype_desc,
                //   //   'variant' => $variant_name,
                //   //   'marker' => $marker_name,
                //   // ),
                //   // RISH
                //   'genotype_id' => $genotype_id,
                //   'variant_id' => $variant_id,
                //   'marker_id' => $marker_id,
                // );

                // It is in use for genotype materialized views and Emily's function to generate tables
                // which is used for tpps/details page
                $records['stock_genotype']["{$stocks[$j - 9]}-$genotype_name"] = array(
                  'stock_id' => $stocks[$j - 9],
                  // PETER
                  // '#fk' => array(
                  //   'genotype' => $genotype_desc,
                  // ),
                  // RISH
                  'genotype_id' => $genotype_id,
                );


                // chado_insert_record('feature_genotype', [
                //   'feature_id' => $variant_id,
                //   'genotype_id' => $genotype_id,
                //   'chromosome_id' => NULL,
                //   'rank' => 0,
                //   'cgroup' => 0,
                //   'cvterm_id' => $snp_cvterm,
                // ]);

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
            tpps_log('[INFO] - Last bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            tpps_log('[INFO] - Last insert cumulative time: ' . $insert_cumulative_time . ' seconds');
            $genotype_count = 0;
            $insert_start_time = microtime(true);
            if ($insert_mode == 'multi') {
              tpps_log('[INFO] - Inserting data into database using insert_multi...');
              tpps_chado_insert_multi($records, $multi_insert_options);
            }
            else if ($insert_mode == 'hybrid') {
              // THIS WILL DO SOME MULTI INSERTS BUT ALSO USE COPY FOR GENOTYPE_CALL TABLE
              tpps_log('[INFO] - Inserting data into database using insert_hybrid...');
              tpps_chado_insert_hybrid($records, $multi_insert_options);
            }

            tpps_log('[INFO] - Done.');
            $insert_end_time = microtime(true);
            $insert_elapsed_time = $insert_end_time - $insert_start_time;
            tpps_log('[INFO] - Bulk insert of ' . $record_group . ' took ' . $insert_elapsed_time . ' seconds');
            if(!isset($insert_cumulative_time)) {
              $insert_cumulative_time = 0;
            }
            $insert_cumulative_time += $insert_elapsed_time;
            tpps_log('[INFO] - Insert cumulative time: ' . $insert_cumulative_time . ' seconds');
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
            $tree_ids[] = trim($vcf_line[$j]);
          }
        }
      }
      // Insert the last set of values.
      if ($insert_mode == 'multi') {
        tpps_log('[INFO] - Inserting data into database using insert_multi...');
        tpps_chado_insert_multi($records, $multi_insert_options);
      }
      elseif ($insert_mode == 'hybrid') {
        tpps_log('[INFO] - Inserting data into database using insert_hybrid...');
        tpps_chado_insert_hybrid($records, $multi_insert_options);
      }

      // Recreate the indexes.
      // tpps_create_genotype_call_indexes();
      tpps_log('[INFO] - Done.');
      unset($records);
      $genotype_count = 0;
    }
  }
}

/**
 * Generates genotype sample file from VCF.
 *
 * @param mixed $options
 *   Array of options. Keys are: study_accession, form_state, job.
 */
function tpps_generate_genotype_sample_file_from_vcf($options = NULL) {
  // If study accession value exists, use this to look up the form_state.
  // $form_state = NULL;
  $shared_state = NULL;
  if (isset($options['study_accession'])) {
    $submission = new Submission($options['study_accession']);
    $form_state = $submission->state;
    $shared_state = $submission->getSharedState();
  }
  // elseif (isset($options['form_state'])) {
  //   $form_state = $options['form_state'];
  // }

  // If $form_state is not NULL.
  if (isset($shared_state)) {
    // Get page 1 form_state data.
    $page1_values = $shared_state['saved_values'][TPPS_PAGE_1];
    // Get page 4 form_state data.
    $page4_values = $shared_state['saved_values'][TPPS_PAGE_4];
    // Organism count.
    $organism_number = $shared_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
    // Project ID.
    $project_id = $shared_state['ids']['project_id'];

    // Go through each organism.
    for ($i = 1; $i <= $organism_number; $i++) {
      $organism_name = $page1_values['organism'][$i]['name'];
      echo "Organism name: $organism_name\n";
      $genotype = $page4_values["organism-$i"]['genotype'] ?? NULL;
      // Note: Value of this field is string (not array). Correct check:
      //if ($genotype['files']['file-type'] == TPPS_GENOTYPING_FILE_TYPE_VCF) {
      if (empty($genotype['files']['file-type']['VCF'])) {
        echo "Could not find a VCF file for organism-$i\n";
      }
      else {

        // Initialize sample.list text.
        $sample_list_data = "VCF_header_sample\tSample_name\tSample_Accession\tGermplasm_name\tGermplasm_Accession\tGermplasm_type\tOrganism\n"; // header

        // Get the VCF fid.
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
        $file_name = $shared_state['accession'] . '-sample-list-' . $i . '.txt';
        $file = file_save_data($sample_list_data, $dest_folder . $file_name);
        echo "File managed as FID: " . $file->fid . "\n";
        echo "File managed location: " . $file->uri . "\n";
        echo "Real managed real path: " . tpps_realpath($file->uri) . "\n";
        // We could store this in the submit_state - TODO if we need this
        // $form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['genotype']['vcf_sample_list'] = $file->fid;
        // print_r($sample_list_data);
      } // end else
    } // end for
  }
  else {
    echo "Could not find a shared_state\n";
  }
}

/**
 * TPPS Generate Population Structure
 * FastStructure requires pip install pip==9.0.1 to install dependencies
 */

 // drush php-eval 'include("/var/www/Drupal/sites/all/modules/TGDR/forms/submit/submit_all.php"); tpps_generate_popstruct("TGDR675", "/var/www/Drupal/sites/default/files/popstruct_temp/Panel4SNPv3.vcf");'
// @TODO Could tpps_log() be used in this function instead of tpps_job_logger_write()?
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
  $public_path = tpps_realpath($path);
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
      $insert_sql .= "'" . $study_accession ."',". $tree_data[$tree_id]['latitude'] ."," . $tree_data[$tree_id]['longitude'] . "";
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
  tpps_log('[INFO] - Submitting environment data...');
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4];
  $environment = $page4_values["organism-$i"]['environment'] ?? NULL;
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
    tpps_log('[INFO] - Processing environment_layers file data...');
    tpps_file_iterator($tree_acc_fid, 'tpps_process_environment_layers', $options);
    tpps_log('[INFO] - Done.');

    tpps_log('[INFO] - Inserting data into database using insert_multi...');
    tpps_chado_insert_multi($options['records']);
    tpps_log('[INFO] - Done.');
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
  if (
    !empty($columns['struct'])
    and isset($row[$columns['struct']])
    and $row[$columns['struct']] != ''
  ) {
    $meta[$name]['struct'] = 'other';
    $meta[$name]['struct-other'] = $row[$columns['struct']];
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
  // tpps_log('Meta array');
  // tpps_log(print_r($meta, true));
  // tpps_log("\n");
  $cvt_cache = [];
  $local_cv = chado_get_cv(['name' => 'local']);
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
          if (!empty($data["{$type}-other"])) {
            $result = tpps_ols_install_term("{$info['ontology']}:{$data["{$type}-other"]}");
            if ($result !== FALSE) {
              $meta[$name]["{$type}_id"] = $result->cvterm_id;
              $job->logMessage(
                "[INFO] New OLS Term '{$info['ontology']}:{$data["{$type}-other"]}' installed"
              );
            }
          }

          if (empty($meta[$name]["{$type}_id"]) && !empty($data["{$type}-other"])) {
            $term = chado_select_record('cvterm', ['cvterm_id'],
              ['name' => ['data' => $data["{$type}-other"], 'op' => 'LIKE']],
              ['limit' => 1]
            );
            $meta[$name]["{$type}_id"] = current($term)->cvterm_id ?? NULL;
          }

          // [VS] Create new CVTerm for new (custom) unit from Metafile.
          if (empty($meta[$name]["{$type}_id"])) {
            if (empty($data["{$type}-other"])) {
              // Usually it will be 'other-other'.
              // @todo Check empty units in metafile on validation stage.
              $cvterm_name = 'no unit';
            }
            else {
              $cvterm_name = $data["{$type}-other"];
            }
            $cvterm_row_values = [
              'id' => "{$local_db->name}:{$data["{$type}-other"]}",
              'name' => $cvterm_name,
              'definition' => '',
              'cv_name' => $local_cv->name,
            ];
            tpps_log("[CREATING] New CVTERM for custom unit FROM metafile\n");
            tpps_log(print_r($cvterm_row_values, true));

            $meta[$name]["{$type}_id"] = chado_insert_cvterm($cvterm_row_values)->cvterm_id;
            tpps_log('CVTERM ID (unit_cvterm_id): ' . $meta[$name]["{$type}_id"]);
            tpps_log("\n");
            if (!empty($meta[$name]["{$type}_id"])) {
              if ($cvterm_name == 'no unit') {
                // 'other-other'.
                $job->logMessage("[INFO] Used Local '{$info['label']}' Term '{$cvterm_name}'.");
              }
              else {
                $job->logMessage("[INFO] New Local '{$info['label']}' Term '{$cvterm_name}' installed");
              }
            }
          }
          // [/VS]
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

    // When Metadata File was used we have no Synonym Id in $meta
    // so we need to get Synonym Id by Unit Id.
    // WARNING: Must be called when $meta[$name]['unit_id'] already set.
    //
    // @todo Minor. Implement ability for admins/curation team to set
    // synonym on Phenotype edit form when Metadata File is used.
    // List of synonyms must be limited by Unit from Metadata File.
    if (empty($meta[$name]['synonym_id']) && !empty($meta[$name]['unit_id'])) {
      // Note: Unit Id could belong to many Synonyms and we are
      // using 1st Synonym Id from the list.
      if ($synonym_id = tpps_unit_get_synonym($meta[$name]['unit_id'])) {
        $meta[$name]['synonym_id'] = $synonym_id;
      }
      else {
        $message = t(
         '[WARNING] Unit #@unit_id has no phenotype synonym. @raw',
         [
          '@unit_id' => $meta[$name]['unit_id'],
          '@raw' => print_r($meta[$name], true),
         ]
        );
        tpps_log($message);
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
function tpps_process_phenotype_data($row, array &$options = []) {
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

  // Get genus and species from the organism name.
  $organism_name_parts = explode(' ', $organism_name, 2);
  $genus = $organism_name_parts[0];
  $species = $organism_name_parts[1];

  // Ensure that we got the genus and species or error out.
  if ($genus == "" || $species == "") {
    throw new Exception('Organism genus and species could not be processed. '
      . 'Please ensure you added an organism that exists within the chado.organism table!');
  }

  // Query the organism table to get the organism id.
  $organism_id_results = chado_query(
    'SELECT * FROM chado.organism
    WHERE genus = :genus
      AND species = :species
    ORDER BY organism_id ASC
    LIMIT 1',
    [':genus' => $genus, ':species' => $species]
  );

  // Dummy value for organism_id until we get it from the sql results row.
  $organism_id = -1;
  foreach ($organism_id_results as $organism_id_row) {
    $organism_id = $organism_id_row->organism_id;
  }

  // Check that the organism id is valid.
  if ($organism_id == -1 || $organism_id == "") {
    throw new Exception('Could not find organism id for ' . $organism_name
      . '. This organism does not seem to exist in the chado.organism table!');
  }

  $cvterm_id_4lettercode = -1;
  // Get the cvterm_id (which is the type_id) for the organism 4 letter code.
  $cvterm_results = chado_query(
    'SELECT * FROM chado.cvterm WHERE name = :name LIMIT 1',
    [':name' => 'organism 4 letter code']
  );
  foreach ($cvterm_results as $cvterm_row) {
    $cvterm_id_4lettercode = $cvterm_row->cvterm_id;
  }
  if ($cvterm_id_4lettercode == -1 || $cvterm_id_4lettercode == "") {
    throw new Exception('Could not find the cvterm id for organism '
      . '4 letter code within the chado.cvterm table. '
      . 'This is needed to generate the phenotype name.');
  }

  // We need to use the cvterm_id 4 letter code to find the actual code
  // within the organismprop table (using the organism_id).
  $value_4lettercode = "";
  $organismprop_results = chado_query('
    SELECT *
    FROM chado.organismprop
    WHERE type_id = :type_id
      AND organism_id = :organism_id
    LIMIT 1',
    [':type_id' => $cvterm_id_4lettercode, ':organism_id' => $organism_id]
  );
  foreach ($organismprop_results as $organismprop_row) {
    $value_4lettercode = $organismprop_row->value;
  }

  if ($value_4lettercode == "" || $value_4lettercode == NULL) {
    throw new Exception('4 letter code could not be found for '
      . $organism_name . ' in the chado.organismprop table. '
      . 'This is needed to create the phenotype_name.');
  }

  if ($iso) {
    // 'Iso/Mass Spectrometry'.
    foreach ($row as $id => $value) {
      if (empty($tree_id)) {
        $tree_id = $value;
        continue;
      }
      $values[$id] = $file_headers[$id];
    }
  }
  else {
    // 'Normal Check'.
    if (isset($meta_headers['name']) and (isset($meta_headers['value']))) {
      $id = $row[$meta_headers['value']];
      $values = [$id => $row[$meta_headers['name']]];
    }
    if (!empty($options['data_columns'])) {
      $values = $options['data_columns'];
    }
    $tree_id = $row[$options['tree_id']];
    $clone_col = $meta_headers['clone'] ?? NULL;
    if (isset($clone_col)
      and !empty($row[$clone_col])
      and $row[$clone_col] !== $empty
    ) {
      $tree_id .= "-" . $row[$clone_col];
    }
  }

  if ($tree_id == NULL || $tree_id == "") {
    throw new Exception('tree_id was NULL or empty - there might be '
      . 'a problem with the format of the phenotype data file or '
      . 'selected column options for the file via the user information, '
      . 'cannot continue until resolved.'
    );
  }
  $phenotype_name_previous = "<none set>";
  foreach ($values as $id => $name) {
    // $name is a phenotype name. For example: 'flower color'.
    // $id is column name. For example: 'D'.
    if ($name == NULL || $name == "") {
      throw new Exception('Phenotype name was NULL or empty - there might be '
        . 'a problem with the format of the phenotype data file or '
        . 'selected column options for the file via the user information, '
        . 'cannot continue until resolved.'
      );
    }
    $attr_id = $iso ? $meta['attr_id'] : $meta[strtolower($name)]['attr_id'];
    if ($attr_id == NULL || $attr_id == "") {
      print_r('$meta[attr_id]:' . $meta['attr_id'] . "\n");
      print_r('$name:' . $name . "\n");
      print_r('$meta[$name]:' . $meta[strtolower($name)]['attr_id'] . "\n");
      print_r('$attr_id:' . $attr_id . "\n");
      throw new Exception('Attribute id is NULL which causes phenotype '
        . 'data to not be added to database correctly.');
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
    elseif ($iso) {
      // Override the value - likely for intensity / mass spectrometry.
      $struct_id = $meta['struct_id'];
    }

    $phenotype_row_data = array(
      'uniquename' => $phenotype_name,
      'name' => $name,
      'attr_id' => $attr_id,
      // Removed this old obserable_id to cater for mass spectrometry.
      // 'observable_id' => $meta[strtolower($name)]['struct_id'] ?? NULL,
      // this is the new way of adding observable_id to cater for mass spectrometry as well.
      'observable_id' => $struct_id,
      'value' => $value,
    );
    // tpps_log("Phenotype row data to be inserted\n");
    // tpps_log(print_r($phenotype_row_data, true));
    // tpps_log("\n");
    $records['phenotype'][$phenotype_name] = $phenotype_row_data;
    $records['stock_phenotype'][$phenotype_name] = array(
      'stock_id' => $tree_info[$tree_id]['stock_id'],
      '#fk' => ['phenotype' => $phenotype_name],
    );
    if (isset($meta[strtolower($name)]['time'])) {
      $records['phenotypeprop']["$phenotype_name-time"] = array(
        'type_id' => $cvterms['time'],
        'value' => $meta[strtolower($name)]['time'],
        '#fk' => ['phenotype' => $phenotype_name],
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
        '#fk' => ['phenotype' => $phenotype_name],
      );
      $options['data'][$phenotype_name]['time'] = $val;
    }
    $records['phenotypeprop']["$phenotype_name-desc"] = array(
      'type_id' => $cvterms['desc'],
      'value' => $iso ? $meta['desc'] : $meta[strtolower($name)]['desc'],
      '#fk' => ['phenotype' => $phenotype_name],
    );
    // $iso means "intensity / mass spectrometry".
    if ($iso) {
      // "Iso Check"
      $records['phenotypeprop']["$phenotype_name-unit"] = [
        'type_id' => 139527,
        // value: the chemical name/identifier.
        'value' => $meta['unit'],
        '#fk' => ['phenotype' => $phenotype_name],
      ];
    }
    else {
      // "Normal Check"
      $records['phenotype_cvterm']["$phenotype_name-unit"] = [
        'cvterm_id' => $meta[strtolower($name)]['unit_id'],
        '#fk' => ['phenotype' => $phenotype_name],
      ];
    }
    // @todo Remove 'min' and 'max' processing.
    // Min and max not used anymore.
    if (isset($meta[strtolower($name)]['min'])) {
      $records['phenotypeprop']["$phenotype_name-min"] = [
        'type_id' => $cvterms['min'],
        'value' => $meta[strtolower($name)]['min'],
        '#fk' => ['phenotype' => $phenotype_name],
      ];
    }
    if (isset($meta[strtolower($name)]['max'])) {
      $records['phenotypeprop']["$phenotype_name-max"] = [
        'type_id' => $cvterms['max'],
        'value' => $meta[strtolower($name)]['max'],
        '#fk' => ['phenotype' => $phenotype_name],
      ];
    }
    if (!empty($meta[strtolower($name)]['env'])) {
      $records['phenotype_cvterm']["$phenotype_name-env"] = [
        'cvterm_id' => $cvterms['environment'],
        '#fk' => ['phenotype' => $phenotype_name],
      ];
    }
    if ($phenotype_count >= $record_group) {
      tpps_log('[INFO] -- Inserting data into database using insert_multi...');
      tpps_chado_insert_multi($records);
      tpps_log('[INFO] - Done.');

      // $temp_results = chado_query(
      //   'SELECT * FROM chado.phenotype
      //   WHERE uniquename ILIKE :phenotype_name', array(
      //   ':phenotype_name' => $phenotype_name
      // ));
      // foreach($temp_results as $temp_row) {
      //   echo "Found phenotype saved: " . $temp_row->uniquename . "\n";
      // }
      $records = [
        'phenotype' => [],
        'phenotypeprop' => [],
        'stock_phenotype' => [],
      ];
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
  // print_r("row\n");
  // print_r($row);
  // print_r("tree_info\n");
  // print_r($tree_info);
  // print_r("\n");
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
  // $analysis_id = $options['analysis_id'];
  // echo "Analysis ID: $analysis_id\n";

  $record_group = variable_get('tpps_record_group', 10000);
  $stock_id = NULL;

  if (!empty($options['tree_id'])) {
    $val = $row[$options['tree_id']];
    $stock_id = $tree_info[trim($val)]['stock_id'];
    $current_id = $tree_info[trim($val)]['organism_id'];
    $species_code = $species_codes[$current_id];
  }

  $keys = array_keys($row);
  $key_index = -1;
  $tree_id = NULL;
  foreach ($row as $key => $val) {
    $key_index++;
    // echo "ROW key:$key, val:$val\n";
    // tpps_log("[INFO] ROW KEY $key and ROW VAL $val");
    if (empty($headers[$key])) {
      continue;
    }

    // This $val is different from the $val later on
    // so order is important

    if (!isset($stock_id)) {
      // Set the tree_id
      $tree_id = trim($val);
      $study_accession = $options['study_accession'];
      $tree_id = $study_accession . '-' . $tree_id;
      $stock_id = $tree_info[trim($val)]['stock_id'];
      $current_id = $tree_info[trim($val)]['organism_id'];
      $species_code = $species_codes[$current_id];
      continue;
    }
    $genotype_count++;
    echo "Stock ID: $stock_id, Current ID: $current_id, Genotype_count: $genotype_count\n";


    echo "Header before alterations:" . $headers[$key] . "\n";

    $header_length = strlen($headers[$key]);
    // Cater for Diploids [Rish: 8/3/2023]
    if($options['ploidy'] == 'Diploid' && substr($headers[$key], $header_length - 2, 2) == "_A") {
      // Remove the _A from the first diploid header
      // and allow the below code to continue to be processed so the SSR can be imported in
      $headers[$key] = substr($headers[$key], 0, $header_length - 2);
      $options['diploid_header'] = $headers[$key];
      $options['diploid_val'] = $val;
      // Save this header for use in a later iteration when _B gets called
      // This reason for this is we want _A and _B values recorded
      echo "Diploid first header reset to: " . $headers[$key] . "\n";
      // This will skip processing iteration by ONE iteration if _A (SSR diploid detected)
      continue;
    }

    // [RISH] This is a minor adjustment for diploid done on 8/3/2023
    if($options['ploidy'] == 'Diploid' && substr($headers[$key], $header_length - 2, 2) == "_B") {
      $options['diploid_val'] .= ',' . $val;

      // Reset to these new values for insertion into the database later on
      $headers[$key] = $options['diploid_header'];
      $val = $options['diploid_val'];
      echo "Diploid val: $val\n";
    }
    // End of cater for diploids

    // Cater for Polyploids [RISH: 8/7/2023]
    // Get header without the trailing _X (_1,_2,_3 etc)
    $header_parts = explode("_", $headers[$key]);
    $header_parts_length = count($header_parts);
    $header_without_polyploid_index = "";
    for ($j = 0; $j < $header_parts_length - 1; $j++) {
      if ($j > 0) {
        $header_without_polyploid_index .= "_";
      }
      $header_without_polyploid_index .= $header_parts[$j];
    }

    if($options['ploidy'] == 'Polyploid' && $options['polyploid_header'] != $header_without_polyploid_index) {
      // Remove the _1 from the first diploid header
      // and allow the below code to continue to be processed so the SSR can be imported in
      $headers[$key] = $header_without_polyploid_index;
      $options['polyploid_header'] = $headers[$key];
      $options['polyploid_val'] = $val;
      // Save this header for use in a later iteration when _B gets called
      // This reason for this is we want _A and _B values recorded
      echo "Polyploid first header reset to: " . $headers[$key] . "\n";
      // This will skip processing iteration by ONE iteration if _1 (SSR diploid detected)
      continue;
    }

    // [RISH] This is a minor adjustment for polyploid done on 8/7/2023
    // Look forward to see if the next headers_key
    $header_next = @$headers[$keys[$key_index + 1]];
    // Get next header without the trailing _X (_1,_2,_3 etc)
    $header_next_parts = explode("_", $header_next);
    $header_next_parts_length = count($header_next_parts);
    $header_next_without_polyploid_index = "";
    for ($j = 0; $j < $header_next_parts_length - 1; $j++) {
      if ($j > 0) {
        $header_next_without_polyploid_index .= "_";
      }
      $header_next_without_polyploid_index .= $header_next_parts[$j];
    }

    if($options['ploidy'] == 'Polyploid' && $options['polyploid_header'] == $header_without_polyploid_index) {
      $options['polyploid_val'] .= ',' . $val; // append the new value to what was already there

      // Check if the next header does not match current header (this would mean next header starts a new SSR polyploid) OR
      // if the next header is NULL (this means end of headers of the file)
      // so we need to allow the rest of code below to happen to insert this current SRR polyploid
      if (($header_without_polyploid_index != $header_next_without_polyploid_index) || $header_next == NULL) { // NULL happens if end of all headers
        // we have found that $headers[$key] is the last polyploid column for the current SSR
        // so reset to these new values for insertion into the database later on
        $headers[$key] = $options['polyploid_header'];
        $val = $options['polyploid_val'];
        echo "Polyploid val: $val for insertion using SSR marker" . $headers[$key] . "\n";
      }
      else {
        echo "Skipping insert\n";
        continue; // this will skip insertion (below code) until all values for the current SSR polyploid is found
      }
    }
    // End of catering for polyploids
    echo "Processing the insert\n";

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

    // Check if feature exists, if not insert
    $feature_check_results = chado_query('SELECT count(*) as c1 FROM chado.feature WHERE uniquename = :marker_name',[
      ':marker_name' => $marker_name
    ]);
    $feature_check_count = $feature_check_results->fetchObject()->c1;
    if ($feature_check_count <= 0) {
      chado_insert_record('feature', [
        'name' => $marker_name,
        'organism_id' => $organism_id,
        'uniquename' => $marker_name,
        'type_id' => $seq_var_cvterm,
      ]);
    }
    // Lookup the marker_name_id.
    $results = chado_query("SELECT feature_id FROM chado.feature
      WHERE uniquename = :uniquename", [
        ':uniquename' => $marker_name,
        // ':organism_id' => $organism_id
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

    // Check if feature exists, if not insert
    $feature_check_results = chado_query('SELECT count(*) as c1 FROM chado.feature WHERE uniquename = :variant_name',[
      ':variant_name' => $variant_name
    ]);
    $feature_check_count = $feature_check_results->fetchObject()->c1;

    if ($feature_check_count <= 0) {
      chado_insert_record('feature', [
        'name' => $variant_name,
        'organism_id' => $organism_id,
        'uniquename' => $variant_name,
        'type_id' => $seq_var_cvterm
      ]);
    }

    // Lookup the variant_name_id
    $results = chado_query("SELECT feature_id FROM chado.feature
      WHERE uniquename = :uniquename", [
        ':uniquename' => $variant_name,
        // ':organism_id' => $organism_id
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

      // RISH - 12/18/2023 - Requested by Emily
      ob_start();
      chado_insert_record('feature_genotype', [
        'feature_id' => $variant_name_id,
        'genotype_id' => $genotype_id,
        'chromosome_id' => NULL,
        'rank' => 0,
        'cgroup' => 0,
        'cvterm_id' => $type_cvterm,
      ]);
      // echo "feature_id: $variant_name_id, genotype_id: $genotype_id\n";
      ob_end_clean();

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
      echo "Genotype_call key: $stock_id-$genotype_name\n";
      if (isset($records2['genotype_call']["$stock_id-$genotype_name"])) {
        echo "This genotype_call key is already set (so uniqueness is maybe broken?\n";
      }

      // [RISH] Removed on 02/26/2024 in favor of new genotype_reads_per_plant
      // $records2['genotype_call']["$stock_id-$genotype_name"] = array(
      //   'project_id' => $project_id,
      //   'stock_id' => $stock_id,
      //   'genotype_id' => $genotype_id,
      //   'variant_id' => $variant_name_id,
      //   'marker_id' => $marker_name_id,
      // );

      // [RISH] 02/26/2024
      // Insert genotype reads into chado.genotype_reads_per_plant
      // We need plant name, study, marker_name
      // $options['tree_id'], $options['study_accession'], $marker_name
      // Check if a record already exists, if not, create initial record
      $per_plant_results = chado_query('
      SELECT COUNT(*) as c1 FROM chado.genotype_reads_per_plant
      WHERE tree_acc = :tree_id AND study_accession = :study_accession
      ', [
      ':tree_id' => $tree_id,
      ':study_accession' => $study_accession
      ]);
      $per_plant_records_count = $per_plant_results->fetchObject()->c1;
      if ($per_plant_records_count == 0) {
        // CREATE AN EMPTY RECORD IN TABLE
        chado_query("
          INSERT INTO chado.genotype_reads_per_plant
          (tree_acc, study_accession, marker_array, read_array)
          VALUES
          ('$tree_id', '$study_accession', ARRAY[]::text[], ARRAY[]::text[])
        ");
      }
      // So now we have a record in the table for the plant, so append the new value
      chado_query("
        UPDATE chado.genotype_reads_per_plant
        set marker_array = array_append(marker_array, '$variant_name')
        WHERE tree_acc = '$tree_id' AND study_accession = '$study_accession'
      ");
      chado_query("
        UPDATE chado.genotype_reads_per_plant
        set read_array = array_append(read_array, '$val')
        WHERE tree_acc = '$tree_id' AND study_accession = '$study_accession'
      ");

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
      if ($vcf_processing_completed == TRUE && $type == 'snp') {
        tpps_log('[INFO] - Skipped genotype and genotype_call SNPs since VCF already loaded...');
      }
      tpps_log('[INFO] - Inserting data into database using insert_multi...');
      tpps_chado_insert_multi($records, $multi_insert_options);
      tpps_log('[INFO] - Inserting data into database using insert_hybrid...');
      tpps_chado_insert_hybrid($records2, $multi_insert_options);
      tpps_log('[INFO] - Done.');
      $records = array(
        'feature' => array(),
        'genotype' => array(),
        'genotype_call' => array(),
        'stock_genotype' => array(),
      );
      // Do this for the hybrid (COPY) command.
      $records2 = array(
        'genotype_call' => array(),
      );
      if (!empty($associations)) {
        $records['featureloc'] = array();
        $records['featureprop'] = array();
      }
      $options['genotype_total'] += $genotype_count;
      tpps_log('[INFO] - Genotypes inserted:' . $options['genotype_total']);
      $genotype_count = 0;
    }
  }
  // throw new Exception("DEBUG");
}

/**
 * Processes genotype SNP design assay file.
 *
 * Initially done to get position data from the assay design file.
 */
function tpps_process_genotype_snp_assay_design($row, array &$options = array()) {
  $line_rows = $row;
  $analysis_id = $options['analysis_id']; // needed to lookup source features
  $headers = &$options['headers'];
  $records = &$options['records'];
  $columns = $options['file_columns'];
  $organism_index = $options['organism_index'];
  $seq_var_cvterm = $options['seq_var_cvterm'];
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


  // RISH NOTES: This addition uses the organism_id based on the organism order
  // of the fourth page (we likely have to pass the i from previous function here)
  // THIS TECHNICALLY OVERRIDES PETER'S LOGIC ABOVE. TO BE DETERMINED IF RISH'S WAY IS CORRECT
  // OR NOT [6/22/2023]
  // THIS WAS AN ISSUE BROUGHT UP BY EMILY REGARDING SNPS NOT BEING ASSOCIATED WITH POP TRICH (665 STUDY)
  $species_codes = $options['species_codes'];
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

    if ($feature_id == NULL) {
      // We should add the marker (marker_name)
      // $marker_name
      chado_insert_record('feature', [
        'name' => $marker_name,
        'organism_id' => $organism_id,
        'uniquename' => $marker_name,
        'type_id' => $seq_var_cvterm,
      ]);

      // Recheck for the feature_id
      $results = chado_query("SELECT * FROM chado.feature WHERE uniquename = :uniquename", [
        ':uniquename' => $marker_name
      ]);
      foreach ($results as $feature) {
        $feature_id = $feature->feature_id;
      }
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
            $results[$key] .= "_A"; // Removed by Rish 8/2/2023
            break;
          }
          $results[$key] = $last . "_B"; // Removed by Rish 8/2/2023
          // unset($results[$key]); // [RISH] this removes the second value
          break;
        }

        if ($num_headers == $row_len) {
          // All of the marker column names are filled out.
          if ($num_headers != $num_unique_headers) {
            // The marker column names are duplicates, need to append
            // _A and _B.
            if ($results[$key] == $results[$next_key]) {
              $results[$key] .= "_A"; // Removed by Rish 8/2/2023
              break;
            }
            $results[$key] .= "_B"; // Removed by Rish 8/2/2023
            // unset($results[$key]); // [RISH] Hopefully this removes the second marker
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
 * This will generate all genotype materialized views.
 */
function tpps_generate_all_genotype_materialized_views() {
  // Get count of all studies.
  $results = chado_query(
    'SELECT COUNT(DISTINCT(accession)) as c1
    FROM "public"."tpps_submission";',
    []
  );
  $total = 0;
  foreach ($results as $row) {
    $total = $row->c1;
  }
  $current_count = 0;
  // Get all the study accessions from database.
  $results = chado_query(
    'SELECT DISTINCT(accession) as accession
    FROM "public"."tpps_submission"
    ORDER BY accession;',
    []
  );
  foreach ($results as $row) {
    $current_count = $current_count + 1;
    echo "Processing genotype materialized view: $current_count of $total\n";
    // Get the Submission Shared State.
    $submission = new Submission($row->accession);
    $project_id = $submission->state['ids']['project_id'] ?? NULL;
    // Once the project_id is not NULL, we're good.
    if (isset($project_id)) {
      tpps_generate_genotype_materialized_view($project_id);
    }
  }
}

/**
 * Generate a genotype materialized view for the specific project_id.
 *
 * Project Id you must get from the state object.
 * This is used for the tpps/details genotypes tab.
 *
 * @param mixed $project_id
 *   The project ID of the study. NOT THE STUDY ACCESSION!
 */
function tpps_generate_genotype_materialized_view($project_id) {

  // Ensure the project_id is an integer.
  $project_id = intval($project_id);
  if ($project_id <= 0) {
    return;
  }
  $view_name = 'chado.genotypes_' . $project_id;
  $view_name_without_schema = 'genotypes_' . $project_id;

  // Added to fix previous queries that used stock_genotype which we don't
  // want to keep using.
  echo "Attempting to drop materialized view (if exist): " . $view_name . "\n";
  // Check first to see if it is a materialized view
  $mat_view_results = chado_query("select count(*) as c1 from pg_matviews where matviewname = :view_name;", [
    ':view_name' => $view_name_without_schema
  ]);
  $mat_view_count = $mat_view_results->fetchObject()->c1;
  // If count is more than 0, then the materialized view exists, so drop it
  if ($mat_view_count > 0) {
    chado_query("DROP MATERIALIZED VIEW IF EXISTS " . $view_name . ";");
  }

  // V3 - Emily decided to use tables (3/20/2024)
  $table_name = $view_name; // the table names will be the same as what the view names used to be
  chado_query('SET search_path TO chado,public;');
  echo "Attempting to drop table (if exist): " . $view_name . "\n";
  chado_query("DROP TABLE IF EXISTS " . $table_name . ";");
  echo "Attempting to create table (if it does not exist): " . $table_name . "\n";
  chado_query("select create_project_genotype_table(" . intval($project_id) . ");");
  echo "Genotype view table has finished being generated: " . $table_name . "\n";

  // echo "Attempting to create materialized view (if it does not exist): " . $view_name . "\n";

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

  // // @NEW V2 - This adds an index_id column
  // chado_query('CREATE MATERIALIZED VIEW ' . $view_name . ' AS ' .
  // "(SELECT
  //   row_number() over (order by g.genotype_id, s.stock_id) as index_id,
  //   g.genotype_id AS genotype_id,
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


  // // Generate the data / regenerate if necessary
  // echo "Refreshing materialized view: " . $view_name . "\n";
  // chado_query('REFRESH MATERIALIZED VIEW ' . $view_name, []);
  // echo "Finished refresh of " . $view_name . "\n";

  // // Add an index using the index_id column (IF NOT EXISTS)
  // echo "Adding index_id index to $view_name\n";
  // chado_query("CREATE UNIQUE INDEX " . str_replace('chado.', 'chado_' , $view_name) .
  //   "_id on " . $view_name . "(index_id);", []);
  // echo "Index completed\n";
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
      ->fields('l', ['title'])
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
        tpps_log('[INFO] - Inserting data into database using insert_multi...');
        tpps_chado_insert_multi($records);
        tpps_log('[INFO] - Done.');
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
 * Adds back genotype_call indexes.
 */
function tpps_create_genotype_call_indexes() {
  echo "SET CONSTRAINTS ALL IMMEDIATE\n";
  chado_query("SET CONSTRAINTS ALL IMMEDIATE;", []);
  // Recreate indexes.
  tpps_log('[INFO] - Recreating indexes...');
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_genotype_id_idx ON chado.genotype_call USING btree (genotype_id)");
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_project_id_idx ON chado.genotype_call USING btree (project_id)");
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_stock_id_idx ON chado.genotype_call USING btree (stock_id)");
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_marker_id_idx ON chado.genotype_call USING btree (marker_id)");
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_variant_id_idx ON chado.genotype_call USING btree (variant_id)");
  tpps_log('[INFO] - Recreating INDEXES - Done.');
  chado_query("SET CONSTRAINTS ALL DEFERRED;", []);
}

/**
 * This function will drop genotype_call indexes from the database.
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

      if (variable_get('tpps_submitall_skip_gps_request') && isset($geo_api_key)) {
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
    tpps_log('[INFO] - Inserting data into database using insert_multi...');
    $new_ids = tpps_chado_insert_multi($records, $multi_insert_options);
    tpps_log('[INFO] - Done.');
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
 * Lookup analysis_id from ref_genome string.
 *
 * The ref_genome string usually comes from TPPS form select.
 */
function tpps_get_analysis_id_from_ref_genome($ref_genome) {
    // We need to find the analysis id from the ref genome
    $analysis_id = NULL;
    if (isset($ref_genome)) {
      // DEPRECATED 8/12/2024 in favour of chado.tpps_ref_assembly_view created by Emily
      // // Get the species and version from the reference genome selected
      // // if match occurs thats in index [0].
      // // The group match index [1] is species, group match index [2] is version
      // preg_match('/(.+) +v(\d*\.*\d*)/', $ref_genome, $matches);
      // print_r($matches);
      // print_r("\n");
      // $ref_genome_species = NULL;
      // $ref_genome_version = NULL;
      // if (count($matches) > 0) {
      //   $ref_genome_species = $matches[1];
      //   $ref_genome_version = $matches[2];
      // }
      // echo "ref_genome_species: $ref_genome_species\n";
      // echo "ref_genome_version: $ref_genome_version\n";

      // if (isset($ref_genome_species) && isset($ref_genome_version)) {
      //   // Look up the analysis
      //   $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
      //     WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
      //     [
      //       ':name' => $ref_genome_species . '%',
      //       ':programversion' => $ref_genome_version
      //     ]
      //   );
      //   foreach ($analysis_results as $row) {
      //     print_r("analysis_row\n");
      //     print_r($row);
      //     print_r("\n");
      //     $analysis_id = $row->analysis_id;
      //   }
      // }
      // echo "analysis_id: $analysis_id\n";

      // if($analysis_id == NULL) {
      //   // Look up the analysis
      //   $analysis_results = chado_query('SELECT analysis_id FROM chado.analysis
      //     WHERE name ILIKE :name AND programversion = :programversion LIMIT 1',
      //     [
      //       ':name' => $ref_genome_species . '%',
      //       ':programversion' => 'v' . $ref_genome_version
      //     ]
      //   );
      //   foreach ($analysis_results as $row) {
      //     print_r("analysis_row\n");
      //     print_r($row);
      //     print_r("\n");
      //     $analysis_id = $row->analysis_id;
      //   }
      // }
      // else {
      //   return $analysis_id;
      // }


      // If an analysis_id still was not found, it's possibly from the db data source
      // instead of the genome directory. The genome directory code is in page_4*.php
      // New code to cater for new analysis checks via db - query given by Emily Grau (6/6/2023)
      if ($analysis_id == NULL) {
        $genome_query_results = chado_query("select * from chado.tpps_ref_assembly_view WHERE name LIKE :ref_genome;", [
          ':ref_genome' => $ref_genome
        ]);
        foreach ($genome_query_results as $genome_query_row) {
          print_r("genome_query_row\n");
          print_r($genome_query_row);
          print_r("\n");
          $analysis_id = $genome_query_row->analysis_id;
        }
      }
      // $options['analysis_id'] = $analysis_id;
      echo "analysis_id: $analysis_id\n";
      return $analysis_id;
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
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $page4_values["organism-$i"]['genotype'] ?? NULL;
  if (!isset($genotype)) {
    return false;
  }
  if ($genotype['files']['file-type'] == TPPS_GENOTYPING_FILE_TYPE_VCF) {
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
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $page4_values["organism-$i"]['genotype'] ?? NULL;
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


function tpps_assay_design_location($form_state, $i) {
  $results = [
    'status' => 'empty', // empty, exists, missing
    'location' => NULL,
    'fid' => 0,
  ];
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $page4_values["organism-$i"]['genotype'] ?? NULL;
  $snp_fid = $genotype['files']['assay-design'];
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
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $page4_values["organism-$i"]['genotype'] ?? NULL;
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
    // [RISH] 8/28/2023 - TODO - check if they (markers) match or not
    // because we changed logic of how this is done, we may not need this check
    // since we are interested in a design file and snps assay file check instead

    // require_once(__DIR__ . '/../includes/form_utils.php');
    // $accession = $form_state['accession'];
    // $results = tpps_compare_vcf_markers_vs_snps_assay_markers_results_array($accession, $i);
    // if ($results['markers_not_in_vcf_count'] > 0) {
    //   throw new Exception("SNPs assay contains markers that are not in the VCF file");
    // }
    // else {
    //   echo "[CHECK PASSED] SNPs assay does not contain additional markers compared to VCF file\n";
    // }
    // if ($results['markers_not_in_snps_assay'] > 0) {
    //   throw new Exception("VCF contains markers that are not present in SNPs assay");
    // }
    // else {
    //   echo "[CHECK PASSED] VCF does not contain additional markers compared to SNPs assay file\n";
    // }
  }

}

// TODO
function tpps_accession_file_get_tree_ids($form_state, $i) {
  $page3_values = $form_state['saved_values'][TPPS_PAGE_3];
  $fid = $page3_values['tree-accession']['species-' . $i]['file'];
  $snp_assay_header = tpps_file_headers($fid);
}

function tpps_accession_file_location($form_state, $i) {
  $results = [
    'status' => 'empty', // empty, exists, missing
    'location' => NULL,
    'fid' => 0,
  ];
  $page3_values = $form_state['saved_values'][TPPS_PAGE_3];
  $fid = $page3_values['tree-accession']['species-' . $i]['file'];
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
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $page4_values["organism-$i"]['genotype'] ?? NULL;
  if (!isset($genotype)) {
    return false;
  }
  if ($genotype['files']['file-type'] == TPPS_GENOTYPING_FILE_TYPE_VCF) {
    $vcf_fid = $genotype['files']['vcf'];
    // This means it was uploaded.
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
    if (
      stripos($vcf_line,'#CHROM') !== FALSE
      && $vcf_line != ""
      && str_replace("\0", "", $vcf_line) != ""
    ) {
      $vcf_line = explode("\t", $vcf_line);
      $vcf_line_parts_count = count($vcf_line);
      for ($i = 9; $i < $vcf_line_parts_count; $i++) {
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
 * Returns the VCF markers (called variants) from a file specified by location.
 *
 * @param string $location
 *   File location.
 *
 * @return array
 *   Values, unique_count, count.
 */
function tpps_genotype_get_vcf_markers($location) {
  $vcf_content = gzopen($location, 'r');
  $variants = []; // markers basically
  $duplicate_variants = []; // record duplicates
  $count = 0;
  while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
    if (
      $vcf_line[0] != '#'
      && stripos($vcf_line, '.vcf') === FALSE
      && trim($vcf_line) != ""
      && str_replace("\0", "", $vcf_line) != ""
    ) {
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
 * Return the snps assay markers from a file specified by the FILE ID.
 *
 * @param int $fid
 *   FID (File ID).
 *
 * @return array
 *   Values, unique_count, count.
 */
function tpps_genotype_get_snps_assay_markers($fid) {
  $snp_assay_header = tpps_file_headers($fid);
  // Ignore the first column which contains the tree_id.
  $results = [];
  $duplicates = [];
  $count = 0;
  foreach ($snp_assay_header as $column_letters => $column_value) {
    $column_value = trim($column_value);
    if ($count == 0) {
      // ignore (this is the first column which is the tree id header).
    }
    else {
      if (isset($results[$column_value])) {
        // duplicate detected.
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

/**
 * Adds message to 2 logs [VS].
 *
 * @param string $message
 *   The message to store in the logs.
 * @param array $variables
 *   Array of variables to replace in the message on display.
 *   or NULL if message is already translated or not possible to translate.
 * @param string $severity
 *   The severity of the message; one of the following values:
 *      TRIPAL_CRITICAL: Critical conditions.
 *      TRIPAL_ERROR: Error conditions.
 *      TRIPAL_WARNING: Warning conditions.
 *      TRIPAL_NOTICE: Normal but significant conditions.
 *      TRIPAL_INFO: (default) Informational messages.
 *      TRIPAL_DEBUG: Debug-level messages.
 *
 * @TODO Move substrings like [INFO] into separate param.
 */
function tpps_log($message, array $variables = [], $severity = TRIPAL_INFO) {
  global $tpps_job;
  // Writes to file and will be shown at site.
  tpps_job_logger_write($message, $variables);
  // Add time to CLI output. Tripal logs will be unchanged.
  if (variable_get('tpps_submitall_log_cli_show_time', FALSE)) {
    $time = format_date(time(), 'custom', "Y/m/d H:i:s O");
    $message = $time . ' ' . $message;
  }
  // Command line messages.
  try {
    $tpps_job->logMessage($message, $variables, $severity);
  }
  catch (Exception $ex) {

  }
  catch (Error $err) {

  }
}

/**
 * Processes SSR file.
 *
 * @param array $form_state
 *   Drupal Form State.
 * @param int $fid
 *   Managed File Id.
 * @param array $options
 *   Some options.
 * @param object $job
 *   Tripal Job.
 * @param array $multi_insert_options
 *   Some options again.
 */
function tpps_ssr_process(array &$form_state, $fid, array &$options, $job, array $multi_insert_options) {
  tpps_add_project_file($form_state, $fid);
  // tpps_drop_genotype_call_indexes($job);

  tpps_log('[INFO] - Processing EXTRA genotype_spreadsheet file data...');
  echo "trace 3\n";
  tpps_file_iterator($fid, 'tpps_process_genotype_spreadsheet', $options);
  tpps_log('[INFO] - Done.');

  tpps_log('[INFO] - Inserting data into database using insert_multi...');
  tpps_chado_insert_multi($options['records'], $multi_insert_options);

  tpps_log('[INFO] - Inserting data into database using insert_hybrid...');
  tpps_chado_insert_hybrid($options['records2'], $multi_insert_options);
  tpps_log('[INFO] - Done.');
  // CREATE INDEXES FROM GENOTYPE_CALL TABLE.
  // tpps_create_genotype_call_indexes($job);
}
