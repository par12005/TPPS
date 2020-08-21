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
    $values = array(
      'accession' => 'TGDR.*',
    );

    $options = array(
      'order_by' => array(
        'accession' => 'DESC',
      ),
      'limit' => '1',
      'regex_columns' => array('accession'),
    );

    $result = chado_select_record('dbxref', array('accession'), $values, $options);

    if (empty($result)) {
      $accession = 'TGDR001';
    }
    else {
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
