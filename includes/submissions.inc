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
  // For newer TPPS submissions, $result should be populated.
  if ($result){
    if ($state)
      return unserialize($result->submission_state);
    else
      return $result;
  }
  // Provide legacy support for older TPPS submissions.
  else {
    global $user;
    $state = variable_get('tpps_incomplete_' . $user->mail . $accession, NULL);
    if (!empty($state))
      return $state;
    else
      return variable_get('tpps_complete_' . $user->mail . $accession, NULL);
  }
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

  db_insert('tpps_submission')
    ->fields($values)
    ->execute();
}

/**
 *
 */
function tpps_update_submission($state, array $options = array()){
  $options['submission_state'] = serialize($state);

  db_update('tpps_submission')
    ->fields($options)
    ->condition('accession', $state['accession'])
    ->execute();
}

/**
 *
 */
function tpps_delete_submission($accession){
  $dbxref_id = tpps_load_submission($accession, FALSE)->dbxref_id;
  db_delete('tpps_submission')
    ->condition('accession', $accession)
    ->execute();
  db_delete('chado.dbxref')
    ->condition('dbxref_id', $dbxref_id)
    ->execute();
}
