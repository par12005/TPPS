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
  if (isset($form_state['saved_values']['confirmed'])) {
    $accession = (
      $form_state['saved_values']['tpps_submission_accession']
      ?? $form_state['values']['tpps_submission_accession'] ?? ''
    );
    $message = t(
      'Are you sure you want to purge Submission @accession?',
      ['@accession' => $accession]
    );
    $cancel_redirect = 'admin/config/tpps/submission/purge';
    $description = t('Submission related records in database and also linked '
      . 'files will be removed. '
      . '<br />If files are used by other study or module they will '
      . 'be left until you have unchecked "Leave files" on previous step.')
      . '<br />'
      . l(
          t('Submistion @accession view page', ['@accession' => $accession]),
          'admin/config/tpps/submission/view/' . $accession
        );
    $confirm_button_text = t('Yes, Purge Submission');
    $cancel_button_text = t('No, Leave Submission');
    return confirm_form(
      $form,
      $message,
      $cancel_redirect,
      $description,
      $confirm_button_text,
      $cancel_button_text
    );
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  else {
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
      '#default_value' => check_plain($accession),
      '#required' => TRUE,
    ];
    $form['tpps_submission_leave_files'] = [
      '#type' => 'checkbox',
      '#title' => t('Leave files'),
      '#default_value' => FALSE,
    ];
    $form['tpps_submission_skip_confirmation'] = [
      '#type' => 'checkbox',
      '#title' => t('Skip purge confirmation'),
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
  if (
    $form_state['saved_values']['confirmed'] ?? FALSE
    || $form_state['values']['tpps_submission_skip_confirmation'] ?? FALSE
  ) {
    // Confirmed or Skip Confirmation was chosen.
    $accession = (
      $form_state['saved_values']['tpps_submission_accession']
      ?? $form_state['values']['tpps_submission_accession'] ?? ''
    );
    $remove_files = !(
      $form['saved_values']['tpps_submission_leave_files']
      ?? $form['values']['tpps_submission_leave_files'] ?? 0
    );
    tpps_submission_purge($accession, $remove_files);
    cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
    drupal_set_message(
      t("Submission @accession and it's files was removed and cache was cleared.",
        ['@accession' => $accession]
      )
    );
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  else {
    // Not confirmed yet.
    $form_state['saved_values']['tpps_submission_accession']
      = ($form_state['values']['tpps_submission_accession'] ?? '');
    $form_state['saved_values']['tpps_submission_leave_files']
      = (bool) !($form['saved_values']['tpps_submission_leave_files'] ?? 0);
    // Rebuild form to show confirmation page.
    $form_state['saved_values']['confirmed'] = TRUE;
    $form_state['rebuild'] = TRUE;
  }
}
