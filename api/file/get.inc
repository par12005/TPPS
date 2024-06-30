<?php

/**
 * @file
 * Get TPPS Submission File.
 */

/**
 * Menu callback. Get managed file by FileId.
 *
 * For managed files could be used code below but it doesn't work for files
 * outside webroot:
 * file_transfer($file->uri, file_get_content_headers($file));
 * So it's better to use the same code for both cases instead.
 *
 * @param object $file
 *   Fully loaded managed file object.
 */
function tpps_api_file_get($file) {
  // Send file to browser.
  // https://stackoverflow.com/questions/36499625.
  header('Content-Disposition: attachment; filename="' . $file->filename . '"');
  header("Content-Length: " . $file->filesize);
  header("Content-Type: application/octet-stream");
  readfile(tpps_realpath($file->uri));
}
