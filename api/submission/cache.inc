<?php

/**
 * @file
 * TPPS Submission Export in JSON format.
 */

/**
 * Shows TPPS Submission Cache in JSON format.
 *
 * @param string $accession
 *   TPPS Submission accession. E.g., 'TGDRxxxx'.
 */
function tpps_api_submission_cache($accession) {
  $url = $_GET['q'];
  $output = [];

  $menu_path = 'api/submission/' . $accession . '/cache';
  $parents = explode('/', str_replace($menu_path, '', $url));

  // Get cached Submission.
  $cached_submission = SubmissionCache::get($accession);
  if (empty($output = $cached_submission)) {
    $output = 'NULL';
  }
  // Get sub-elements.
  if (!empty($parents = array_filter($parents))) {
    $key_exists = FALSE;
    $output = TppsArray::get($cached_submission, $parents, $key_exists);
    if ($key_exists && is_null($output)) {
      $output = 'NULL';
    }
  }
  return $output;
}
