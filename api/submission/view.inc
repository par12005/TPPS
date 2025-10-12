<?php

/**
 * @file
 * TPPS Submission Export in JSON format.
 */

/**
 * Shows TPPS Submission Data in JSON format.
 *
 * @param object $submission
 *   Fully loaded object of the 'Submission' class.
 */
function tpps_api_submission_view($submission) {
  $url = $_GET['q'];
  $output = [];

  // Get parents.
  $menu_path = 'api/submission/' . $submission->accession . '/view';
  $parents = explode('/', str_replace($menu_path, '', $url));
  $parents = array_filter($parents);
  if (empty($parents)) {
    drupal_goto($menu_path . '/state');
  }
  if ($submission->doesExist()) {
    if ($parents[1] == 'state') {
      $data = $submission->state;
    }
    elseif ($parents[1] == 'shared_state') {
      $data = $submission->sharedState;
    }
    else {
      drupal_goto($menu_path . '/state');
    }
    if ($data) {
      array_shift($parents);
      $key_exist = FALSE;
      $output = drupal_array_get_nested_value($data, $parents, $key_exist);
      if ($key_exist && is_null($output)) {
        $output = 'NULL';
      }
    }
  }
  return $output;
}
