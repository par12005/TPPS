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

  $cache = SubmissionCache::get();
  if (empty($output = $cache[$accession])) {
    $output = 'NULL';
  }
  return $output;
}
