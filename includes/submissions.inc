<?php

/**
 * @file
 * 
 */

/**
 * 
 */
function tpps_load_submission($accession, $state = TRUE){
  $result = db_select('tpps_submission', 's')
    ->fields('s')
    ->condition('accession', $accession)
    ->range(0, 1)
    ->execute()->fetchObject();
  if ($state)
    return unserialize($result->submission_state);
  else
    return $result;
}

/**
 * 
 */
function tpps_create_submission($state, $uid){
  $values = array(
    'uid' => $uid,
    'status' => 'Incomplete',
    'accession' => $state['accession'],
    'dbxref_id' => $state['dbxref_id'],
    'submission_state' => serialize($state)
  );

  dpm($state);
  db_insert('tpps_submission')
    ->fields($values)
    ->execute();
}
