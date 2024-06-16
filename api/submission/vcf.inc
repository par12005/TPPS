<?php

/**
 * @file
 * API Callback. Shows list of VCF file Ids in JSON format.
*/

/**
 * API Callback. Shows list of VCF file Ids in JSON format.
 *
 * @param object $submission
 *   TPPS Submission object.
 */
function tpps_api_submission_vcf_files_list($submission) {
  $output = [
    'accession' => $submision->accession,
    'error' => 'Submission not found.',
    'organism_number' => $submission->getOrganismNumber(),
  ];
  $shared_state = $submission->getSharedState();
  $page4_values = $shared_state['saved_values'][TPPS_PAGE_4];

  $organism_number = $submission->getOrganismNumber();
  // Non-empty organism number means that submission exists.
  if ($organism_number) {
    $output = [
      'accession' => $submision->accession,
      'organism_number' => $organism_number,
    ];
    for ($i = 1; $i <= $submission->getOrganismNumber(); $i++) {
      $output['vcf'][$i] = $page4_values['organism-' . $i]['genotype']['files']['vcf'] ?? NULL;
    }
  }
  drupal_json_output($output);
  drupal_exit();
}
