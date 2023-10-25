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
  $form['#submit'][] = 'tpps_form_front_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Form submitter.
 */
function tpps_form_front_settings_form_submit(array $form, array &$form_state) {
  if (!empty($form_state['values']['tpps_front_cache_accession_list'])) {
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    // @TODO Minor. Clear only accession list.
    cache_clear_all('*', $cache_bin, TRUE);
  }
}
