<?php

/**
 * @file
 * Defines functions to be used in the TPPS Details pages.
 */



/**
 * AJAX callback for the search form details_op element.
 *
 * @param array $form
 *   The form being updated.
 * @param array $form_state
 *   The state of the form being updated.
 *
 * @return array
 *   The part of the form to be updated.
 */
function tpps_details_form_callback(array $form, array $form_state) {
  return array(
    'details_type' => $form['details_type'],
    'details_op' => $form['details_op'],
    'details_value' => $form['details_value'],
  );
}

/**
 * Gets list of study's File Ids.
 *
 * @param array $state
 *   Drupal Form State Array of any version.
 *
 * @return array
 *   Returns list of File Ids.
 *
 * @todo Minor. Use $submission->getFileParents().
 * @todo Probably not in use and outdated. Remove if possible.
 * (Called by orphan function tpps_details_study()).
 */
function tpps_details_get_study_file_ids(array $state) {

  $submission = new Submission();
  $submission->setState($state);
  $submission->generateSharedState();
  $submission->save();
  $saved_values = &$submission->sharedState['saved_values'];
  // print_r($saved_values);
  $file_ids = [];
  $page4_file_list = [
    // Phenotype files.
    ['phenotype', 'file'],
    ['phenotype', 'metadata'],
    // Genotype files.
    ['genotype', 'files', 'snps-assay'], // Shared State.
    ['genotype', 'files', 'assay-design'],
    ['genotype', 'files', 'snps-association'],
    ['genotype', 'files', 'snps-pop-struct'],
    ['genotype', 'files', 'snps-kinship'],
    ['genotype', 'files', 'vcf'],
    ['genotype', 'files', 'ssrs'],
    ['genotype', 'files', 'ssrs_extra'],
    ['genotype', 'files', 'other'],
  ];
  for ($j = 1; $j <= ($saved_values[TPPS_PAGE_1]['organism']['number'] ?? 1); $j++) {
    // Page 3.
    $file_ids[] = $saved_values[TPPS_PAGE_3]['tree-accession']['species-' . $j]['file'] ?? NULL;
    // Page 4.
    foreach ($page4_file_list as $parents) {
      $file_ids[] = drupal_array_get_nested_value(
        $saved_values[TPPS_PAGE_4]['organism-' . $j],
        $parents
      );
    }
  }

  // Supplemental files at 'summarypage'.
  for ($i = 1; $i <= 10; $i++) {
    $file_ids[] = $saved_values['summarypage']['files'][$i];
  }

  // Remove empty elements.
  return array_filter($file_ids);
}
