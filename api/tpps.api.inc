<?php

require_once 'tpps.submission.api.inc';

/**
 * 
 */

/**
 * 
 */
function tpps_api_paths() {
  $items = array();

  $items['tpps/api/submission_raw/%'] = array(
    'title' => 'TPPS Raw Submissions',
    'page callback' => 'tpps_api_submission',
    'page arguments' => array(3, TRUE),
    'access callback' => 'tpps_access',
    'access arguments' => array('access content'),
    'type' => MENU_CALLBACK,
  );

  return $items;
}

/**
 * 
 */
function tpps_api_call($type, $id = NULL, $query = array()) {
  global $base_url;
  $path = $base_url . "/tpps/api/$type";
  if (!empty($id)) {
    $path .= "/$id";
  }
  if (!empty($query)) {
    $args = array();
    foreach ($query as $key => $val) {
      $args[] = "$key=$val";
    }
    $path .= "?" . implode("&", $args);
  }
  $response = file_get_contents($path);
  return json_decode($response);
}
