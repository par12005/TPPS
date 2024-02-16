<?php

/**
 * @file
 * Defines function to initialize a TPPS Submission in the database.
 */

/**
 * This function initializes the TPPS Submission in the database.
 *
 * Creates an entry for the submission in the tpps_submission table and also
 * adds it to the chado.dbxref table.
 *
 * @param array $form_state
 *   The state of the submission form.
 *
 * @return int
 *   The dbxref_id of the submission.
 */
function tpps_init_project(array &$form_state, $accession = NULL) {

  if (empty($accession)) {
    // This code is deprecated since it doesn't work for TGDR numbers over 999
    // Code originally written by Peter Richter and kept as a backup
    // Code deprecated by Rish (12/20/2022) as part of a patch to cater for TGDR
    // numbers over 999

    // $values = array(
    //   'accession' => 'TGDR.*',
    // );

    // $options = array(
    //   'order_by' => array(
    //     'accession' => 'DESC',
    //   ),
    //   'limit' => '1',
    //   'regex_columns' => array('accession'),
    // );

    // $result = chado_select_record('dbxref', array('accession'), $values, $options);

    $accession = 'TGDR' . tpps_init_project_get_next_accession();
  }

  $form_state['dbxref_id'] = tpps_init_project_get_dbxref_id($accession);
  // @todo [VS] It's better to use REQUEST_TIME to have the same timestamp
  // even if processing takes more then 1 second.
  $form_state['created'] = time();
  $form_state['accession'] = $accession;
  $form_state['saved_values']['frontpage']['accession'] = $accession;
  $form_state['status'] = 'Incomplete';
  return $form_state['dbxref_id'];
}

/**
 * Get dbxref_id for new accession.
 *
 * WARNING: Be sure to check if given $accession is not in use yet.
 * See tpps_init_project().
 *
 * @param string $accession
 *   Study accession in format 'TGDRnnnn'.
 *
 * @return int
 *   Returns dbxref_id (example: 10515652) for given accession
 *   and db_id = 95 (which is 'tpps_local_db' Id).
 */
function tpps_init_project_get_dbxref_id($accession) {
  return chado_insert_record(
    'dbxref',
    [
      'db_id' => variable_get('tpps_local_db')->db_id,
      'accession' => $accession,
    ]
  )['dbxref_id'];
}

/**
 * Get next available accession number.
 *
 * @return int
 *   Returns Study accession number (without 'TGDR' prefix).
 */
function tpps_init_project_get_next_accession() {
  // Rish: New patch that sorts based on integer instead of string values.
  // Note: Seems postreSQL can't sort by integer inside string correctly.
  // So it shows, for example '980' instead '12650' when sorting by
  // 'accession' column instead of 'accession_number'.
  $sql = "SELECT *, substr(accession, 5)::INTEGER as accession_number "
    . "FROM chado.dbxref WHERE accession LIKE 'TGDR%' "
    . "ORDER BY accession_number DESC LIMIT 1;";
  $result_query = chado_query($sql, []);
  if (!empty($result_query)) {
    $result = $result_query->fetchObject();
    $accession = $result->accession_number + 1;
    while (strlen($accession) < 3) {
      $accession = "0$accession";
    }
  }
  return $accession ?? '001';
}
