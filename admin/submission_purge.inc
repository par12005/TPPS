<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'includes/common');

/**
 * Menu callback. TPPS Study View Form.
 */
function tpps_submission_purge_form(array $form, array &$form_state, $accession = '') {
  if (($form_state['input']['op'] ?? NULL) == t('Reset')) {
    $accession = '';
  }
  elseif (empty($accession)) {
    $accession = $form_state['input']['tpps_submission_accession'] ?? '';
  }
  $form['warning'] = [
    '#markup' => '<h3>'
    . t("WARNING: Submission's records in database and files (if they not in use) will be removed.")
    . '</h3>',
  ];
  $form['tpps_submission_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => tpps_submission_get_accession_list(),
    '#default_value' => $accession,
    '#required' => TRUE,
  ];
  $form['tpps_submission_leave_files'] = [
    '#type' => 'checkbox',
    '#title' => t('Leave files'),
    '#default_value' => FALSE,
  ];
  $form['action_purge'] = [
    '#type' => 'submit',
    '#value' => t('Purge'),
    '#submit' => ['tpps_submission_purge_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_submission_purge_form_reset'],
  ];
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_submission_purge_form_reset() {
  $form_state['input']['tpps_submission_accession'] = '';
  $form_state['values']['tpps_submission_accession'] = '';
  drupal_goto('admin/config/tpps/submission/purge');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_purge_form_submit(array $form, array &$form_state) {
  $accession = ($form_state['values']['tpps_submission_accession'] ?? '');
  tpps_submission_purge(
    $accession,
    !($form['values']['tpps_submission_leave_files'] ?? FALSE)
  );
  drupal_set_message(t("Submission @accession and it's files was removed",
    ['@accession' => $accession]
  ));
  cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
  drupal_goto('admin/config/tpps/submission/purge');
}
