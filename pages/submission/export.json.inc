<?php

/**
 * @file
 * TPPS Submission Export in JSON format.
 */

/**
 * Shows TPPS Submission Export array in JSON format.
 *
 * @param string $accession
 *   Subission Accession. E.g., 'TGDRxxx'.
 */
function tpps_submission_export_json($accession = '') {
  $output = [];
  $key = variable_get('tpps_submission_secret_key', 'hX99S976sDXtedW4BSf3BGDKLwjR8Gs0');

  // Allowed only POST for live-site and both (POST and GET) for dev-site.
  if (is_live_site()) {
    $received_key = $_POST['key'] ?? NULL;
  }
  else {
    $received_key = $_POST['key'] ?? $_GET['key'] ?? NULL;
  }
  if ($received_key != $key) {
    drupal_json_output($output);
    drupal_exit();
  }

  module_load_include('inc', 'tpps', 'pages/submission/export');
  tpps_submission_list_validate_accession($accession);
  // Get Submission State.
  $submission = new Submission($accession);
  if ($submission->doesExist()) {
    tpps_submission_export_fixes($submission->state);
    // Files.
    $fid_list = tpps_submission_file_get_id_list($submission->state);
    $submission->state['tpps_submission_export_files']
      = tpps_submission_export_get_files_info($fid_list);
    $submission->state['tpps_submission_export_site'] = $base_url;
    $output = $submission->state;
  }
  drupal_json_output($output);
  drupal_exit();
}
