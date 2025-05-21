<?php

/**
 * @file
 * Shows Dataset DOI metadata.
 */

/**
 * Menu callback. Creates Zenodo Deposit and uploads all the files.
 *
 * Path: /tpps/:accession/dataset_doi/create.
 */
function tpps_dataset_doi_create($accession = NULL) {
  $parent_path = str_replace('/create', '', current_path());
  if (empty($accession)) {
    drupal_set_message(
      t('Empty study accession. Please choose one from the list below.'),
      'error'
    );
    drupal_goto($parent_path);
  }

  // Create Zenodo Deposit if not yet created.
  $submission = new Submission($accession);
  $deposit_id = $submission->getZenodoDepositId();
  if (empty($deposit_id)) {
    ZenodoDeposit::create([
      'accession' => $accession,
    ]);
    // Reload submission object.
    $submission = new Submission($accession);
  }
  $submission->uploadAllFilesZenodo();
  drupal_goto($parent_path);
}
