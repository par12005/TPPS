<?php

require_once 'tpps.submission.api.inc';
require_once 'tpps.queries.api.inc';

/**
 * @file
 */

/**
 * 
 */
function tpps_api_paths() {
  $items = array();

  $items['tpps/api/check_accession/%'] = array(
    'title' => 'TPPS Check Accession',
    'page callback' => 'tpps_api_check_accession',
    'page arguments' => array(3),
    'access callback' => 'tpps_access',
    'access arguments' => array('access content'),
    'delivery callback' => 'drupal_json_output',
    'type' => MENU_CALLBACK,
  );

  $items['tpps/api/submission/%'] = array(
    'title' => 'TPPS Raw Submissions',
    'page callback' => 'tpps_load_submission',
    'page arguments' => array(3),
    'access callback' => 'tpps_access',
    'access arguments' => array('access content'),
    'delivery callback' => 'drupal_json_output',
    'type' => MENU_CALLBACK,
  );

  $items['tpps/api/submissions'] = array(
    'title' => 'TPPS Submissions Query',
    'page callback' => 'tpps_api_submission_query',
    'page arguments' => array(),
    'access callback' => 'tpps_access',
    'access arguments' => array('access content'),
    'delivery callback' => 'drupal_json_output',
    'type' => MENU_CALLBACK,
  );
  $items['tpps/api/submissions/q'] = $items['tpps/api/submissions'];

  $items['tpps/api/submission_info'] = array(
    'title' => 'TPPS Submission Data',
    'page callback' => 'tpps_api_submission_info',
    'page arguments' => array(),
    'access callback' => 'tpps_access',
    'access arguments' => array('access content'),
    'delivery callback' => 'drupal_json_output',
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

/**
 *
 */
function tpps_api_check_accession($accession = NULL) {
  if (empty($accession)) {
    return FALSE;
  }

  return tpps_load_submission($accession) ? TRUE : FALSE;
}