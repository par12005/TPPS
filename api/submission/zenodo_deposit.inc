<?php

/**
 * @file
 * Shows list of Zenodo Deposition data.
 */

/**
 * Shows list of Zenodo Deposition in JSON format for given Submission.
 *
 * @param object $submission
 *   TPPS Submission object.
 */
function tpps_api_submission_zenodo_deposit($submission) {
  if (empty($deposit_id = $submission->getZenodoDepositId())) {
    return ['error' => 'Submission has no Zenodo Deposition.'];
  }

  //dpm($deposit_id, 'deposit_id');
  $response = ZenodoDeposit::retrieve([
    'deposit_id' => $deposit_id,
  ]);
  //dpm(print_r($response, 1));
  return $response;
}
