<?php

/**
 * @file
 * Functions for manipulation of TPPS Submissions.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/submissions');
 *
 * @TODO Minor. Rename functions to use pattern 'tpps_submission_[action]'.
 * @TODO Move tag-functions to separate file.
 */

module_load_include('inc', 'tpps', 'includes/submission_file');
module_load_include('inc', 'tpps', 'includes/submission_interface');
module_load_include('inc', 'tpps', 'includes/submission_form_version');

/**
 * Loads a TPPS Submission Info.
 *
 * @param string $accession
 *   The Submission Accession Number like 'TGDRxxxx'.
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 *
 * @return mixed
 *   Returns an array with Submission's Info from DB table 'tpps_submission'.
 *   Returns NULL if study wasn't found.
 */
function tpps_load_submission_info($accession, $reset = FALSE) {
  $submission = new Submission($accession);
  return $submission->getInfo($reset);
}

/**
 * Loads a TPPS Submission Form State Array.
 *
 * @param string $accession
 *   The accession number of the submission.
 *
 * @return mixed
 *   Returns Submission From State array or NULL if not found.
 */
function tpps_load_submission_state($accession, $reset = FALSE) {
  $submission = new Submission($accession);
  return $submission->getState($reset);
}

/**
 * This function loads multiple TPPS submissions based on conditions.
 *
 * @param array $conditions
 *   The conditions that each TPPS submission must meet.
 * @param bool $state
 *   An option to return only the state of the TPPS Submissions, default True.
 *
 * @return array
 *   An array of loaded TPPS submissions or their form_states.
 */
function tpps_load_submission_multiple(array $conditions = array(), $state = TRUE) {
  $submissions = [];
  $query = db_select('tpps_submission', 's');
  // Exclude 'submission_interface' column.
  $query->fields('s', array_diff(
    array_keys(drupal_get_schema('tpps_submission')['fields']),
    ['submission_interface']
  ));

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

  $results = $query->execute();
  while (($result = $results->fetchObject())) {
    if ($state) {
      $submissions[] = unserialize($result->submission_state);
      continue;
    }
    $submissions[] = $result;
  }
  return $submissions;
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
  $submission = new Submission();
  $submission->create($state, $uid);
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

  $job->logMessage('[INFO] Alter submission state data from ' . $old_accession . ' to ' . $new_accession);
  // Update the form state data for the study.
  $submission = new Submission($old_accession);
  $state = $submission->state;
  $interface = tpps_submission_interface_load($old_accession);

  $job->logMessage('Original submission data for ' . $old_accession);
  $job->logMessage(print_r($state, 1));

  // Update the TGDR with the new_accession.
  module_load_include('inc', 'tpps', 'includes/array');
  $state = tpps_array_replace($state, $old_accession, $new_accession);
  $interface = tpps_array_replace($interface, $old_accession, $new_accession);

  $job->logMessage('New submission data for ' . $new_accession);
  $job->logMessage(print_r($state, 1));

  // The call of tpps_update_submission() below will not work here
  // because it tries to update a non-existing TGDR (the new one has
  // not been created as yet). The update SQL after this, will make
  // it exist because it renames the old tgdr number to the new tgdr number
  // So instead of this function tpps_update_submission, manually do it
  // tpps_update_submission($state);

  $state['updated'] = time();
  db_update('tpps_submission')
    ->fields([
      'status' => 'Approved',
      // Note: REQUEST_TIME couldn't be used because this function also
      // called from submit_all.php and time() != REQUEST_TIME.
      'submission_state' => serialize($state),
      'submission_interface' => serialize($interface),
    ])
    ->condition('accession', $old_accession)
    ->execute();

  $job->logMessage('[INFO] Update tpps_submission table study ' . $old_accession . ' to ' . $new_accession);
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

  $job->logMessage('Completed! The next job in the queue should reimport the study data for ' . $new_accession);
}

/**
 * Updates Submission Information in DB table 'tpps_submission'.
 *
 * @param string $accession
 *   The Study accession.
 * @param array $options
 *   Values for existing columns in DB table 'tpps_submission'.
 */
function tpps_update_submission_info($accession, array $options = []) {
  $submission = new Submission($accession);
  $submission->updateInfo($options);
}

