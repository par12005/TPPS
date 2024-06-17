<?php

/**
 * @file
 * API Callback. Return's file info in JSON format.
*/

/**
 * API Callback. Return's file info in JSON format.
 *
 * Note: there is an extra elements:
 * 'path': an absolute path in filesystem.
 * 'url': URL with dynamic token to download file. Managed file and uploaded
 * directly to server (outside of the web-root) could be downloaded.
 *
 * @param object $file
 *   Fully loaded managed file object.
 */
function tpps_api_file_get_info($file) {
  global $base_url;
  $output = [];
  if (!$file) {
    $output['error'] = 'File not found.';
  }
  // Get file info.
  if (empty($output['error'])) {
    // Get absolute path.
    $path = $file->uri;
    if (strpos($file->uri, 'public://') !== FALSE) {
      // Managed file.
      $path = drupal_realpath($file->uri);
    }
    $output = $file;
    module_load_include('inc', 'tpps', 'pages/file/get');
    $output->url = tpps_file_get_build_url($fid);
    $output->path = $path;
  }
  drupal_json_output($output);
  drupal_exit();
}
