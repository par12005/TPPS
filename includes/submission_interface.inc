<?php

/**
 * @file
 * Functions for manipulation of TPPS Submissions.
 *
 * ----------------------------------------------------------------------------
 * Concept.
 *
 * Submission interface initially is a copy of the Submission Form State
 * modified for Submission Pipeline to always have the same structure and
 * data format (type). For this each Submission Form State has version
 * (default version is '1') and function tpps_submission_interface_generate()
 * will convert existing $form_state to the $submission_interface which
 * will be stored in public.tpps_submissions database table.
 *
 * ----------------------------------------------------------------------------
 * Storage.
 *
 * 1. Serialized SIA is stored in DB (tpps_submission.submission_interface).
 * 2. PHP code always uses unserialized version if SIA.
 *
 * ----------------------------------------------------------------------------
 * API:
 * Note: functions has prefix 'tpps_submission_interface' (represented by '*').
 *   *_load() - Loads Submission Interface Array from DB. Uses cache.
 *   *_save() - Saves given Submission Interface Array to DB.
 *   *_generate() - Creates SIA and returns it (DB won't be updated).
 *   *_new() - Creates SIA, saves to DB and returns it.
 *   *_remove($accession) - Removes SIA field in DB.
 *
 * ----------------------------------------------------------------------------
 * Usage.
 *   module_load_include('inc', 'tpps', 'includes/submission_interface');
 *
 * Since submission data could be updated from admin's pages and they not use
 * a single function but update DB table directly it's safer for now to update
 * Submission Interface Array on each change of Submission State Array.
 *
 * Study statuses previously stored serialized in column tpps_submission.submission_state.
 * So we will store status for now inside Submission Interface Array as well.
 *
 * ----------------------------------------------------------------------------
 * Dates.
 *
 * There are several dates which also was stored in $submission_interface.
 *  updated
 *  created
 *  approved
 *  completed
 *  loaded
 *
 *  @todo Better to store them not inside serialized array.
 *  Create separate table for study statuses:
 *  Accession | Status Id | Timestamp
 *
 * ----------------------------------------------------------------------------
 * Usage tips.
 *
 * To exclude only 'submission_interface' from the list of query's fields:
 *   $query = db_select('tpps_submission', 's')
 *     // Exclude 'submission_interface' column.
 *     ->fields('s', array_diff(
 *       array_keys(drupal_get_schema('tpps_submission')['fields']),
 *       ['submission_interface']
 *     ));
 *
 * Exclude 'submission_interface' from chado_query().
 *   // Exclude 'submission_interface' column.
 *   $fields = implode(', ', array_diff(
 *     array_keys(drupal_get_schema('tpps_submission')['fields']),
 *     ['submission_interface']
 *   ));
 *   $all_submissions = chado_query("SELECT $fields FROM public.tpps_submission;");
 *
 * ----------------------------------------------------------------------------
 * @todo Use 'study_state' or even 'study' instead of 'Submission Interface'
 * because 'Submission Interface' is a name of the feature but not name of
 * the array with data.
 *
 * @todo See submit_all.php:75. There 'status' is stored twice:
 * 1. $submission_interface['status'].
 * 2. public.tpps_submission.status.
 * Check if $submission_interface['status'] is used anywhere and replace with
 * public.tpps_submission.status. Then update this code to use
 * Submission Interface only. Do not use tpps_update_submission() but
 * update db table directly to avoid $submission_interface update.
 *
 * @TODO Remove column 'Submission Interface' but it stores results of
 * study processing which must be then stored in State and all the pages
 * must be updated to use State instead of Interface.
 * Generation of Submision Interface takes about 25-30 msec so there is no
 * sense to store it in database because it must be updated each time when
 * Submission State changes.
 */

/**
 * Checks if given array is Submission Interface Array.
 *
 * @param array $form_state
 *   Form State array.
 *
 * @return bool
 *   Returns TRUE if given array is a Submission Interface Array.
 *   Returns FALSE otherwise.
 */
