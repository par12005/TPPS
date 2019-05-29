<?php

/**
 * @file
 */

/**
 *
 */
function tpps_init_project($form, &$form_state) {

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

  $dbxref_id = chado_insert_record('dbxref', array(
    'db_id' => array(
      'name' => 'local',
    ),
    'accession' => $accession,
  ));

  $form_state['dbxref_id'] = $dbxref_id['dbxref_id'];
  $form_state['accession'] = $accession;
  $form_state['saved_values']['frontpage']['accession'] = $accession;

  // Project will be created when name is provided, after the first page is complete.
  return $dbxref_id;
}
