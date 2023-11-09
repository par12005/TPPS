<?php

/**
 * @file
 * TPPS Front Page Settings Form.
 */

/**
 * Generates settings form.
 */
function tpps_form_front_settings_form(array $form, array &$form_state) {
  $form['tpps_front_cache_accession_list'] = [
    '#type' => 'checkbox',
    '#title' => t('Cache list of accessions'),
    '#default_value' => variable_get('tpps_front_cache_accession_list', TRUE),
    '#description' => t('Default is <strong>checked</strong>.<br />'
      . "When checked list of accessions will be cached using custom bin 'tpps'."
      . "<br />Execution time (95 items):"
      . "<br />No caching 412.081 msec"
      . "<br />Caching:     1.809 msec"
    ),
  ];
  $form['tpps_front_show_others_studies'] = [
    '#type' => 'checkbox',
    '#title' => t('Show others studies in the list of accessions'),
    '#default_value' => variable_get('tpps_front_show_others_studies', TRUE),
    '#description' => t('Default is <strong>unchecked</strong>.'),
  ];
  $form['tpps_front_show_pending_status_mine'] = [
    '#type' => 'checkbox',
    '#title' => t("Show mine studies in 'Pending Approval' status in the list of accessions"),
    '#default_value' => variable_get('tpps_front_show_pending_status_mine', FALSE),
    '#description' => t('Default is <strong>unchecked</strong>.'),
  ];
  $form['tpps_front_show_pending_status_others'] = [
    '#type' => 'checkbox',
    '#title' => t("Show other user's studies in 'Pending Approval' status in the list of accessions"),
    '#default_value' => variable_get('tpps_front_show_pending_status_others', FALSE),
    '#description' => t('Default is <strong>unchecked</strong>.'),
  ];
  $form['#submit'][] = 'tpps_form_front_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Form submitter.
 */
function tpps_form_front_settings_form_submit(array $form, array &$form_state) {
  // Clear cache on change only.
  if (
    ($form_state['values']['tpps_front_cache_accession_list'] ?? NULL)
      != variable_get('tpps_front_cache_accession_list', TRUE)
    || ($form_state['values']['tpps_front_show_pending_status_mine'] ?? NULL)
      != variable_get('tpps_front_show_pending_status_mine', FALSE)
    || ($form_state['values']['tpps_front_show_pending_status_others'] ?? NULL)
      != variable_get('tpps_front_show_pending_status_others', FALSE)
    || ($form_state['values']['tpps_front_show_others_studies'] ?? NULL)
      != variable_get('tpps_front_show_others_studies', TRUE)
  ) {
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    // @TODO Minor. Clear only accession list.
    cache_clear_all('*', $cache_bin, TRUE);
  }
}
