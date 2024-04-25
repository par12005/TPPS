<?php

/**
 * @file
 * TPPS Submission Tools Settings Form.
 */

/**
 * TPPS Submission Tools Settings Form.
 */
function tpps_submission_settings_form(array $form, array &$form_state) {
  $form['tpps_submission_secret_key'] = [
    '#type' => 'textfield',
    '#title' => t('TPPS Submission Secret Key'),
    '#default_value' => variable_get('tpps_submission_secret_key'),
    '#description' => t('This key must be sent in POST requests to export '
      . 'Submission in JSON format.'),
  ];
  // For searching: 'tpps_dump_max_len_state', 'tpps_dump_max_len_sharedState'.
  foreach (['sharedState', 'state'] as $data_type) {
    $form['tpps_dump_max_len_' . $data_type] = [
      '#type' => 'textfield',
      '#title' => t('"@type" dump max length, chars', ['@type' => $data_type]),
      '#default_value' => variable_get('tpps_dump_max_len_' . $data_type, 300000),
      '#description' => t('Default is "300 000". '
        . "<br />Huge dump lenght: 5M."
        . "<br />Used on Submission View page. When dump's length exceeds this "
        . "limit then it will be shown as a flat dump instead of HTML tables in "
        . "fieldsets"),
    ];
  }
  return system_settings_form($form);
}