/**
 * Updates Submission State in DB table 'tpps_submission'.
 *
 * @param array $form_state
 *   The Submission's Form State.
 */
function tpps_update_submission_state(array &$form_state) {
  $submission = new Submission();
  $submission->updateState($form_state);
  // Get updated Submission State.
  $form_state = $submission->state;
}

/**
 * Deletes a TPPS Submission.
 *
 * Removes records from tables:
 * 1. chado.dbxref,
 * 2. chado.tpps_submission.
 *
 * @param string $accession
 *   The accession of the submission. Format: TGDR12345.
 */
function tpps_delete_submission($accession) {
  $submission = new Submission($accession);
  $submission->delete();
}

/**
 * Menu callback to remove the TPPS Submission.
 *
 * @param string $accession
 *   The accession of the submission. Format: TGDR12345.
 */
function tpps_submission_delete_callback($accession) {
  global $user;
  tpps_delete_submission($accession);
  drupal_goto("user/{$user->uid}/tpps");
}

/**
 * Removes TPPS Submission and it's files.
 *
 * @param string $accession
 *   The accession of the submission. Format: TGDR12345.
 * @param bool $force
 *   Boolean indicating that the file should be deleted
 *   if the file not in use by the file_usage table.
 *   Removement of not used files are safe but I'm not sure that old studies
 *   created correct records about usage of files so do NOT delete files
 *   by default.
 *   But for testing and Study Import file removement should be allowed.
 */
function tpps_submission_purge($accession, $force = FALSE) {
  $submission = new Submission($accession);
  $submission->purge();
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
    $alt_accession = array($alt_accession);
  }

  $submission_info = tpps_load_submission_info($state['accession']);
  $state_id = $submission_info['tpps_submission_id'];

  // Remove the existing alternative accessions.
  db_delete('tpps_submission_dbxref')
    ->condition('tpps_submission_id', $state_id)
    ->execute();

  foreach ($alt_accession as $acc) {
    $dbx = chado_select_record('dbxref', array('*'), array(
      'db_id' => $tpps_local_db->db_id,
      'accession' => $acc,
    ));
    $dbxref_id = current($dbx)->dbxref_id ?? NULL;

    if (empty($dbxref_id)) {
      $dbx = chado_insert_record('dbxref', array(
        'db_id' => $tpps_local_db->db_id,
        'accession' => $acc,
      ));
      $dbxref_id = $dbx['dbxref_id'];
    }

    db_insert('tpps_submission_dbxref')
      ->fields(array(
        'tpps_submission_id' => $state_id,
        'dbxref_id' => $dbxref_id,
      ))
      ->execute();
  }
}

/**
 * Display TPPS submission tags.
 *
 * @return string
 *   String of renderable HTML.
 */
