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

    // Rish: New patch that sorts based on integer instead of string values
    $result_query = chado_query("SELECT *, substr(accession, 5)::INTEGER as accession_number " . 
      "FROM chado.dbxref WHERE accession LIKE 'TGDR%' "  .
      "ORDER BY accession_number DESC LIMIT 1;", []);

    if (empty($result_query)) {
      $accession = 'TGDR001';
    }
    else {
      $result = []; // create an empty array
      $result[] = $result_query->fetchObject(); // add the first record found into result
      $accession = substr($result[0]->accession, 4) + 1;
      while (strlen($accession) < 3) {
        $accession = "0$accession";
      }
      $accession = "TGDR$accession";
    }
  }

  $dbxref_id = chado_insert_record('dbxref', array(
    'db_id' => variable_get('tpps_local_db')->db_id,
    'accession' => $accession,
  ));

  $form_state['dbxref_id'] = $dbxref_id['dbxref_id'];
  $form_state['created'] = time();
  $form_state['accession'] = $accession;
  $form_state['saved_values']['frontpage']['accession'] = $accession;
  $form_state['status'] = 'Incomplete';

  return $dbxref_id;
}
