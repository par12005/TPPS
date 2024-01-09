<?php

/**
 * @file
 * Functions for manipulation of TPPS Submissions.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/submission_interface');
 *
 * Since submission data could be updated from admin's pages and they not use
 * a single function but update DB table directly it's safer for now to update
 * Submission Interface Array on each change of Submission State Array.
 *
 * To get updated and serialized Submission Interface ready to be stored in DB:
 * 'submission_interface' => tpps_submission_interface_convert($state),
 *
 * @TODO Idea: Naming: tpps_submission_convert|get|update_interface().
 */

/**
 * Converts $form_state into the Submission Interface.
 *
 * @param array $form_state
 *   Drupal Form State array.
 *
 * @return array
 *   Returns serialized array with datastructure prepared for TPPS pipeline
 *   (submit_all.php script).
 */
function tpps_submission_interface_convert(array $form_state = []) {
  $interface = $form_state;
  // @TODO Convert.
  // Remove useless elements.
  // Convert versions.

  return serialize($interface);
}

/**
 * Gets Submission Interface for given study.
 *
 * Note:
 * Forked tpps_load_submission().
 *
 * @param string $accession
 *   The accession of the submission in format 'TGDRxxxx'.
 *
 * @return mixed
 *   Returns array with Submission Interface data prepared for pipeline processing.
 *   Returns FALSE if Submission wan't found.
 */
function tpps_submission_interface_load($accession) {
  $result = db_select('tpps_submission', 's')
    // @TODO exclude 'submission_interface' column.
    ->fields('s', ['submission_interface'])
    ->condition('accession', $accession)
    ->range(0, 1)
    ->execute()->fetchObject();

  if ($result) {
    return unserialize($result->submission_interface);
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // @TODO Minor. Check if this code below is really useful.
  // Search for alternative accessions.
  $query = db_select('tpps_submission', 's');
  $query->join('tpps_submission_dbxref', 's_dbx',
    's.tpps_submission_id = s_dbx.tpps_submission_id'
  );
  $query->join('chado.dbxref', 'dbx', 'dbx.dbxref_id = s_dbx.dbxref_id');
  $query->fields('s', ['submission_interface']);
  $query->condition('dbx.accession', $accession);
  $query->condition('s.status', 'Approved');
  $query->range(0, 1);
  $result = $query->execute()->fetchObject();

  if ($result) {
    return unserialize($result->submission_interface);
  }
  return FALSE;
}

/**
 * This function updates the form_state of a submission.
 *
 * Additional attributes of the submission record can be updated by populating
 * the options parameter.
 *
 * @param array $form_state
 *   The form_state of the submission.
 */
function tpps_submission_interface_update(array $form_state) {
  if (empty($form_state) || empty($form_state['accession'])) {
    return;
  }
  // @TODO Replace with oneliner in original function tpps_submission_update().
  $form_state['updated'] = REQUEST_TIME;
  $accession = $form_state['accession'];

  $fields = [
    'submission_interface' => tpps_submission_interface_convert($form_state),
  ];
  db_update('tpps_submission')
    ->fields($fields)
    ->condition('accession', $accession)
    ->execute();
}