function tpps_submission_tag_manage() {

  $query = db_select('tpps_tag', 't')
    ->fields('t')
    ->execute();

  $rows = array();
  while (($result = $query->fetchObject())) {
    $color = !empty($result->color) ? $result->color : 'white';
    $edit_link = $result->static ? "" : "<a href=\"tpps-tag/edit/{$result->tpps_tag_id}\">edit</a>";
    $rows[$result->tpps_tag_id] = array(
      "<span class=\"tag\" style=\"background-color:$color\"><span class=\"tag-text\">{$result->name}</span></span>",
      $result->name,
      $color,
      $edit_link,
    );
  }

  ksort($rows);

  $vars = array(
    'header' => array(
      'Tag',
      'Name',
      'Color',
      '',
    ),
    'rows' => $rows,
    'attributes' => array(
      'class' => array('view'),
      'id' => 'tpps_table_display',
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );

  $output = theme('table', $vars);
  tpps_add_css_js('main');
  return $output;
}

/**
 * Form to create a new submission tag.
 *
 * @param array $form
 *   The form object to be populated.
 * @param array $form_state
 *   The state of the form object to be populated.
 *
 * @return array
 *   The populated form object.
 *
 * @todo Minor. Move to includes/form.inc and rename to tpps_form_tag_create().
 */
function tpps_submission_tag_create(array $form, array &$form_state) {
  // This will add the color picker to the TPPS submission tags edit-color
  // field id.
  drupal_add_js(
    'https://cdnjs.cloudflare.com/ajax/libs/tinyColorPicker/1.1.1/jqColorPicker.min.js',
    'external'
  );
  // @TODO [VS] This file must be stored under /sites/all/libraries to allow
  // other modules to reuse it and get updates independently from this module.
  drupal_add_js(TPPS_MODULE_PATH . '/js/tag_colorpicker.js');

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('The name of the new tag'),
    '#required' => TRUE,
    '#prefix' => '<a href="/tpps-tag">Back to TPPS Submission Tags page</a>',
  );

  $form['color'] = array(
    '#type' => 'textfield',
    '#title' => t('Color'),
    '#description' => t('The color of the new tag. Can be the name of a color, like "red" or "blue", or can be a hexidecimal color like "#FFFFFF" or "#A2C32F". Hexidecimal colors must start with the "#" character.'),
    '#required' => TRUE,
    '#suffix' => '<div id="color-picker"><a>Toggle Color Picker</a></div>',
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
function tpps_submission_tag_create_validate(&$form, &$form_state) {
  $result = db_select('tpps_tag', 't')
    ->fields('t')
    ->condition('name', $form_state['values']['name'], 'ILIKE')
    ->range(0, 1)
    ->execute()->fetchObject();
  if (!empty($result)) {
    form_set_error('name', t("A tag already exists with this name. Please select a different name or use the existing tag."));
  }
}

/**
 * Implements hook_form_submit().
 */
function tpps_submission_tag_create_submit($form, &$form_state) {
  db_insert('tpps_tag')
    ->fields(array(
      'name' => $form_state['values']['name'],
      'color' => $form_state['values']['color'],
    ))
    ->execute();
  drupal_goto('tpps-tag');
}

/**
 * Form to edit a submission tag.
 *
 * @param array $form
 *   The form object to be populated.
 * @param array $form_state
 *   The state of the form object to be populated.
 * @param mixed $tag_id
 *   The id of the tag being edited.
 *
 * @return array
 *   The populated form object.
 */
function tpps_submission_tag_edit(array $form, array &$form_state, $tag_id = NULL) {
  if (empty($tag_id)) {
    drupal_goto('tpps-tag');
  }

  $tag = db_select('tpps_tag', 't')
    ->fields('t')
    ->condition('tpps_tag_id', $tag_id)
    ->range(0, 1)
    ->execute()->fetchObject();

  if (empty($tag) or $tag->static) {
    drupal_goto('tpps-tag');
  }

  $form = tpps_submission_tag_create($form, $form_state);

  $form['name']['#default_value'] = $tag->name;
  $form['color']['#default_value'] = $tag->color;

  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $tag_id,
  );

  return $form;
}

/**
 * Implements hook_form_validate().
 */
function tpps_submission_tag_edit_validate(&$form, &$form_state) {
  $result = db_select('tpps_tag', 't')
    ->fields('t')
    ->condition('name', $form_state['values']['name'], 'ILIKE')
    ->condition('tpps_tag_id', $form_state['values']['id'], '!=')
    ->range(0, 1)
    ->execute()->fetchObject();
  if (!empty($result)) {
    form_set_error('name', t("A tag already exists with this name. Please select a different name or use the existing tag."));
  }
}

/**
 * Implements hook_form_submit().
 */
function tpps_submission_tag_edit_submit($form, &$form_state) {
  db_update('tpps_tag')
    ->fields(array(
      'name' => $form_state['values']['name'],
      'color' => $form_state['values']['color'],
    ))
    ->condition('tpps_tag_id', $form_state['values']['id'])
    ->execute();
  drupal_goto('tpps-tag');
}

/**
 * Returns array of tags associated with a submission.
 *
 * @param string $accession
 *   The accession of the desired submission.
 *
 * @return array
 *   Array of tags associated with the submission.
 */
function tpps_submission_get_tags($accession) {
  $query = db_select('tpps_submission_tag', 'st');
  $query->join('tpps_submission', 's', 's.tpps_submission_id = st.tpps_submission_id');
  $query->join('tpps_tag', 't', 't.tpps_tag_id = st.tpps_tag_id');
  $query->fields('t');
  $query->condition('s.accession', $accession);
  return $query->execute()->fetchAllAssoc('tpps_tag_id', PDO::FETCH_ASSOC);
}

/**
 * Removes default tags associated with a submission.
 *
 * @param string $accession
 *   The accession of the desired submission.
 */
function tpps_submission_clear_default_tags($accession) {
  $tags = array(
    'Genotype',
    'Phenotype',
    'Environment',
  );
  foreach ($tags as $tag) {
    tpps_submission_remove_tag($accession, $tag);
  }
}

/**
 * Edit submission tags.
 *
 * @param string $type
 *   Either 'add' or 'remove'.
 * @param string $accession
 *   The accession of the desired submission.
 * @param mixed $tag
 *   The name or id of the tag to be added or removed.
 */
function tpps_submission_add_remove_tag($type, $accession, $tag) {
  $result = db_select('tpps_tag', 't')
    ->fields('t')
    ->condition('tpps_tag_id', $tag)
    ->range(0, 1)
    ->execute()->fetchObject();

  if ($result->static) {
    drupal_goto('tpps-tag');
  }

  if ($type == 'add') {
    tpps_submission_add_tag($accession, $tag);
  }
  if ($type == 'remove') {
    tpps_submission_remove_tag($accession, $tag);
  }
}

/**
 * Adds a tag to a submission.
 *
 * @param string $accession
 *   The accession of the desired submission.
 * @param mixed $tag
 *   The name or id of the tag to be added.
 */
function tpps_submission_add_tag($accession, $tag) {
  if (gettype($tag) == 'string') {
    $id = tpps_get_tag_id($tag);
    if (!empty($id)) {
      $tag = $id;
    }
  }
  $tags = tpps_submission_get_tags($accession) ?? [];
  if (!array_key_exists($tag, $tags)) {
    $submission_info = tpps_load_submission_info($accession);
    db_insert('tpps_submission_tag')
      ->fields([
        'tpps_submission_id' => $submission_info['tpps_submission_id'],
        'tpps_tag_id' => $tag,
      ])
      ->execute();
  }
}

/**
 * Removes a tag from a submission.
 *
 * @param string $accession
 *   The accession of the desired submission.
 * @param mixed $tag
 *   The name or id of the tag to be removed.
 */
function tpps_submission_remove_tag($accession, $tag) {
  if (gettype($tag) == 'string') {
    $id = tpps_get_tag_id($tag);
    if (!empty($id)) {
      $tag = $id;
    }
  }
  $submission_info = tpps_load_submission_info($accession);
  db_delete('tpps_submission_tag')
    ->condition('tpps_submission_id', $submission_info['tpps_submission_id'])
    ->condition('tpps_tag_id', $tag)
    ->execute();
}

/**
 * Returns the id of a tag based on the name.
 *
 * @param string $name
 *   The name of the desired tag.
 *
 * @return int
 *   The id of the tag.
 */
function tpps_get_tag_id($name) {
  $result = db_select('tpps_tag', 't')
    ->fields('t', array('tpps_tag_id'))
    ->condition('name', $name)
    ->range(0, 1)
    ->execute()->fetchObject();

  if ($result) {
    return $result->tpps_tag_id;
  }
  return FALSE;
}

/**
 * Gives all files in an accession a standardized name.
 *
 * @param string $accession
 *   The accession number of the submission to change file names.
 */
function tpps_submission_rename_files($accession) {

  // @TODO Rename $state into $interface.
  // @TODO Get $interface as a param instead of loading it again.
  $state = tpps_submission_interface_load($accession);
  if (!empty($state['file_info'])) {
    $state['files'] = array();
    foreach ($state['file_info'] as $page_files) {
      foreach ($page_files as $fid => $name) {
        $state['files'][] = $fid;
        if ($name === '#NO_RENAME') {
          continue;
        }
        $old_file = tpps_file_load($fid);
        if ($old_file->uri[0] != '/') {
          $old_path = file_create_url($old_file->uri);
          // Prepare new file_name.
          $file_load = tpps_file_load($fid);
          $new_filename = $accession . '_' . $name . "."
            . tpps_get_path_extension($file_load->uri);
          if (!preg_match('/^(.*\/)(.*)$/', $new_name, $matches)) {
            preg_match('/^(.*\/).*$/', $file->uri, $matches);
            $new_filename = $matches[1] . $new_filename;
          }
          // Check if file already exists then return that file.
          if ($new_filename != '') {
            $result = db_query('SELECT f.fid
            FROM {file_managed} f WHERE f.uri = :uri', array(':uri' => $new_filename));
            $record = $result->fetchObject();
          }

          if (isset($record)) {
            $file = tpps_file_load($record->fid);
          }
          else {
            $file = tpps_rename_file($fid, "{$accession}_{$name}");
            $file->status = FILE_STATUS_PERMANENT;
            $file = file_save($file);
          }

          $new_path = file_create_url($file->uri);
          chado_update_record('projectprop', array(
            'project_id' => $state['ids']['project_id'],
            'value' => $old_path,
          ), array(
            'value' => $new_path,
          ));
        }
        if (!empty($state['revised_files'][$fid])) {
          $rev_fid = $state['revised_files'][$fid];
          $state['files'][] = $rev_fid;
          $file = tpps_rename_file($rev_fid, "{$accession}_{$name}_revised");
          $file->status = FILE_STATUS_PERMANENT;
          file_save($file);
        }
      }
    }
    tpps_submission_interface_save($state);
  }
}

/**
 * This function will clear additional db information for a specific study
 * such as genotypeprop data as well as feature data
 * @param mixed $accession
 * @return void
 */
function tpps_submission_clear_db_extra($accession) {
  // FOR LATER USE MAYBE
  echo "Deleting genotypeprop data...\n";
  $start_time = time();
  chado_query("
  delete from chado.genotypeprop where genotype_id in
  (select genotype_id from chado.genotype where genotype_id not in
  (select genotype_id from chado.genotype_call));",
  []);
  $end_time = time();
  echo "Time taken to complete: " . ($end_time - $start_time) . "\n";


  // FOR EACH ORGANISM IN THIS STUDY
  $organism_count = $state['saved_values']['1']['organism']['number'];
  for ($i = 1; $i <= $organism_count; $i++) {
    // GET THE ORGANISM NAME / GENUS SPECIES
    $organism = $state['saved_values'][1]['organism'][$i];
    $organism_name = $organism['name'];
    // Lookup the organism_id from the organism_name
    $organism_name_parts = explode(' ',$organism_name);
    $genus = $organism_name_parts[0];
    $species = '';
    $organism_name_parts_count = count($organism_name_parts);
    for ($j=1; $j < $organism_name_parts_count; $j++) {
      $species .= $organism_name_parts[$j] . ' ';
    }
    $species = trim($species);

    // LOOKUP ORGANISM_ID
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
      chado_query("
      delete from chado.feature where organism_id = :organism_id and type_id = :cvterm_id
      and feature_id not in
      ((select marker_id from chado.genotype_call) union (select variant_id from chado.genotype_call));
      ", [
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
 *   Returns TRUE if submission successfully cleared.
 */
function tpps_submission_clear_db($accession) {
  // @TODO Minor. Rename $state with $submission_interface.
  $state = tpps_submission_interface_load($accession);
  $state['file_rank'] = 0;
  $project_id = $state['ids']['project_id'] ?? NULL;
  if (empty($project_id)) {
    return FALSE;
  }
  if (empty($accession)) {
    throw new Exception('Accession cannot be empty');
  }

  $time_start = time();

  // This will perform a full delete of feature ids as well (used to delete features
  // from TGDR665 and 674 6/29/2023)
  // tpps_submission_full_clear_db($project_id);

  // @TODO Execute $stocks_sql and use plain list of ids instead of query.
  $stocks_sql = "SELECT stock_id FROM chado.project_stock WHERE project_id = $project_id";
  echo $stocks_sql . "\n";
  chado_query("UPDATE chado.project_dbxref SET is_current = FALSE WHERE project_id = $project_id");
  echo "DELETE chado.project_organism values for project...\n";
  chado_query("DELETE FROM chado.project_organism WHERE project_id = $project_id");
  echo "DELETE chado.project_pub values for project...\n";
  chado_query("DELETE FROM chado.project_pub WHERE project_id = $project_id");
  echo "DELETE chado.projectprop values for project...\n";
  chado_query("DELETE FROM chado.projectprop WHERE project_id = $project_id");
  echo "DELETE chado.projectprop values for project...\n";
  db_query("DELETE FROM public.tpps_project_file_managed WHERE project_id = $project_id");
  echo "DELETE tpps_project_file_managed values for project...\n";
  chado_query("DELETE FROM chado.phenotype_cvterm WHERE phenotype_id IN (SELECT phenotype_id from chado.stock_phenotype WHERE stock_id in ($stocks_sql))");
  echo "DELETE chado.phenotype_to_synonym values for project...\n";
  chado_query("DELETE FROM chado.phenotype_to_synonym WHERE phenotype_id IN (SELECT phenotype_id FROM chado.stock_phenotype WHERE stock_id IN ($stocks_sql))");
  echo "DELETE chado.phenotype values for project...\n";
  chado_query("DELETE FROM chado.phenotype WHERE phenotype_id IN (SELECT phenotype_id FROM chado.stock_phenotype WHERE stock_id IN ($stocks_sql))");
  echo "DELETE chado.genotype_call values for project...\n";
  chado_query("DELETE FROM chado.genotype_call WHERE project_id = $project_id");
  echo "DELETE chado.stock_genotype values for project...\n";
  chado_query("DELETE FROM chado.stock_genotype WHERE stock_id IN ($stocks_sql)");
  echo "DELETE chado.stock_phenotype values for project...\n";
  chado_query("DELETE FROM chado.stock_phenotype WHERE stock_id IN ($stocks_sql)");
  echo "DELETE chado.stock values for project...\n";
  chado_query("DELETE FROM chado.stock WHERE stock_id IN ($stocks_sql)");
  echo "DELETE chado.phenotype values for project...\n";
  chado_query("DELETE FROM chado.phenotype WHERE uniquename LIKE '" . $accession . "-%';");

  echo "FINISHED DB CLEAR DELETES\n";
  $time_end = time();
  echo "Time taken to delete records: " . ($time_end - $time_start) . "\n";

  // Force delete stock just in case by study accession name.
  // Make sure to ensure $accession is properly populated before doing this.
  if (strpos($accession, 'TGDR') !== FALSE && strlen($accession) >= 7) {
    chado_query("DELETE FROM chado.stock WHERE uniquename LIKE :accession", [
      ':accession' => $accession . '-%'
    ]);
  }

  $state['ids'] = ['project_id' => $project_id];
  tpps_submission_interface_save($state);
  return TRUE;
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
        $phenotype = $form_state['values']["organism-$i"]['phenotype'] ?? NULL;
        $form_state['stats']['unique_phenotypes'] += $phenotype['phenotypes-meta']['number'] ?? 0;
        if (tpps_file_load($phenotype['file'])) {
          $rows = tpps_file_len($phenotype['file']) + !empty($phenotype['file-no-header']);
          if ($phenotype['format'] == 0) {
            $phenotype_file_name_cols = $phenotype['file-groups']['Phenotype Data']['0'];
            $form_state['stats']['phenotype_count'] += $rows * count($phenotype_file_name_cols);
          }
          else {
            $form_state['stats']['phenotype_count'] += $rows;
          }

          if (tpps_file_load($phenotype['metadata'])) {
            $form_state['stats']['unique_phenotypes'] += tpps_file_len($phenotype['metadata']) + !empty($phenotype['metadata-no-header']);
          }

          continue;
        }
        if (tpps_file_load($phenotype['iso'])) {
          $headers = tpps_file_headers($phenotype['iso']);
          while (($k = array_search(NULL, $headers))) {
            unset($headers[$k]);
          }
          $num_unique_columns = count(array_unique($headers)) - 1;
          $rows = tpps_file_len($phenotype['iso']);
          $form_state['stats']['phenotype_count'] += $rows * $num_unique_columns;
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
    '#default_value' => $state['saved_values'][TPPS_PAGE_1]['publication']['status'],
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
    '#description' => t('If your publication has not been published yet, please choose the expected year of publication.'),
    '#default_value' => $state['saved_values'][TPPS_PAGE_1]['publication']['year'],
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
  $state = $submission->state;
  $submit_job = FALSE;
  $page1_values = $state['saved_values'][TPPS_PAGE_1] ?? NULL;

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

    $includes = array();
    $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
    $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
    $args = array($accession);
    $jid = tripal_add_job("Update Publication Information - $accession",
      'tpps', 'tpps_submit_all',
      $args, $state['submitting_uid'], 10, $includes, TRUE
    );
    $state['job_id'] = $jid;
    tpps_update_submission_state($state);
  }
}

/**
 * Builds a list of accessions for form.
 *
 * @param array $conditions
 *   List of condtions for Drupal DB API contions.
 *   For example: ['uid', $user->uid, '=']
 *   Possible fields: 'uid', 'accession', 'status' and etc. See table's schema.
 * @param bool $remove_empty
 *   Flag which defines if "empty" studies (has empty publication title)
 *   must be removed or not. Default is TRUE.
 * @param bool $reset
 *   Flag which defined if cached data must be updated or not.
 *   Default is FALSE (do not update cached data).
 *
 *   Total execution time (95 items):
 *   Previous:  703.94587516785 msec
 *   No caching 412.0819568634 msec
 *   Caching:     1.80983543396 msec
 *   Note: previous version loaded each study twice.
 *
 * @return array
 *   Returns list of accession to be used in form.
 *   Key is an accession and value is a human readable title.
 */
function tpps_submission_get_accession_list(array $conditions = [], $remove_empty = TRUE, $reset = FALSE) {
  // We use built-in Drupal Cache System because there is a problem with using
  // 'LIKE' operator for 'tpps_submission' table and processing serialized
  // data takes a lot of time.
  // Caching increases function execution time in 1000 times.
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $key = serialize($conditions);
  $is_cache_allowed = variable_get('tpps_front_cache_accession_list', FALSE);
  if (!$is_cache_allowed || $reset || empty($cache) || empty($cache->data[$key])) {
    // Exclude 'submission_interface' column.
    $fields = array_diff(
      array_keys(drupal_get_schema('tpps_submission')['fields']),
      ['submission_interface']
    );
    $query = db_select('tpps_submission', 's')->fields('s', $fields);
    foreach ($conditions as $item) {
      $query->condition($item[0], $item[1], ($item[2] ?? '='));
    }
    $results = $query->orderBy('tpps_submission_id', 'desc')->execute();
    foreach ($results as $item) {
      if ($state = unserialize($item->submission_state)) {
        $page1_values = $state['saved_values'][TPPS_PAGE_1] ?? NULL;
        if ($title = ($page1_values['publication']['title'] ?? NULL)) {
          $list[$state['accession']] = $state['accession'] . ' - ' . (
            (strlen($title) > 97)
            ? substr($title, 0, 97) . '...'
            : (!empty($title) ? $title : t('No Title'))
          );
        }
        elseif (empty($page1_values) && $remove_empty) {
          // Study is treated as orphan (or empty) and will be removed when:
          // 1. It has no publication title.
          // 2. Page 1 wasn't submitted or saved yet.
          // 3. Removement is allowed (default is allow removement).
          // Note: Empty study will be created when user goes to Page 1 from
          // 'Welcome' page. Empty studies often created during developement
          // but users could also create empty studies. Not sure if accession
          // number of those studies could be reused but it's better to avoid
          // useless records in DB.
          tpps_delete_submission($item->accession);
        }
      }
    }
    $cache->data[$key] = $list;
    cache_set($cid, $cache->data, $cache_bin);
  }
  return $cache->data[$key] ?? [];
}

/**
 * Builds list of accessions (values) and their dbxref_id's as a keys.
 *
 * @param bool $is_optional
 *   Should this list have an extra item '- Select -' which makes this
 *   dropdown list optional.
 *
 * @return array
 *   Returns array where keys are studie's dbxref_id and value is an
 *   accession (like TGDRxxxx).
 */
function tpps_submission_get_tgdr_number_list($is_optional = TRUE) {
  $result = db_query('SELECT dbxref_id, accession '
    . 'FROM chado.dbxref '
    . 'WHERE accession LIKE \'TGDR%\' '
    . 'ORDER BY accession DESC;');
  $list = $result->fetchAllKeyed();
  if ($is_optional) {
    $list = [0 => '- Select -'] + $list;
  }
  return $list;
}
