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
  $menu_path = 'tpps/submission/' . $accession . '/json';
  $parents = explode('/', str_replace($menu_path, '', $url));
  $parents = array_filter($parents);
  if (empty($parents)) {
    drupal_goto('tpps/submission/' . $accession . '/json/state');
  }
  $submission = new Submission($accession);
  if ($submission->doesExist()) {
    if ($parents[1] == 'state') {
      $data = $submission->state;
    }
    elseif ($parents[1] == 'shared_state') {
      $data = $submission->sharedState;
    }
    else {
      drupal_goto('tpps/submission/' . $accession . '/json/state');
    }
    if ($data) {
      array_shift($parents);
      $output = drupal_array_get_nested_value($data, $parents);
    }
  }
  drupal_json_output($output);
  drupal_exit();
}
