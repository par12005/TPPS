<?php

/**
 * @file
 */

/**
 * 
 */
function tpps_api_submission_query() {
  $conditions = drupal_get_query_parameters();

  $submissions = tpps_api_submission_std_query($conditions);
  if (!empty($conditions)) {
    foreach ($conditions as $key => $val) {
      $pids = array();
      $vals = explode(',', $val);
      $function_name = "tpps_api_{$key}_pid_query";

      if (!function_exists($function_name)) {
        return array();
      }

      $query = $function_name($vals);
      while (($result = $query->fetchObject())) {
        $pids[] = $result->project_id;
      }

      if (empty($pids)) {
        return array();
      }

      foreach ($submissions as $num => $state) {
        if (array_search($state['ids']['project_id'], $pids) === FALSE) {
          unset($submissions[$num]);
        }
      }
    }
  }

  $results = array();
  foreach ($submissions as $submission) {
    $results[] = array(
      'accession' => $submission['accession'],
      'dbxref_id' => $submission['dbxref_id'],
      'file_info' => $submission['file_info'] ?? NULL,
      'file_rank' => $submission['file_rank'] ?? NULL,
      'ids' => $submission['ids'] ?? NULL,
      'job_id' => $submission['job_id'] ?? NULL,
      'data' => $submission['saved_values'],
      'stage' => $submission['stage'],
      'status' => $submission['status'],
      'submitting_uid' => $submission['submitting_uid'] ?? NULL,
      'tpps_type' => $submission['tpps_type'] ?? NULL,
    );
  }

  return $results;
}

/**
 * 
 */
function tpps_api_submission_std_query(&$conditions) {
  $args = array();
  $valid_args = tpps_table_columns('tpps_submission');
  foreach ($conditions as $key => $val) {
    if (array_search($key, $valid_args)) {
      $args[$key] = explode(',', $val);
      unset($conditions[$key]);
    }
  }

  $states = tpps_load_submission_multiple($args);
  $results = array();
  foreach ($states as $state) {
    if (!isset($state['ids']['project_id'])) {
      continue;
    }
    $results[$state['ids']['project_id']] = $state;
  }
  return $results;
}

/**
 * 
 */
function tpps_api_submission_info() {
  $params = drupal_get_query_parameters();
  if (empty($params['accessions'])) {
    return NULL;
  }

  if (empty($params['data'])) {
    return NULL;
  }

  $entities = FALSE;
  if (array_key_exists('entities', $params)) {
    $entities = $params['entities'];
  }

  $accessions = explode(',', $params['accessions']);
  $states = tpps_load_submission_multiple(array('accession' => $accessions));
  $pids = array();
  foreach ($states as $state) {
    $pids[] = $state['ids']['project_id'];
  }
  unset($states);
  $data_types = explode(',', $params['data']);
  $results = array();
  foreach ($data_types as $type) {
    $function_name = "tpps_api_project_{$type}_query";
    if ($type == 'project') {
      $function_name = "tpps_api_project_query";
    }

    if (!function_exists($function_name)) {
      return NULL;
    }

    $results[$type] = $function_name($pids, $entities);
  }

  return $results;
}
