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

function tpps_api_new_study_pipeline_finished() {
  $results = [];
  $token = trim(variable_get('tpps_api_token'));
  if ($token == check_plain($_GET['token'])) {
    $results['get'] = $_GET;
    $results['status'] = 'success';
  }
  else {
    $results['status'] = 'Incorrect token';
  }

  drupal_json_output($results);
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
  $submission = new Submission($accession);
  return $submission->doesExist();
}

/**
 * Creates and downloads a .zip file of all relevant files for the submission.
 *
 * @param string $accession
 *   The accession number of the submission with the associated files.
 */
function tpps_api_compress_files($accession = NULL) {
  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    throw new Exception('Invalid accession of submission.');
  }
  ob_end_clean();
  $zip_name = "{$accession}_files.zip";
  if (!file_exists(tpps_realpath(TPPS_TEMP_ZIP))) {
    mkdir(tpps_realpath(TPPS_TEMP_ZIP));
  }
  $zip_loc = tpps_realpath(TPPS_TEMP_ZIP) . '/' . $zip_name;

  foreach (array_keys($submission->getFileIdList()) as $fid) {
    $file_str[] = str_replace(' ', '\\ ', tpps_realpath(file_load($fid)->uri));
  }

  if (file_exists(tpps_realpath($zip_loc))) {
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

/**
 * Refreshes materialized views.
 *
 * @return bool
 *   Returns TRUE.
 */
function tpps_api_refresh_views() {
  db_query('refresh materialized view chado.tpps_search_genotype_name;');
  db_query('refresh materialized view chado.tpps_search_genotype_marker;');
  return TRUE;
}
