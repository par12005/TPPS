<?php

/**
 * @file
 * Manage TPPS Cache.
 */

define('TPPS_CLEAR_CACHE_BUTTON_NAME', 'tpps_clear_cache');

/**
 * TPPS Cache Settings Form.
 */
function tpps_cache_settings_form(array $form, array &$form_state) {

  // Button to clear cache.
  $form['note'] = [
    '#markup' => t('Note: Only "tpps" cache bin will be cleared.<br />'),
  ];
  $form[TPPS_CLEAR_CACHE_BUTTON_NAME] = [
    '#type' => 'submit',
    '#name' => TPPS_CLEAR_CACHE_BUTTON_NAME,
    '#value' => t('Clear TPPS Cache'),
  ];
  return $form;
}

/**
 * TPPS Cache Settings Form.
 */
function tpps_cache_settings_form_submit(array $form, array &$form_state) {
  // Process 'Clear TPPS Cache' button.
  $button_name = $form_state['triggering_element']['#name'];
  if ($button_name == TPPS_CLEAR_CACHE_BUTTON_NAME) {
    $bin = TPPS_CACHE_BIN ?? 'cache';
    cache_clear_all('*', $bin, TRUE);
    drupal_set_message(t('Cache bin "@bin" was cleared.', ['@bin' => $bin]));
  }
}
