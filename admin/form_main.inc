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
  $form['tpps_dark_theme'] = [
    '#type' => 'checkbox',
    '#title' => t('Use Dark Theme'),
    '#default_value' => variable_get('tpps_dark_theme', FALSE),
    '#description' => t('Dark theme only changes backgraund color of the TPPS '
      . 'forms. Enabled by default at dev-server.'),
  ];
  // @TODO Minor. Move to the common module settings page because it's not
  // only form related.
  $form['tpps_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('Debug Mode'),
    '#default_value' => variable_get('tpps_debug_mode', FALSE),
    '#description' => t('Default is unchecked. Must be disabled at live site. '
      . 'When checked then all error messages will be sent to browser and '
      . 'some extra debug messages could be shown.'),
  ];
  $form['tpps_autofocus_delay'] = [
    '#type' => 'textfield',
    '#title' => t('Autofocus Delay, msec'),
    '#default_value' => variable_get('tpps_autofocus_delay', 1000),
    '#description' => t('Default is 1000 (1 second). '
      . '<br />When page loaded scripts waits for a while and then hightlights '
      . 'specified field. When there is no delay it looks ugly.'),
  ];
  $form['#submit'][] = 'tpps_form_main_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Custom form submitter for 'tpps_page_1_settings_form' form.
 */
function tpps_form_main_settings_form_submit(array $form, array &$form_state) {
  // Clear cache on change only but not on each form submission.
  if (
    // CSS theme was changed.
    ($form_state['values']['tpps_theme'] ?? NULL)
    != variable_get('tpps_theme', 'default')
    // Enabled dark theme.
    || ($form_state['values']['tpps_dark_theme'] ?? NULL)
    != variable_get('tpps_dark_theme', FALSE)
  ) {
    cache_clear_all();
  }
}
