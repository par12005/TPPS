<?php

/**
 * @file
 * Functions for manipulation of TPPS Submissions.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/submissions');
 *
 * @TODO Minor. Rename functions to use pattern 'tpps_submission_[action]'.
 */

module_load_include('inc', 'tpps', 'includes/submission_file');

/**
 * Menu API callback. Shows Submission State array in JSON format.
 *
 * Path: tpps/api/submission
 * If a valid TPPS submission exists for the accession, it will return
 * the form_state of the submission.
 *
 * @param string $accession
 *   The accession number of the submission.
 *
 * @return mixed
 *   Returns Submission From State array or NULL if not found.
 *
 * @todo Move to other file.
 */
function tpps_api_get_submission_state($accession) {
  return tpps_submission($accession)->getState();
}

/**
 * This function loads multiple TPPS submissions based on conditions.
 *
 * To load all submissions: Submission::loadAll().
 * To have sorting:
 *   asort(tpps_load_submission_multiple(...), SORT_NATURAL);
 *
 * Execution time (138 studies):
 *  Single Query: 715 msec (current),
 *  Multiple Queries: 1150 msec (before, outdated).
 *
 * @param array $conditions
 *   The conditions that each TPPS submission must meet.
 *
 * @return array
 *   An array of loaded TPPS submissions or their form_states.
 *
 * @todo Cache this list.
 * @todo load_multiple() usually accepts list of Ids but not $conditions.
 * Better to rename to *_search().
 */
function tpps_load_submission_multiple(array $conditions = []) {
  $output = [];
  // We exclude only 'shared_state' field.
  $field_list = [
    'accession',
    'uid',
    'status',
    'dbxref_id',
    'tpps_submission_id',
    'submission_state',
  ];
  $query = db_select('tpps_submission', 't')
    ->fields('t', $field_list)
    ->orderBy('accession', 'DESC');
  if (!empty($conditions)) {
    foreach ($conditions as $key => $cond) {
      if (!empty($cond)) {
        if (!is_array($cond)) {
          $query->condition($key, $cond);
        }
        else {
          $or = db_or();
          foreach ($cond as $item) {
            $or->condition($key, $item);
          }
          $query->condition($or);
        }
      }
    }
  }
  $result = $query->execute()->fetchAllAssoc('accession', PDO::FETCH_ASSOC);
  foreach ($result as $accession => $data) {
    $output[$accession] = new Submission($accession, $data);
  }
  return $output;
}

/**
 * This function creates a TPPS submission entry in the tpps_submission table.
 *
 * @param array $state
 *   The form_state of the submission.
 * @param int $uid
 *   The id of the user creating the submission.
 */
function tpps_create_submission(array $state, $uid) {
  $accession = $state['accession'] ?? NULL;
  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    $submission->create($state, $uid);
  }
}

/**
 * Changes the TGDR number of a study (and does resubmission)
 *
 * @param string $old_accession
 *   The old accession number being submitted.
 * @param string $new_accession
 *   The new accession number being submitted.
 * @param TripalJob $job
 *   The TripalJob object for the submission job.
 */
function tpps_change_tgdr_number($old_accession, $new_accession, TripalJob $job = NULL) {
  $job->logMessage('[INFO] Clearing database for study ' . $old_accession);
  $job->setInterval(1);
  // Clear the db of all data for the old study accession
  // tpps_submission_clear_db($old_accession);

  $message = t('[INFO] Alter submission state data from @old_accession '
    . 'to @new_accession.',
    ['@old_accession' => $old_accession, '@new_accession' => $new_accession]
  );
  $job->logMessage($message);

  // Update the form state data for the study.
  $submission = new Submission($old_accession);

  $job->logMessage('Original submission data for ' . $old_accession);
  $job->logMessage(print_r($submission->state, 1));

  // Update the TGDR with the new_accession.
  module_load_include('inc', 'tpps', 'includes/array');
  $submission->state = TppsArray::replace(
    $submission->state, $old_accession, $new_accession
  );
  $submission->sharedState = TppsArray::replace(
    $submission->sharedState, $old_accession, $new_accession
  );

  $job->logMessage('New submission data for ' . $new_accession);
  $job->logMessage(print_r($submission->state, 1));

  // The call of tpps_update_submission() below will not work here
  // because it tries to update a non-existing TGDR (the new one has
  // not been created as yet). The update SQL after this, will make
  // it exist because it renames the old tgdr number to the new tgdr number
  // So instead of this function tpps_update_submission, manually do it
  // tpps_update_submission($state);

  // @TODO First update accession in 'tpps_submission' then:
  // $submission->accession = $new_accession;
  // $submission->status = Submission::STATUS_APPROVED;
  // $submission->save();
  $state['updated'] = time();
  db_update('tpps_submission')
    ->fields([
      'status' => Submission::STATUS_APPROVED,
      // Note: REQUEST_TIME couldn't be used because this function also
      // called from submit_all.php and time() != REQUEST_TIME.
      'submission_state' => serialize($submission->state),
      'shared_state' => serialize($submission->sharedState),
    ])
    ->condition('accession', $old_accession)
    ->execute();

  $job->logMessage('[INFO] Update tpps_submission table study '
    . $old_accession . ' to ' . $new_accession
  );
  // Now change the accession number in the tpps_submission table.
  $transaction = db_transaction();
  try {
    $sql = 'UPDATE tpps_submission SET accession = :new_accession '
      . ' WHERE accession = :old_accession';
    chado_query($sql,
      [':new_accession' => $new_accession, ':old_accession' => $old_accession]
    );
  }
  catch (\Exception $e) {
    $transaction->rollback();
    throw $e;
  }

  $message = t('Completed! The next job in the queue should reimport the '
    . 'study data for @accession.', ['@accession' => $new_accession]
  );
  $job->logMessage($message);
}

