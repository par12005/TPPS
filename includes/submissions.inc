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
    else {
      return $result;
    }
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
function tpps_delete_submission($accession) {
  $dbxref_id = tpps_load_submission($accession, FALSE)->dbxref_id;
  db_delete('tpps_submission')
    ->condition('accession', $accession)
    ->execute();
  db_delete('chado.dbxref')
    ->condition('dbxref_id', $dbxref_id)
    ->execute();
}
