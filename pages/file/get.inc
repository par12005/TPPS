<?php

/**
 * @file
 * Get TPPS Submission File.
 */

/**
 * Menu callback. Return's Study's file.
 *
 * @param int $fid
 *   Managed Drupal File Id.
 * @param string $token
 *   Secure Token. See tpps_file_generate_token() for more details.
 *
 * @return mixed
 *   Returns file content so it could be downloaded.
 */
function tpps_file_get($fid, $token) {
  $file = tpps_file_load($fid);
  if (!$file) {
    drupal_set_message(t('File not found'), 'error');
    drupal_not_found();
  }
  // Main security check.
  if (variable_get('tpps_submission_file_check_token', TRUE)) {
    module_load_include('inc', 'tpps', 'includes/submission_file');
    if ($token != tpps_file_generate_token($file)) {
      drupal_access_denied();
    }
  }
  // Send file to browser.
  if (strpos($file->uri, 'public://') === FALSE) {
    // File outside webroot.
    // https://stackoverflow.com/questions/36499625.
    header('Content-Disposition: attachment; filename="' . $file->filename . '"');
    header("Content-Length: " . $file->filesize);
    header("Content-Type: application/octet-stream");
    readfile(realpath($file->uri));
  }
  else {
    // Managed file.
    file_transfer($file->uri, file_get_content_headers($file));
  }
}

/**
 * Generates secure URL to download file.
 *
 * To use:
 * $fid = 12979;
 * module_load_include('inc', 'tpps', 'pages/file/get');
 * dpm(tpps_file_get_build_url($fid));
 *
 * @param int $fid
 *   File Id.
 *
 * @return string
 *   Returns URL.
 */
function tpps_file_get_build_url($fid) {
  global $base_url;
  $file = tpps_file_load($fid);
  $token = tpps_file_generate_token($file);
  return "$base_url/tpps/file/$fid/get/$token/$file->filename";
}
