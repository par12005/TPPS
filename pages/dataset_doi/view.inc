<?php

/**
 * @file
 * Shows Dataset DOI metadata.
 */

/**
 * Menu callback. Shows Dataset DOI metadata.
 *
 * We do not use /tpps/submission/:accession/zenodo view page because it
 * requires to grant permissions to the regular users to those admin pages.
 *
 * Path: /tpps/dataset_doi/:accession
 */
function tpps_dataset_doi_view_form(array $form, array &$form_state = [], $accession = NULL) {
  $form = $form ?? [];
  if (empty($accession)) {
    drupal_set_message(
      t('Empty study accession. Please choose one from the list below.'),
      'error'
    );
    drupal_goto($_GET['q']);
  }
  module_load_include('inc', 'tpps', 'pages/submission/list');
  tpps_submission_list_validate_accession($accession);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Zenodo Deposition Widget.
  $context = [
    'debug_mode' => FALSE,
    'form' => &$form,
    'accession' => $accession,
    'zenodo_deposit_page_path' => $_GET['q'],
    'zenodo_files_page_path' => $_GET['q'] . '/files',
  ];
  $deposition = ZenodoDepositWidget::add($context);

  // WARNING: This CSS class adds theme and design.
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  return $form;
}
