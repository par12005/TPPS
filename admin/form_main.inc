<?php

/**
 * @file
 * TPPS Page 1 Settings Form.
 */

/**
 * TPPS Page 1 Settings Form.
 */
function tpps_form_main_settings_form(array $form, array &$form_state) {
  // @TODO Build list of themes dynamically.
  $form['tpps_theme'] = [
    '#type' => 'select',
    '#title' => t('TPPS Theme'),
    '#default_value' => variable_get('tpps_theme', 'default'),
    '#options' => [
      'default' => t('Default Theme'),
      'rachel' => t('Rachel Theme'),
    ],
  ];
  $form['tpps_top_bar'] = [
    '#type' => 'select',
    '#title' => t('Top Bar Type'),
    '#default_value' => variable_get('tpps_top_bar', 'status'),
    '#options' => [
      'status' => t('Status Bar'),
      'navigation' => t('Navigation Bar'),
    ],
    '#description' => t('Status bar has icons which show completed steps.<br />'
      . 'Navigation bar has no icons but allows to go back to completed steps.'
    ),
  ];
  $form['#submit'][] = 'tpps_form_main_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Custom form submitter for 'tpps_page_1_settings_form' form.
 */
function tpps_form_main_settings_form_submit(array $form, array &$form_state) {
  // Clear cache on change only.
  if (
    ($form_state['values']['tpps_theme'] ?? NULL)
    != variable_get('tpps_theme', 'default')
  ) {
    cache_clear_all();
  }
}
