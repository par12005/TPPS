<?php

/**
 * @file
 * TPPS Admin Panel Settings Form.
 */

/**
 * TPPS Admin Panel Settings Form.
 */
function tpps_admin_panel_settings_form(array $form, array &$form_state) {
  $form['tpps_admin_panel_cache_new_species_list'] = [
    '#type' => 'checkbox',
    '#title' => t('Cache New Species List'),
    '#default_value' => variable_get(
      'tpps_admin_panel_cache_new_species_list', FALSE
    ),
  ];
  $form['tpps_admin_panel_show_execution_time'] = [
    '#type' => 'checkbox',
    '#title' => t('Show execution time'),
    '#default_value' => variable_get(
      'tpps_admin_panel_show_execution_time', FALSE
    ),
  ];

  $form['#submit'][] = 'tpps_admin_panel_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Custom form submitter for 'tpps_page_1_settings_form' form.
 */
function tpps_admin_panel_settings_form_submit(array $form, array &$form_state) {
  // Clear cache on change only but not on each form submission.
  $list = [
    'tpps_admin_panel_cache_new_species_list',
  ];
  $clear_cache = FALSE;
  foreach ($list as $key) {
    // Cache settings was changed and we need to reset cache.
    if (($form_state['values'][$key] ?? NULL) != variable_get($key, FALSE)) {
      $clear_cache = TRUE;
      break;
    }
  }
  if ($clear_cache) {
    $cid = 'tpps_admin_panel_show_all';
    tpps_cache_clear_all([$cid]);
    drupal_set_message(t('Cache was cleared for CID "@cid".', ['@cid' => $cid]));
  }
}
