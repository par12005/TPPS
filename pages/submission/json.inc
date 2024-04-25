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
function tpps_submission_json($accession) {
  $url = $_GET['q'];
  $output = [];

  // Get parents.
  $menu_path = 'tpps/submission/' . $accession . '/json/';
  foreach (['state', 'shared_state'] as $type) {
    if (($path = str_replace($menu_path . $type . '/', '', $url)) != $url) {
      break;
    }
  }
  // Get data.
  if ($path != $url) {
    $parents = explode('/', $path);
    $submission = new Submission($accession);
    if ($submission->doesExist()) {
      if ($type == 'state') {
        $data = $submission->state;
      }
      elseif ($type == 'shared_state') {
        $data = $submission->sharedState;
      }
      $output = drupal_array_get_nested_value($data, $parents);
      // Dump at regular page.
      // dpm(print_r($output, 1)); return '  ';
    }
  }
  drupal_json_output($output);
  drupal_exit();
}
