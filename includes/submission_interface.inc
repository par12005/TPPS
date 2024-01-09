<?php

/**
 * @file
 * Functions for manipulation of TPPS Submissions.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/submission_interface');
 */

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
function tpps_si_update(array $state, array $options = array()) {
  if (empty($options['status']) and !empty($state['status'])) {
    $options['status'] = $state['status'];
  }
  $state['updated'] = time();
  // @TODO Convert $state to the interface.
  // Should we receive $form_state or prepared $interface.
  $options['submission_interface'] = serialize($state);
  db_update('tpps_submission')
    ->fields($options)
    ->condition('accession', $state['accession'])
    ->execute();
}

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
 * @return array|object
 *   The form_state of the TPPS Submission form.
 */
function tpps_si_load($accession, $state = TRUE) {
  $result = db_select('tpps_submission', 's')
    // @TODO exclude 'submission_interface' column.
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
  $query->condition('s.status', 'Approved');
  $query->range(0, 1);
  $query = $query->execute();
  $result = $query->fetchObject();


  // @Todo Use only interface.
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

