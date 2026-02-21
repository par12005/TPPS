<?php

/**
 * @file
 * TPPS Submission Clone Form.
 *
 * @TODO Minor. Implement ability to set accession number. Currently there is
 * no ability to specify accession. Only next one could be used
 * or existing one overritten.
 * @TODO Minor. Set Submission::STATUS_APPROVED status and do necessary changes.
 * Just setting Submission::STATUS_APPROVED status don't allow to run Tripal
 * Job just after import. Required more work with code.
 */

module_load_include('inc', 'tpps', 'includes/imported_studies_list');
module_load_include('inc', 'tpps', 'pages/submission/import');



// @TOdO Set status 'Approved'.

/**
 * TPPS Submission Import Form.
 */
function tpps_submission_clone_form(array $form, array &$form_state, $accession = '') {
  global $base_url;
  TppsForm::validateAccession($accession);

  // Check if submission exists.
  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    $message = t("Submission @accession doesn't exists.", ['@accession' => $accession]);
    drupal_set_message($message, 'warning');
    drupal_goto('tpps/submission');
  }

  $accession_list = SubmissionCache::getAccessionList();
  $form['tpps_submission_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => $accession_list,
    '#default_value' => $accession ?? array_key_first($accession_list) ?? '',
    '#required' => TRUE,
  ];
  // Get Submission State.
  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    $message = t(
      'Submission @accession not found. Please choose from the list below.',
      ['@accession' => $accession]
    );
    drupal_set_message($message, 'warning');
    drupal_goto('tpps/submission');
  }

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
 * Submission Clone Form submitter.
 */
function tpps_submission_clone_form_submit(array $form, array $form_state) {

  $accession = $form_state['values']['tpps_submission_accession'];
  TppsForm::validateAccession($accession);

  $submission = new Submission($accession);
  $export = $submission->export();

  $new_accession = tpps_submission_import([
    // Required:
    'accession' => 'new',
    'export' => trim($export),
    // Optional:
    'upgrade_version' => TRUE,
    'new_status' => Submission::STATUS_APPROVED,
  ]);

  drupal_goto('completed-submission/' . $new_accession);
}
