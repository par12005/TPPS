<?php

/**
 * @file
 * TPPS Submission Export Form and related functions.
 *
 * @TODO Minor. Get status of submission in
 * https://tgwebdev.cam.uchc.edu/admin/reports/db/public.tripal_jobs.
 * @TODO Check $form_state['saved_values'] for files but it's structure differs
 * from $form_state['value'] and splitted by pages. But some studies at
 * dev-server (maybe broken) has no 'values' but only 'saves_values' but in
 * reviewed studies files was detected correclty using 'comolete form'.
 */

/**
 * TPPS Study Export Form.
 */
function tpps_submission_export_form(array $form, array &$form_state, $accession = '') {
  global $base_url;
  TppsForm::validateAccession($accession);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Build dropdown list and validate accession.
  $accession_list = SubmissionCache::getAccessionList();
  // Get Submission State.
  if (empty($accession_list[$accession])) {
    $message = t(
      'Submission @accession not found. Please choose from the list below.',
      ['@accession' => $accession]
    );
    drupal_set_message($message, 'warning');
    drupal_goto('tpps/submission');
  }
  $form['tpps_submission_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => $accession_list,
    '#default_value' => $accession ?? array_key_first($accession_list) ?? '',
    '#required' => TRUE,
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Prepare submission for export.
  $submission = new Submission($accession);
  $form['tpps_submission_export_state'] = [
    '#type' => 'textarea',
    '#title' => t('State PHP Array'),
    '#value' => $submission->export(),
    '#rows' => 10,
    '#resizable' => TRUE,
    '#attributes' => ['readonly' => 'readonly'],
    '#states' => [
      'visible' => [
        ':input[name="tpps_submission_accession"]' => ['value' => $accession],
      ],
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add common form elements.
  TppsForm::addSubmissionActionButtons($form);
  TppsForm::autofocus($form, ['tpps_submission_export_state']);

  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('submission_export', $form);
  tpps_add_css_js('theme', $form);
  $form['report']['#states'] = [
    'visible' => [
      ':input[name="tpps_submission_accession"]' => ['value' => $accession],
    ],
  ];
  return $form;
}

/**
 * Form submitter. Redirects to export page when user selected other accession.
 */
function tpps_submission_export_form_submit(array $form, array $form_state) {
  $accession = $form_state['values']['tpps_submission_accession'];
  TppsForm::validateAccession($accession);
  drupal_goto('tpps/submission/' . $accession . '/export');

}

/**
 * Show file detection report.
 *
 * @param array $fid_list
 *   List of File Ids.
 * @param array $files
 *   List of File metadata.
 * @param array $form
 *   Drupal Form API array to add report to.
 */
function tpps_submission_export_file_report(array &$fid_list = [], array &$files = [], array &$form = []) {
  if (!empty($fid_list)) {
    if (empty($files)) {
      $message = t('Study has files but they not found at server.'
        . '<br/>Probably those files must be uploaded again.'
        . '<br />List of File IDs: @list',
        ['@list' => implode(', ', $fid_list)]
      );
      drupal_set_message($message, 'warning');
    }
    else {
      drupal_set_message(t('Files was found and processed. See report below.'));
      TppsForm::addReport($form, [
        'files_prepared_for_import' => [
          'title' => t('List of files prepared for import'),
          'data' => TppsArray::print($files),
        ],
      ]);
    }
  }
  else {
    $message = t('No files found in this submission. Please double-check.');
    drupal_set_message($message, 'warning');
  }
}
