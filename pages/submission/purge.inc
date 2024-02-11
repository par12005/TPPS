<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'includes/common');
module_load_include('inc', 'tpps', 'pages/submission/list');
module_load_include('inc', 'tpps', 'includes/form');

/**
 * Menu callback. TPPS Study View Form.
 */
function tpps_submission_purge_form(array $form, array &$form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);
  $message = t(
    'Are you sure you want to purge Submission @accession?',
    ['@accession' => $accession]
  );
  $cancel_redirect = 'tpps/submission';
  $description = t("Submission @accession and it's files will be removed.",
      ['@accession' => $accession]
    ) . '<br />' . l(
        t('Submistion @accession view page', ['@accession' => $accession]),
        'tpps/submission/' . $accession . '/view'
      );
  $confirm_button_text = t('Purge');
  $cancel_button_text = t('Cancel');
  // @TODO Get accession from URL (if it's safe).
  $form['tpps_submission_accession'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];
  $form = confirm_form(
    $form,
    $message,
    $cancel_redirect,
    $description,
    $confirm_button_text,
    $cancel_button_text
  );
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_purge_form_submit(array $form, array &$form_state) {
  $accession = $form_state['values']['tpps_submission_accession'] ?? NULL;
  tpps_submission_list_validate_accession($accession);
  // Remove study.
  tpps_submission_purge($accession, $remove_files = TRUE);
  cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
  drupal_set_message(
    t("Submission @accession and it's files was removed and cache was cleared.",
      ['@accession' => $accession]
    )
  );
  drupal_goto('tpps/submission');
}
