<?php

/**
 * @file
 * Shows list of Zenodo Deposition Files.
 */

/**
 * Shows list of Zenodo Deposition Files in JSON format for given Submission.
 *
 * @param object $submission
 *   TPPS Submission object.
 */
function tpps_api_submission_zenodo_files($submission) {
  if (empty($deposit_id = $submission->getZenodoDepositId())) {
    return ['error' => 'Submission has no Zenodo Deposition.'];
  }
  $response = ZenodoFile::list([
    'deposit_id' => $deposit_id,
  ]);
  return $response;
}