function tpps_form_is_submission_interface(array $form_state) {
  return (($form_state['is_submission_interface'] ?? NULL) == TRUE);
}

/**
 * Generates new Submission Interface by Submission Accession.
 *
 * @param string $accession
 *   Study Accession.
 *
 * @return mixed
 *   Returns array with datastructure prepared for TPPS pipeline
 *   (submit_all.php script).
 *   Returns empty array if $form_state is empty too.
 */
function tpps_submission_interface_new($accession) {
  $submission = new Submission($accession);
  return $submission->createSharedState();
}

/**
 * Removes Submission Interface Array.
 *
 * @param string $accession
 *   Study Accession. E.g., 'TGDR12345'.
 */
function tpps_submission_interface_remove($accession) {
  $submission = new Submission($accession);
  $submission->removeSharedState();
}

/**
 * Removes and create from scratch Submission Interface.
 *
 * @param string $accession
 *   Study accession.
 */
function tpps_submistion_interface_recreate($accession) {
  $submission = new Submission($accession);
  $submission->recreateSharedState();
}

/**
 * Creates Submission Interface from $form_state array.
 *
 * WARNING:
 * Use tpps_submission_interface_save() to store interface in DB.
 *
 * @param array $form_state
 *   Drupal Form State array.
 *
 * @return mixed
 *   Returns array with datastructure prepared for TPPS pipeline
 *   (submit_all.php script).
 *   Returns empty array if $form_state is empty too.
 */
function tpps_submission_interface_generate(array $form_state = []) {
  $submission = new Submission();
  $submission->state = $form_state;
  $submission->generateSharedState();
  return $submission->sharedState;
}

/**
 * Gets existing or creates new Submission Interface for given study.
 *
 * Note: Forked tpps_load_submission(). Code was improved.
 *
 * @param string $accession
 *   The accession of the submission in format 'TGDRxxxx'.
 *
 * @return array
 *   Returns Submission Interface Array which is ready for pipeline processing.
 *   Returns empty array if Submission wan't found.
 */
function tpps_submission_interface_load($accession) {
  // @TODO Update related code.
  $submission = new Submission($accession);
  $submission->loadSharedState();
  return $submission->sharedState;
}

/**
 * Updates Submission Interface using given array.
 *
 * By default given array will overwrite SIA stored in DB but it could contain
 * extra data if processing was completed and to store those data we could
 * merge existing SIA with new values. To do this set variable:
 *  variable_set('tpps_submission_interface_merge_on_update', TRUE);
 * or use admin/config/tpps/form/main
 * to check parameter 'Merge Submission Interface on update'.
 *
 * To update submission's status:
 *   tpps_update_submission_info($accession, $data);
 * To update submission's Form State:
 *   tpps_update_submission_state($form_state);
 *
 * To remove submission Interface Array:
 *   tpps_submission_interface_remove($accession);
 * WARNING: do not use 'phppgadmin' because it can't edit serialized data.
 *
 * @param array $interface
 *   The Submission Interface Array.
 * @param string $status
 *   Optional. Study processing status.
 *   This value will be added to the Submission Interface Array and to
 *   public.tpps_submission.status column.
 *
 * @return mixed
 *   Returns Submission Interface Array which will have updated 'status' and
 *   'updated' fields.
 *   Returns NULL if $form_state or Study accession is empty.
 */
function tpps_submission_interface_save(array &$interface, $status = NULL) {
  // @WARNING: Reference!
  $submission = new Submission();
  $submission->sharedState = $interface;
  $submission->saveSharedState($status);
  // Note: arrays updates during save.
  $interface = $submission->sharedState;
}

/**
 * Reduce size of the Submission Interface array.
 *
 * Items not used by pipeline processing script will be removed.
 *
 * @param array $interface
 *   Submission Interface.
 */
function tpps_submission_interface_purify(array &$interface) {
  // WARNING: Reference!
  $submission = new Submission();
  $submission->sharedState = $interface;
  $submission->purifySharedState();
  // Note: arrays updates during save.
  $interface = $submission->sharedState;
}