/**
 * Menu callback to remove the TPPS Submission.
 *
 * @param string $accession
 *   The accession of the submission. Format: TGDR12345.
 */
function tpps_submission_delete_callback($accession) {
  global $user;
  tpps_submission_load($accession)->purge();
  drupal_goto("user/{$user->uid}/tpps");
}

/**
 * Updates alternative accessions of a TPPS Submission.
 *
 * @param array $state
 *   The submission being updated.
 * @param mixed $alt_accession
 *   The alternative accessions of the state.
 */
function tpps_submission_add_alternative_accession(array $state, $alt_accession) {
  $tpps_local_db = variable_get('tpps_local_db');
  if (!is_array($alt_accession)) {
    $alt_accession = [$alt_accession];
  }

  $submission = new Submission($state['accession']);

  // Remove the existing alternative accessions.
  db_delete('tpps_submission_dbxref')
    ->condition('tpps_submission_id', $submission->id)
    ->execute();

  foreach ($alt_accession as $accession) {
    $dbx = chado_select_record('dbxref', ['*'], [
      'db_id' => $tpps_local_db->db_id,
      'accession' => $accession,
    ]);
    $dbxref_id = current($dbx)->dbxref_id ?? NULL;

    if (empty($dbxref_id)) {
      $dbx = chado_insert_record('dbxref', [
        'db_id' => $tpps_local_db->db_id,
        'accession' => $accession,
      ]);
      $dbxref_id = $dbx['dbxref_id'];
    }

    db_insert('tpps_submission_dbxref')
      ->fields([
        'tpps_submission_id' => $submission->id,
        'dbxref_id' => $dbxref_id,
      ])
      ->execute();
  }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/**
 * Gives all files in an accession a standardized name.
 *
 * @param string $accession
 *   The accession number of the submission to change file names.
 */
function tpps_submission_rename_files($accession) {
  $submission = new Submission($accession);
  if (empty($submission->sharedState['file_info'])) {
    return;
  }
  $submission->sharedState['files'] = [];
  foreach ($submission->sharedState['file_info'] as $page_files) {
    foreach ($page_files as $fid => $name) {
      $submission->sharedState['files'][] = $fid;
      if ($name === '#NO_RENAME') {
        continue;
      }
      $old_file = TppsFile::load($fid);
      if ($old_file->uri[0] != '/') {
        $old_path = file_create_url($old_file->uri);
        // Prepare new file_name.
        $file = $old_file;
        $new_filename = tpps_file_build_new_name(
          $accession . '_' . $name,
          $file->uri
        );
        // Check if file already exists then return that file.
        if ($new_filename != '') {
          $new_fid = db_select('file_managed', 'fm')
            ->fields('fm', ['fid'])
            ->condition('uri', $new_filename)
            ->range(0, 1)->execute()->fetchField();
        }
        if (!empty($new_fid)) {
          $file = TppsFile::load($new_fid);
        }
        else {
          $file = tpps_rename_file($fid, "{$accession}_{$name}");
          $file->status = FILE_STATUS_PERMANENT;
          $file = file_save($file);
        }
        $new_path = file_create_url($file->uri);
        chado_update_record('projectprop', [
          'project_id' => $submission->sharedState['ids']['project_id'],
          'value' => $old_path,
        ], [
          'value' => $new_path,
        ]);
      }
      // @TODO $fid could differ from $file->fid because code above searches
      // path in db to reuse it. Should $file->fid be used instead of $fid?
      if (!empty($submission->sharedState['revised_files'][$fid])) {
        $rev_fid = $submission->sharedState['revised_files'][$fid];
        $submission->sharedState['files'][] = $rev_fid;
        $file = tpps_rename_file($rev_fid, "{$accession}_{$name}_revised");
        $file->status = FILE_STATUS_PERMANENT;
        file_save($file);
      }
    }
  }
  $submission->save();
}

/**
 * Clears additional db information for a specific study.
 *
 * Such as genotypeprop data as well as feature data.
 *
 * @param string $accession
 *   Study accession.
 * @return void
 */
function tpps_submission_clear_db_extra($accession) {
  // FOR LATER USE MAYBE.
  echo "Deleting genotypeprop data...\n";
  $start_time = time();
  chado_query('
    delete from chado.genotypeprop where genotype_id in
    (select genotype_id from chado.genotype where genotype_id not in
    (select genotype_id from chado.genotype_call));',
  []);
  $end_time = time();
  echo "Time taken to complete: " . ($end_time - $start_time) . "\n";

  // FOR EACH ORGANISM IN THIS STUDY.
  for ($i = 1; $i <= ($state['saved_values'][TPPS_PAGE_1]['organism']['number'] ?? 1); $i++) {
    // GET THE ORGANISM NAME / GENUS SPECIES.
    $organism = $state['saved_values'][TPPS_PAGE_1]['organism'][$i];
    $organism_name = $organism['name'];
    // Lookup the organism_id from the organism_name.
    $organism_name_parts = explode(' ', $organism_name);
    $genus = $organism_name_parts[0];
    $species = '';
    $organism_name_parts_count = count($organism_name_parts);
    for ($j = 1; $j < $organism_name_parts_count; $j++) {
      $species .= $organism_name_parts[$j] . ' ';
    }
    $species = trim($species);

    // LOOKUP ORGANISM_ID.
    $results = chado_query('SELECT * FROM chado.organism WHERE genus ILIKE :genus AND species ILIKE :species LIMIT 1;',[
      ':genus' => $genus,
      ':species' => $species
    ]);
    $organism_id = NULL;
    foreach ($results as $row) {
      $organism_id = $row->organism_id;
    }


    if ($organism_id != NULL) {
      // // DELETE features based on organism_id if it is not NULL
      $start_time = time();

      $cvterm_id = NULL;
      // Get cvterm for sequence_variant
      $results = chado_query("SELECT * FROM chado.cvterm WHERE name = :name LIMIT 1;", [
        ':name' => 'sequence_variant'
      ]);

      foreach ($results as $row) {
        $cvterm_id = $row->cvterm_id;
      }

      if ($cvterm_id == NULL) {
        echo "Could not find cvterm name for sequence_variant\n";
        echo "This means no features will be able to be deleted! COPY command could fail to insert!\n";
        echo "Please contact db administrator to determine how to delete features\n";
        break;
      }

      echo "Deleting features data for organism_id = $organism_id ($genus $species)... \n";
      chado_query('
        delete from chado.feature where organism_id = :organism_id and type_id = :cvterm_id
        and feature_id not in
        ((select marker_id from chado.genotype_call) union (select variant_id from chado.genotype_call));
      ', [
        ':organism_id' => $organism_id,
        ':cvterm_id' => $cvterm_id
      ]);

      $end_time = time();
      echo "Time taken to complete: " . ($end_time - $start_time) . "\n";
    }
    // print_r($organism);
  }
}

