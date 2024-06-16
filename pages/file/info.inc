<?php

/**
 * @file
 * Get File Info.
 *
 * Including absolute path and download URL.
 */

/**
 * Menu callback. Return's file info.
 *
 * @param int $fid
 *   Managed File Id.
 *
 * @return mixed
 *   Returns file content so it could be downloaded.
 */
function tpps_file_get_info($fid) {
  global $base_url;
  $file = tpps_file_load($fid);
  if (!$file) {
    $output['error'] = 'File not found.';
  }
  // Get file info.
  //if (empty($output['error'])) {
  //  // Get absolute path.
  //  if (strpos($file->uri, 'public://') === FALSE) {
  //    // File outside webroot.
  //    $path = realpath($file->uri);
  //  }
  //  else {
  //    // Managed file.
  //    $path => '/core/labs/Wegrzyn/VCF/tpps_genotype_web_uploads/' . $file->uri;
  //  }

  //  $output = (array) $file;
  //  //module_load_include('inc', 'tpps', 'pages/file/get');
  //  //$output['url'] = tpps_file_get_build_url($fid);
  //  //$output['path'] = '/core/labs/Wegrzyn/VCF/tpps_genotype_web_uploads/' . $path;
  //}

  drupal_json_output($output);
  drupal_exit();
}
