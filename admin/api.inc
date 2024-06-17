<?php

/**
 * @file
 * Settings for API.
 */

/**
 * Menu callback. Shows settings form.
 */
function tpps_api_settings_form(array $form, array &$form_state) {
  $token = trim(variable_get('tpps_api_token'));
  $description = [
    t('The same token will be used for all new API Endpoints.'),
    t('To block access to all API Endpoints make it empty.'),
    t('To get access send token in GET-parameter "token".'),
  ];
  if (empty($token)) {
    $description[] = t('Example of the token: <strong>@token</strong>',
      ['@token' => md5(time())]
    );
  }
  $form['tpps_api_token'] = [
    '#type' => 'textfield',
    '#title' => t('API Token'),
    '#default_value' => $token,
    '#description' => theme('item_list', ['items' => $description]),
  ];

  return system_settings_form($form);
}
