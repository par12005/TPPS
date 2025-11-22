<?php

/**
 * @file
 * TPPS Front Page Settings Form.
 */

/**
 * Generates settings form.
 */
function tpps_form_front_settings_form(array $form, array &$form_state) {
  $form['tpps_front_show_others_studies'] = [
    '#type' => 'checkbox',
    '#title' => t('Show others studies in the list of accessions'),
    '#default_value' => variable_get('tpps_front_show_others_studies', TRUE),
    '#description' => t('Default is <strong>unchecked</strong>.'),
  ];
  $form['tpps_front_show_pending_status_mine'] = [
    '#type' => 'checkbox',
    '#title' => t(
      "Show mine studies in '@status_name' status in the list of accessions",
      ['@status_name' => Submission::STATUS_PENDING_APPROVAL]
    ),
    '#default_value' => variable_get('tpps_front_show_pending_status_mine', FALSE),
    '#description' => t('Default is <strong>unchecked</strong>.'),
  ];
  $form['tpps_front_show_approved_status_mine'] = [
    '#type' => 'checkbox',
    '#title' => t(
      "Show mine studies in '@status_name' status in the list of accessions"),
      ['@status_name' => Submission::STATUS_APPROVED]
    ),
    '#default_value' => variable_get('tpps_front_show_approved_status_mine', FALSE),
    '#description' => t('Default is <strong>unchecked</strong>.'),
  ];
  $form['tpps_front_show_pending_status_others'] = [
    '#type' => 'checkbox',
    '#title' => t(
      "Show other user's studies in '@status_name' status in the list of accessions"),
      ['@status_name' => Submission::STATUS_PENDING_APPROVAL]
    ),
    '#default_value' => variable_get('tpps_front_show_pending_status_others', FALSE),
    '#description' => t('Default is <strong>unchecked</strong>.'),
  ];
  $form['#submit'][] = 'tpps_form_front_settings_form_submit';
  return system_settings_form($form);
}
