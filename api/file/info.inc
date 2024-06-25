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
  // Get absolute path.
  if (
    strpos($file->uri, 'public://') !== FALSE
    || strpos($file->uri, 'temporary://') !== FALSE
  ) {
    // Managed file.
    $file->path = tpps_realpath($file->uri);
  }
  else {
    // Manually uploaded file.
    $file->path = $file->uri;
  }

  // Build URL to download file using API.
  $token = trim(variable_get('tpps_api_token'));
  $file->url = "$base_url/api/file/$file->fid/get/$file->filename?token=$token";

  return $file;
}
