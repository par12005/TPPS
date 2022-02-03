<?php

/**
 * @file
 * Main file for the TPPS API.
 *
 * This file includes the other TPPS API files, defines required paths that
 * TPPS uses for its api, provides a function which can be used to call the API
 * internally, and a function to verify that a provided accession number has a
 * valid TPPS submission associated with it.
 */

require_once 'tpps.submission.api.inc';
require_once 'tpps.queries.api.inc';

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

  $items['tpps/api/debug/state/%'] = array(
    'title' => 'TPPS Show Submission State',
    'page callback' => 'tpps_api_debug_state',
    'page arguments' => array(4),
    'access callback' => 'tpps_access',
    'access arguments' => array('administer tpps module'),
    // 'delivery callback' => 'drupal_json_output',
    'type' => MENU_CALLBACK,
  );

  // tpps/api/refresh_views calls tpps_api_refresh_views which will
  // update the tpps specific views used in particular for details pages
  $items['tpps/api/refresh_views'] = array(
    'title' => 'TPPS Refresh Views',
    'page callback' => 'tpps_api_refresh_views',
    'page arguments' => array(),
    'access arguments' => array('access content'),
    'delivery callback' => 'drupal_json_output',
    'type' => MENU_CALLBACK,
  );  

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

  $items['tpps/api/files/%'] = array(
    'title' => 'TPPS Submission Files Download',
    'page callback' => 'tpps_api_compress_files',
    'page arguments' => array(3),
    'access callback' => 'tpps_access',
    'access arguments' => array('access tpps details'),
    'type' => MENU_NORMAL_ITEM,
  );

  return $items;
}

/**
 * Performs a TPPS API call that will return JSON formatted submission_state
 * 
 * @param string $accession
 *  The accession to search for in the tpps_submissions database table
 * 
 * @return string
 *  Prints the array using print_r which can be seen using viewsource in browser
 *  Example: view-source:https://tgwebdev.cam.uchc.edu/tpps/api/debug/state/TGDR493
 */
function tpps_api_debug_state($accession) {
  $results = chado_query('SELECT submission_state FROM tpps_submission WHERE accession ILIKE :accession', array(
    ':accession' => $accession
  ));
  foreach ($results as $row) {
    print_r(unserialize($row->submission_state));
  }
}



/**
 * Performs a TPPS API call.
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

/**
 * Creates and downloads a .zip file of all relevant files for the submission.
 *
 * @param string $accession
 *   The accession number of the submission with the associated files.
 */
function tpps_api_compress_files($accession = NULL) {
  $state = tpps_load_submission($accession);
  if (empty($state)) {
    throw new Exception('Invalid state accession number');
  }
  ob_end_clean();
  $zip_name = "{$accession}_files.zip";
  if (!file_exists(drupal_realpath(TPPS_TEMP_ZIP))) {
    mkdir(drupal_realpath(TPPS_TEMP_ZIP));
  }
  $zip_loc = drupal_realpath(TPPS_TEMP_ZIP) . '/' . $zip_name;

  $files = $state['files'];
  foreach ($files as $fid) {
    $file_str[] = str_replace(' ', '\\ ', drupal_realpath(file_load($fid)->uri));
  }

  if (file_exists(drupal_realpath($zip_loc))) {
    unlink($zip_loc);
  }

  $file_str = implode(" ", $file_str);
  $cmd = "zip -j $zip_loc $file_str";
  exec($cmd);
  drupal_add_http_header('Content-Type', 'application/zip');
  drupal_add_http_header('Content-Disposition', 'attachment; filename="' . $zip_name . '"');
  drupal_add_http_header('Content-length', filesize($zip_loc));
  drupal_add_http_header('Pragma', "no-cache");
  drupal_add_http_header('Expires', '0');

  readfile("$zip_loc");
  exit;
}


function tpps_api_refresh_views() {
  db_query('refresh materialized view chado.tpps_search_genotype_name;');
  db_query('refresh materialized view chado.tpps_search_genotype_marker;');
  return TRUE;
}