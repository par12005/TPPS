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
    'accession' => $submission->accession,
    'error' => 'Submission not found.',
    'organism_number' => $submission->getOrganismNumber(),
  ];
  $shared_state = $submission->getSharedState();
  $page4_values = $shared_state['saved_values'][TPPS_PAGE_4];
  $organism_number = $submission->getOrganismNumber();
  // Non-empty organism number means that submission exists.
  if ($organism_number) {
    $output = [
      'accession' => $submission->accession,
      'organism_number' => $organism_number,
    ];
    for ($i = 1; $i <= $submission->getOrganismNumber(); $i++) {
      $output['vcf'][$i] = $page4_values['organism-' . $i]['genotype']['files']['vcf'] ?? NULL;
      $output['ref-genome'][$i] = $page4_values['organism-' . $i]['genotype']['ref-genome'] ?? NULL;
    }
  }
  return $output;
}
