<?php

/**
 * @file
 */

/**
 * 
 */
function tpps_api_submission_query() {
  $conditions = drupal_get_query_parameters();//NULL, array());

  $submissions = tpps_api_submission_std_query($conditions);
  if (empty($conditions)) {
    return $submissions;
  }

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

  return $submissions;
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