function tpps_submission_full_clear_db($project_id) {
  // Create index in genotype_call if they don't exist
  echo "Creating genotype_call indexes if they do not exist...\n";
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_genotype_id_idx ON chado.genotype_call USING btree (genotype_id)",[]);
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_project_id_idx ON chado.genotype_call USING btree (project_id)",[]);
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_stock_id_idx ON chado.genotype_call USING btree (stock_id)",[]);
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_marker_id_idx ON chado.genotype_call USING btree (marker_id)",[]);
  chado_query("CREATE INDEX IF NOT EXISTS genotype_call_variant_id_idx ON chado.genotype_call USING btree (variant_id)",[]);

  echo "Delete the temporary feature_ids_$project_id table...\n";
  chado_query("DROP TABLE IF EXISTS chado.feature_ids_" . $project_id . "", []);

  echo "Create table to get features for deletion...\n";
  chado_query("create table chado.feature_ids_" . $project_id . " as
    ((select variant_id  as feature_id from chado.genotype_call where project_id = :project_id_1)
    union
    (select marker_id  as feature_id from chado.genotype_call where project_id = :project_id_2));", [
      ':project_id_1' => $project_id,
      ':project_id_2' => $project_id
  ]);


  echo "Delete genotype_calls...\n";
  chado_query("delete from chado.genotype_call where variant_id in
    (select feature_id from chado.feature_ids_" . $project_id . ")
    or marker_id in (select feature_id from chado.feature_ids_" . $project_id . ");", [
  ]);

  echo "DELETE featurelocs...\n";
  chado_query("delete from chado.featureloc where feature_id in
    (select feature_id from chado.feature_ids_" . $project_id . ");", []);

  echo "DELETE feature_relationship records...\n";
  chado_query("delete from chado.feature_relationship where subject_id in
  (select feature_id from chado.feature_ids_" . $project_id . ") or object_id in
  (select feature_id from chado.feature_ids_" . $project_id .  ");", []);

  echo "DELETE feature records...\n";
  chado_query("delete from chado.feature where feature_id in
  (select feature_id from chado.feature_ids_" . $project_id . ");", []);

  echo "Delete the temporary feature_ids_$project_id table...\n";
  chado_query("DROP TABLE IF EXISTS chado.feature_ids_" . $project_id, []);
}

/**
 * Clears submission data from the database.
 *
 * This function is particularly useful when submitting a study to the database
 * multiple times. Note that Tripal Entities should be manually un-published
 * after this process has completed.
 *
 * @param string $accession
 *   The accession number of the submission.
 *
 * @return bool
 *   Returns TRUE if submission successfully cleared and FALSE otherwise.
 */
function tpps_submission_clear_db($accession) {
  $time_start = time();
  $submission = new Submission($accession);
  if (empty($accession)) {
    throw new Exception('Accession cannot be empty');
  }

  // This will perform a full delete of feature ids as well (used to delete features
  // from TGDR665 and 674 6/29/2023)
  // tpps_submission_full_clear_db($project_id);

  $result = $submission->clearDataBase();
  if ($result) {
    tpps_log('FINISHED DB CLEAR DELETES');
    tpps_log('Time taken to delete records: @time seconds.',
      ['@time' => time() - $time_start]
    );
  }
  return $result;
}

/**
 * Updates all statistics for a TPPS Submission.
 *
 * @param array $form_state
 *   The form_state of the submission being updated.
 */
function tpps_submission_update_all_stats(array &$form_state) {
  for ($i = 1; $i <= 4; $i++) {
    $form_state['values'] = $form_state['saved_values'][$i];
    $form_state['stage'] = $i;
    tpps_submission_update_stats($form_state);
  }
}

/**
 * Updates statistics for a TPPS Submission for one form step.
 *
 * @param array $form_state
 *   The form_state of the submission being updated.
 */
function tpps_submission_update_stats(array &$form_state) {
  switch ($form_state['stage']) {
    case TPPS_PAGE_1:
      $form_state['stats']['author_count'] = $form_state['values']['publication']['secondaryAuthors']['number'] + 1;
      // @TODO [VS] Review this code it seems outdated.
      // 'check' will be set to FALSE when publication data received from
      // DOI server but 'file' seems not used in code.
      if (!empty($form_state['values']['publication']['secondaryAuthors']['check'])) {
        $form_state['stats']['author_count'] = tpps_file_len($form_state['values']['publication']['secondaryAuthors']['file']);
      }

      $form_state['stats']['species_count'] = $form_state['values']['organism']['number'];
      break;

    case TPPS_PAGE_2:
      if (empty($form_state['tpps_type']) or $form_state['tpps_type'] == 'tpps') {
        $start = new DateTime("{$form_state['values']['StartingDate']['year']}-{$form_state['values']['StartingDate']['month']}");
        $end = new DateTime("{$form_state['values']['EndingDate']['year']}-{$form_state['values']['EndingDate']['month']}");
        $dur = date_diff($start, $end);
        $form_state['stats']['duration'] = $dur->format('%y years %m months');
      }
      break;

    case TPPS_PAGE_3:
      $form_state['stats']['tree_count'] = 0;
      for ($i = 1; $i <= $form_state['stats']['species_count']; $i++) {
        $form_state['stats']['tree_count'] += tpps_file_len($form_state['values']['tree-accession']["species-$i"]['file']);
        $form_state['stats']['tree_count'] += $form_state['values']['tree-accession']["species-$i"]['file-no-header'];
        if ($form_state['stats']['species_count'] == 1 or empty($form_state['values']['tree-accession']['check'])) {
          break;
        }
      }
      break;

    case TPPS_PAGE_4:
      $form_state['stats']['phenotype_count'] = 0;
      $form_state['stats']['unique_phenotypes'] = 0;
      for ($i = 1; $i <= $form_state['stats']['species_count']; $i++) {
        $organism_key = 'organism-' . $i;
        $phenotype = $form_state['values'][$organism_key]['phenotype'] ?? NULL;
        $form_state['stats']['unique_phenotypes'] += $phenotype['phenotypes-meta']['number'] ?? 0;
        // Note: both 'Normal' and 'Iso' files could be used at the same time.
        if (PhenotypeData::isSelected($i, $form_state)) {
          $fid = PhenotypeData::getFileId($i, $form_state);
          $rows = tpps_file_len($fid) + !empty($phenotype['file-no-header']);
          $phenotype_file_name_cols = $phenotype['file-groups']['Phenotype Data']['0'];
          $form_state['stats']['phenotype_count'] += $rows * count($phenotype_file_name_cols);
          if (TppsFile::load($phenotype['metadata'])) {
            $form_state['stats']['unique_phenotypes'] +=
              tpps_file_len($phenotype['metadata'])
              + !empty($phenotype['metadata-no-header']);
          }
        }
        if (PhenotypeIso::isSelected($i, $form_state)) {
          $headers = PhenotypeIso::getFileHeader($i, $form_state);
          if (!empty($headers)) {
            while (($k = array_search(NULL, $headers))) {
              unset($headers[$k]);
            }
            // @TODO Update to count new 'Clone Number' column.
            // If column set - use '2' but if not - '1'
            $num_unique_columns = count(array_unique($headers)) - 1;
            $form_state['stats']['phenotype_count'] +=
              PhenotypeData::countRows($i, $form_state)
              * $num_unique_columns;
          }
        }
      }
      break;

    default:
      break;
  }
}

/**
 * Form to edit submission publication information.
 *
 * @param array $form
 *   The form object to be populated.
 * @param array $form_state
 *   The state of the form object to be populated.
 * @param mixed $accession
 *   The accession of the submission being edited.
 *
 * @return array
 *   The populated form object.
 */
function tpps_submission_edit_publication(array $form, array &$form_state, $accession = NULL) {
  global $base_url;
  $submission = new Submission($accession);
  $state = $submission->state;

  $form['accession'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];

  $form['status'] = array(
    '#type' => 'select',
    '#title' => t('Publication Status'),
    '#options' => array(
      0 => t('- Select -'),
      'In Preparation or Submitted' => t('In Preparation or Submitted'),
      'In Press' => t('In Press'),
      'Published' => t('Published'),
    ),
    '#default_value' => $state['saved_values'][TPPS_PAGE_1]['publication']['status'] ?? NULL,
    '#required' => TRUE,
    '#prefix' => "<a href=\"$base_url/tpps/details/$accession\">Back to TPPS Details Page</a>",
  );

  $year_options = array(0 => '- Select -');
  for ($i = 1990; $i <= date('Y'); $i++) {
    $year_options[$i] = "$i";
  }

  $form['year'] = array(
    '#type' => 'select',
    '#title' => t('Year of Publication'),
    '#options' => $year_options,
    '#description' => t('If your publication has not been published yet, '
    . 'please choose the expected year of publication.'),
    '#default_value' => $state['saved_values'][TPPS_PAGE_1]['publication']['year'] ?? NULL,
    '#required' => TRUE,
  );

  $form['journal'] = array(
    '#type' => 'textfield',
    '#title' => t('Journal'),
    '#autocomplete_path' => 'tpps/autocomplete/journal',
    '#default_value' => $state['saved_values'][TPPS_PAGE_1]['publication']['journal'],
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Implements hook_form_validate().
 */
function tpps_submission_edit_publication_validate(&$form, &$form_state) {
  if (empty($form_state['values']['status'])) {
    form_set_error('status', t('Publication Status: field is required.'));
  }
  if (empty($form_state['values']['year'])) {
    form_set_error('year', t('Publication Year: field is required.'));
  }
}

/**
 * Implements hook_form_submit().
 */
function tpps_submission_edit_publication_submit($form, &$form_state) {
  $accession = $form_state['values']['accession'];
  $submission = new Submission($accession);
  $submit_job = FALSE;
  $page1_values = $submission->state['saved_values'][TPPS_PAGE_1] ?? NULL;

  if ($page1_values['publication']['status'] != $form_state['values']['status']) {
    $page1_values['publication']['status'] = $form_state['values']['status'];
    $submit_job = TRUE;
  }

  if ($page1_values['publication']['year'] != $form_state['values']['year']) {
    $page1_values['publication']['year'] = $form_state['values']['year'];
    $submit_job = TRUE;
  }

  if ($page1_values['publication']['journal'] != $form_state['values']['journal']) {
    $page1_values['publication']['journal'] = $form_state['values']['journal'];
    $submit_job = TRUE;
  }

  if ($submit_job) {
    module_load_include('php', 'tpps', 'forms/submit/submit_all');

    $includes = [];
    $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
    $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
    $args = [$accession];
    $jid = tripal_add_job(t("Update Publication Information - @accession",
      ['@accession' => $accession]), 'tpps', 'tpps_submit_all',
      $args, $submission->state['submitting_uid'], 10, $includes, TRUE
    );
    $submission->state['job_id'] = $jid;
  }
  $submission->state['saved_values'][TPPS_PAGE_1] = $page1_values;
  $submission->save();
}

/**
 * Builds list of accessions (values) and their dbxref_id's as a keys.
 *
 * Cached version: ~3 msec.
 * Non-cached version: ~280 msec.
 *
 * @param bool $is_optional
 *   Should this list have an extra item '- Select -' which makes this
 *   dropdown list optional.
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 *
 * @return array
 *   Returns array where keys are studie's dbxref_id and value is an
 *   accession (like TGDRxxxx).
 */
function tpps_submission_get_tgdr_number_list($is_optional = TRUE, $reset = FALSE) {
  $default_value = [];

  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $key = md5(json_encode(func_get_args()));
  if ($reset || empty($cache) || empty($cache->data[$key])) {
    $content = $default_value;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $content = db_query('SELECT dbxref_id, accession '
      . 'FROM chado.dbxref '
      . 'WHERE accession LIKE \'TGDR%\' '
      . 'ORDER BY accession DESC;')->fetchAllKeyed();
    if ($is_optional) {
      $content = array_merge([0 => '- Select -'], $content);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $cache->data[$key] = $content;
    cache_set($cid, $cache->data, $cache_bin);
  }
  $content = $cache->data[$key] ?? $default_value;
  return $content;
}

/**
 * This function loads a TPPS Submission based on the provided accession number.
 *
 * WARNING: This function is used but other modules.
 *
 * If the state option is set to false, this function will return the full
 * tpps_submission table record for the submission. Otherwise, the function will
 * return only the form_state of the submission. The state option defaults to
 * True.
 *
 * @param string $accession
 *   The accession number of the submission.
 * @param bool $state
 *   An option to return only the state of the TPPS Submission. Default to True.
 *
 * @global stdClass $user
 *   The user trying to access the submission.
 *
 * @return array|object
 *   The form_state of the TPPS Submission form.
 */
function tpps_load_submission($accession, $state = TRUE) {
  $result = db_select('tpps_submission', 's')
    ->fields('s')
    ->condition('accession', $accession)
    ->range(0, 1)
    ->execute()->fetchObject();

  if ($result) {
    if ($state) {
      return unserialize($result->submission_state);
    }
    return $result;
  }

  // Search for alternative accessions.
  $query = db_select('tpps_submission', 's');
  $query->join('tpps_submission_dbxref', 's_dbx', 's.tpps_submission_id = s_dbx.tpps_submission_id');
  $query->join('chado.dbxref', 'dbx', 'dbx.dbxref_id = s_dbx.dbxref_id');
  $query->fields('s');
  $query->condition('dbx.accession', $accession);
  $query->condition('s.status', Submission::STATUS_APPROVED);
  $query->range(0, 1);
  $query = $query->execute();
  $result = $query->fetchObject();

  if ($result) {
    if ($state) {
      return unserialize($result->submission_state);
    }
    return $result;
  }

  // Provide legacy support for older TPPS submissions.
  global $user;
  $state = variable_get('tpps_incomplete_' . $user->mail . $accession, NULL);
  if (!empty($state)) {
    return $state;
  }
  else {
    return variable_get('tpps_complete_' . $user->mail . $accession, NULL);
  }
}
