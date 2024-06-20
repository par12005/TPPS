<?php

/**
 * @file
 * Get TPPS Submission File.
 */

/**
 * Menu callback. Get managed file by FileId.
 *
 * @param object $file
 *   Fully loaded managed file object.
 */
function tpps_api_file_get($file) {
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
