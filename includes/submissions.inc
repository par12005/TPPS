<?php

/**
 * @file
 * Defines functions for manipulation of TPPS Submissions.
 */

/**
 * This function loads a TPPS Submission based on the provided accession number.
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
 * @return array
 *   The form_state of the TPPS Submission form.
 */
function tpps_load_submission($accession, $state = TRUE) {
  $result = db_select('tpps_submission', 's')
    ->fields('s')
    ->condition('accession', $accession)
    ->range(0, 1)
    ->execute()->fetchObject();
  // For newer TPPS submissions, $result should be populated.
  if ($result) {
    if ($state) {
      return unserialize($result->submission_state);
    }
    return $result;
  }
  // Provide legacy support for older TPPS submissions.
  else {
    global $user;
    $state = variable_get('tpps_incomplete_' . $user->mail . $accession, NULL);
    if (!empty($state)) {
      return $state;
    }
    else {
      return variable_get('tpps_complete_' . $user->mail . $accession, NULL);
    }
  }
}

/**
 * This function loads multiple TPPS submissions based on conditions.
 *
 * Collects accessions based on conditions, then makes multiple calls to
 * tpps_load_submission() to load all of the submissions that meet the
 * conditions.
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
  $submissions = array();
  $query = db_select('tpps_submission', 's')
    ->fields('s', array('accession'));

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

  $results = $query->execute()->fetchAll();
  foreach ($results as $result) {
    $submissions[] = tpps_load_submission($result->accession, $state);
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
  $values = array(
    'uid' => $uid,
    'status' => 'Incomplete',
    'accession' => $state['accession'],
    'dbxref_id' => $state['dbxref_id'],
    'submission_state' => serialize($state),
  );

  db_insert('tpps_submission')
    ->fields($values)
    ->execute();
}

/**
 * This function updates the form_state of a submission.
 *
 * Additional attributes of the submission record can be updated by populating
 * the options parameter.
 *
 * @param array $state
 *   The form_state of the submission.
 * @param array $options
 *   Additional submission attributes to update.
 */
function tpps_update_submission(array $state, array $options = array()) {
  if (empty($options['status']) and !empty($state['status'])) {
    $options['status'] = $state['status'];
  }
  $options['submission_state'] = serialize($state);

  db_update('tpps_submission')
    ->fields($options)
    ->condition('accession', $state['accession'])
    ->execute();
}

/**
 * This function deletes a TPPS Submission based on an accession number.
 *
 * It also removes the submission record from the chado.dbxref table.
 *
 * @param string $accession
 *   The accession number of the submission to delete.
 */
function tpps_delete_submission($accession, $redirect = TRUE) {
  global $user;
  $submission = tpps_load_submission($accession, FALSE);
  $dbxref_id = $submission->dbxref_id;
  $state = unserialize($submission->submission_state);
  db_delete('tpps_submission')
    ->condition('accession', $accession)
    ->execute();
  if (empty($state['saved_values']['frontpage']['use_old_tgdr'])) {
    db_delete('chado.dbxref')
      ->condition('dbxref_id', $dbxref_id)
      ->execute();
  }
  if ($redirect) {
    drupal_goto("user/{$user->uid}/tpps");
  }
}

/**
 * Gives all files in an accession a standardized name.
 *
 * @param string $accession
 *   The accession number of the submission to change file names.
 */
function tpps_submission_rename_files($accession) {
  $state = tpps_load_submission($accession);
  foreach ($state['file_info'] as $page_files) {
    foreach ($page_files as $fid => $name) {
      $file = tpps_rename_file($fid, "{$accession}_{$name}");
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
    }
  }
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
  $state = tpps_load_submission($accession);
  $state['file_rank'] = 0;
  $project_id = $state['ids']['project_id'] ?? NULL;
  if (empty($project_id)) {
    return FALSE;
  }

  $stocks_sql = "SELECT stock_id FROM chado.project_stock WHERE project_id = $project_id";
  chado_query("DELETE FROM chado.project_organism WHERE project_id = $project_id");
  chado_query("DELETE FROM chado.projectprop WHERE project_id = $project_id");
  chado_query("DELETE FROM chado.phenotype_cvterm WHERE phenotype_id IN (SELECT phenotype_id from chado.stock_phenotype WHERE stock_id in ($stocks_sql))");
  chado_query("DELETE FROM chado.phenotype WHERE phenotype_id IN (SELECT phenotype_id FROM chado.stock_phenotype WHERE stock_id IN ($stocks_sql))");
  chado_query("DELETE FROM chado.genotype_call WHERE project_id = $project_id");
  chado_query("DELETE FROM chado.stock_genotype WHERE stock_id IN ($stocks_sql)");
  chado_query("DELETE FROM chado.stock_phenotype WHERE stock_id IN ($stocks_sql)");
  chado_query("DELETE FROM chado.stock WHERE stock_id IN ($stocks_sql)");

  $state['ids'] = array('project_id' => $project_id);
  tpps_update_submission($state);
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
        $form_state['stats']['tree_count'] += tpps_file_len($form_state['values']['tree-accession']["species-$i"]['file']) - 1;
        $form_state['stats']['tree_count'] += $form_state['values']['tree-accession']["species-$i"]['file-no-header'];
        if ($form_state['stats']['species_count'] == 1 or empty($form_state['values']['tree-accession']['check'])) {
          break;
        }
      }
      break;

    case TPPS_PAGE_4:
      $form_state['stats']['phenotype_count'] = 0;
      for ($i = 1; $i <= $form_state['stats']['species_count']; $i++) {
        $phenotype = $form_state['values']["organism-$i"]['phenotype'];
        if (empty($phenotype['iso_check']) and !empty($phenotype['file']) and file_load($phenotype['file'])) {
          $rows = tpps_file_len($phenotype['file']) - 1 + !empty($phenotype['file-no-header']);
          if ($phenotype['format'] == 0) {
            $phenotype_file_name_cols = $phenotype['file-groups']['Phenotype Data']['0'];
            $form_state['stats']['phenotype_count'] += $rows * count($phenotype_file_name_cols);
          }
          else {
            $form_state['stats']['phenotype_count'] += $rows;
          }
          continue;
        }
        if (!empty($phenotype['iso']) and file_load($phenotype['iso'])) {
          $content = tpps_parse_file($phenotype['iso'], 1);
          while (($k = array_search(NULL, $content['headers']))) {
            unset($content['headers'][$k]);
          }
          $num_unique_columns = count(array_unique($content['headers'])) - 1;
          $rows = tpps_file_len($phenotype['iso']) - 1;
          $form_state['stats']['phenotype_count'] += $rows * $num_unique_columns;
        }
      }
      break;

    default:
      break;
  }
}
