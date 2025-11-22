<?php

/**
 * @file
 * Shows Dataset DOI files.
 */

/**
 * Menu callback. Shows Dataset DOI metadata.
 *
 * Path: /tpps/:accession/dataset_doi/files
 *
 * Note:
 * We do not use /tpps/submission/:accession/zenodo view page because it
 * requires to grant permissions to the regular users to those admin pages.
 */
function tpps_dataset_doi_files_form(array $form, array &$form_state = [], $accession = NULL) {
  $form = $form ?? [];

  // Check submission accession.
  if (empty($accession)) {
    drupal_set_message(t('Empty accession.'), 'error');
    drupal_goto('<front>');
  }
  module_load_include('inc', 'tpps', 'pages/submission/list');
  tpps_submission_list_validate_accession($accession);

  // Zenodo Deposition Widget.
  $path = 'tpps/' . $accession . '/dataset-doi';
  $context = [
    'debug_mode' => FALSE,
    'form' => &$form,
    'accession' => $accession,
    'zenodo_deposit_page_path' => $path,
    'zenodo_files_page_path' => $path . '/files',
  ];
  ZenodoFilesWidget::add($context);
  return $form;
}
