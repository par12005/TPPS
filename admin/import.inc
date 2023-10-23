<?php

/**
 * @file
 * TPPS Study Import Form and related functions.
 */

/**
 * TPPS Study Import Form.
 */
function tpps_study_import_form(array $form, array &$form_state) {
  $form['tpps_import_new_accession'] = [
    '#type' => 'textfield',
    '#title' => t('New Accession'),
    '#default_value' => '',
    '#required' => FALSE,
    '#description' => t(
      'Default: files/<br />'
      . "Path to folder where study's files are stored."
    ),
  ];
  // @TODO Minor. Use file field instead.
  $form['tpps_import_submission_state'] = [
    '#type' => 'textarea',
    '#title' => t('Submission State'),
    '#default_value' => $form_state['values']['tpps_import_submission_state'] ?? '',
    '#required' => TRUE,
  ];
  $form['action_import'] = [
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => ['tpps_study_import_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_study_import_form_reset'],
  ];
  return $form;
}

/**
 * Resets TPPS Import Form.
 */
function tpps_study_import_form_reset(array $form, array &$form_state) {
  drupal_goto('admin/config/tpps/migration/import');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_study_import_form_submit(array $form, array &$form_state) {
  // Remove useless data from the submission state.
  if ($state_raw = $form_state['values']['tpps_import_submission_state']) {
    eval('$state = ' . $state_raw . ';');
    $state['values'] = $state['saved_values'];
    if (!empty($new_accession = $form_stat['values']['tpps_import_new_accession'])) {
      $form_state['values']['frontpage']['accession'] = $new_accession;
      // Check also $form_state['input'] and $form_state['completed_form'].
    }


    // @TODO Download files.
    // @TODO Change fid.


    dpm(
      drupal_form_submit('tpps_main', $state)
    );
    dpm($state);
  }
}
