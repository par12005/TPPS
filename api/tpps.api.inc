<?php

require_once 'tpps.submission.api.inc';
require_once 'tpps.queries.api.inc';

/**
 * @file
 * Main file for the TPPS API.
 *
 * This file includes the other TPPS API files, defines required paths that
 * TPPS uses for its api, provides a function which can be used to call the API
 * internally, and a function to verify that a provided accession number has a
 * valid TPPS submission associated with it.
 */

/**
 * Defines TPPS API paths.
 *
 * This function is called from the tpps_menu() function and returns the menu
 * items that TPPS will use for its API. All API paths start with `tpps/api/`
 * and return their data as JSON objects. The api paths include:
 *   check_accession/<accession number>
 *   submission/<accession number>
 *   submissions?<arguments>
 *   submission_info?<arguments>
 * Details about each of the paths can be found in the inline documentation of
 * this function, as well as the documentation for each of the functions the
 * API calls.
 *
 * @return array
 *   An array of menu items.
 */
function tpps_api_paths() {
  $items = array();

  // tpps/api/check_accession calls tpps_api_check_accession on a provided
  // accession number. It will return TRUE for accessions with a valid
  // TPPS submission and FALSE for all other accessions.
  $items['tpps/api/check_accession/%'] = array(
    'title' => 'TPPS Check Accession',
    'page callback' => 'tpps_api_check_accession',
    'page arguments' => array(3),
    'access callback' => 'tpps_access',
    'access arguments' => array('access content'),
    'delivery callback' => 'drupal_json_output',
    'type' => MENU_CALLBACK,
  );

  // tpps/api/submission calls tpps_load_submission on a provided accession.
  // If a valid TPPS submission exists for the accession, it will return the
  // form_state of the submission.
  $items['tpps/api/submission/%'] = array(
    'title' => 'TPPS Raw Submissions',
    'page callback' => 'tpps_load_submission',
    'page arguments' => array(3),
    'access callback' => 'tpps_access',
    'access arguments' => array('access content'),
    'delivery callback' => 'drupal_json_output',
    'type' => MENU_CALLBACK,
  );

  // tpps/api/submissions calls tpps_api_submission_query. If no query
  // conditions are provided, it will return information about all of the
  // existing valid TPPS submissions. Otherwise, it will filter the submissions
  // based on the provided conditions. Please see the tpps_api_submission_query
  // function for more details.
  $items['tpps/api/submissions'] = array(
    'title' => 'TPPS Submissions Query',
    'page callback' => 'tpps_api_submission_query',
    'page arguments' => array(),
    'access callback' => 'tpps_access',
    'access arguments' => array('access content'),
    'delivery callback' => 'drupal_json_output',
    'type' => MENU_CALLBACK,
  );

  // tpps/api/submission_info calls tpps_api_submission_info. It will return
  // data on multiple TPPS submissions based on accessions provided in the
  // API query. The data returned can be specified using the data query
  // argument. Please see the tpps_api_submission_info function for more
  // details.
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
 * Performs a TPPS API call
 *
 * This function will perform an internal call to the TPPS API. It constructs a
 * TPPS API url, performs a get request, and decodes the resulting JSON object.
 *
 * @param string $type
 *   The base API path to be called.
 * @param string $id
 *   An id to be passed to the base API path. (Optional)
 * @param array $query
 *   Any additional query arguments to be passed to the API.
 *
 * @return mixed
 *   The decoded JSON object returned by the API.
 */
function tpps_api_call($type, $id = NULL, array $query = array()) {
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
 * Verifies an existing TPPS accession number.
 *
 * If the provided accession has a valid existing TPPS submission, return TRUE.
 * Otherwise, return FALSE.
 *
 * @param string $accession
 *   The accession to be verified.
 *
 * @return bool
 *   TRUE if there exists a valid TPPS submission, otherwise FALSE.
 */
function tpps_api_check_accession($accession = NULL) {
  if (empty($accession)) {
    return FALSE;
  }

  return tpps_load_submission($accession) ? TRUE : FALSE;
}
