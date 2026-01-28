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
  $sharedState = new FormState($submission->sharedState);
  $organism_number = $submission->getOrganismNumber();
  // Non-empty organism number means that submission exists.
  if ($organism_number) {
    $output = [
      'accession' => $submission->accession,
      'organism_number' => $organism_number,
    ];
    for ($i = 1; $i <= $submission->getOrganismNumber(); $i++) {
      $organism_key = 'organism-' . $i;
      if (empty($local_vcf_path = $submission->getLocalVcf($i))) {
        $output['vcf'][$i] = $sharedState->saved_values(
          [TPPS_PAGE_4, $organism_key, 'genotype', 'files', 'vcf']
        );
      }
      else {
        $output['vcf'][$i] = $local_vcf_path;
      }
      $output['ref-genome'][$i] = $sharedState->saved_values(
        [TPPS_PAGE_4, $organism_key, 'genotype', 'ref-genome']
      );
    }
  }
  return $output;
}
