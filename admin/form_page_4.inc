<?php

/**
 * @file
 * TPPS Page 4 Settings Form.
 */

/**
 * Generates settings form.
 */
function tpps_form_page_4_settings_form(array $form, array &$form_state) {
  $form['tpps_page_4_cache_synonym_list'] = [
    '#type' => 'checkbox',
    '#title' => t('Cache list of Synonyms'),
    '#default_value' => variable_get('tpps_page_4_cache_synonym_list', FALSE),
    '#description' => t(
      'Default is <strong>unchecked</strong>.<br />'
      . "When checked list of synonyms will be cached using custom bin 'tpps'."
    ),
  ];
  $form['tpps_page_4_cache_unit_list'] = [
    '#type' => 'checkbox',
    '#title' => t('Cache lists of Units'),
    '#default_value' => variable_get('tpps_page_4_cache_unit_list', FALSE),
    '#description' => t(
      'Default is <strong>unchecked</strong>.<br />'
      . "When checked list of units per synonym will be cached using "
      . "custom bin 'tpps'."
    ),
  ];
  $form['tpps_page_4_update_ploidy_description'] = [
    '#type' => 'checkbox',
    '#title' => t('Update Ploidy Description'),
    '#default_value' => variable_get('tpps_page_4_update_ploidy_description', TRUE),
    '#description' => t(
      'Default is <strong>checked</strong>.'
      "<br />When checked description of the 'Ploidy' field will be "
      . " updated according to current selection."
    ),
  ];
  $form['#submit'][] = 'tpps_form_page_4_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Form submitter.
 */
function tpps_form_page_4_settings_form_submit(array $form, array &$form_state) {
  // Clear cache on change only.
  if (
    ($form_state['values']['tpps_page_4_cache_synonym_list'] ?? NULL)
      != variable_get('tpps_page_4_cache_synonym_list', FALSE)
    || ($form_state['values']['tpps_page_4_cache_unit_list'] ?? NULL)
      != variable_get('tpps_page_4_cache_unit_list', FALSE)
  ) {
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    // @TODO Minor. Clear only unit/synonym lists.
    cache_clear_all('*', $cache_bin, TRUE);
  }
}
