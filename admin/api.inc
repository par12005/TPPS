<?php

/**
 * @file
 * Settings for API.
 */

/**
 * Menu callback. Shows settings form.
 */
function tpps_api_settings_form(array $form, array &$form_state) {
  $form['tpps_api_token'] = [
    '#type' => 'textfield',
    '#title' => t('API Token'),
    '#default_value' => variable_get('tpps_api_token'),
    '#description' => t('Generated token: @token', ['@token' => md5(time())]),

  ];

  return system_settings_form($form);
}
