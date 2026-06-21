<?php

/**
 * @file
 * TPPS REST API Class Settings Form.
 */

/**
 * TPPS REST API Class Settings Form.
 */
function tpps_rest_api_settings_form(array $form, array &$form_state) {
  $form['cache'] = [
    '#type' => 'fieldset',
    '#title' => t('Request/Response caching'),
    '#collapsible' => TRUE,
  ];
  $form['cache']['tpps_rest_api_use_cache'] = [
    '#type' => 'checkbox',
    '#title' => t('Use cache'),
    '#default_value' => variable_get('tpps_rest_api_use_cache'),
    '#description' => t("When checked then both request and response will be "
     . "saved into DB and reused next time."),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $form['time'] = [
    '#type' => 'fieldset',
    '#title' => t('Time'),
    '#collapsible' => TRUE,
  ];
  $form['time']['tpps_rest_api_max_execution_time'] = [
    '#type' => 'textfield',
    '#title' => t('Request Max Execution Time, milliseconds'),
    '#default_value' => variable_get('tpps_rest_api_max_execution_time'),
    '#description' => t("Longer requests will be logged using watchdog()."
      . "<br />Default: 5000."
      . "<br />Note: Any request which runs more then this limit "
      . "will be logged to watchdog even if debug mode is disabled."
    ),
    '#element_validate' => ['element_validate_integer_positive'],
  ];
  $form['time']['tpps_rest_api_timeout'] = [
    '#type' => 'textfield',
    '#title' => t('RestAPI request timeout'),
    '#default_value' => variable_get('tpps_rest_api_timeout'),
    '#description' => t("The maximum number of seconds to allow cURL "
      . "functions to execute. <br />To set in milliseconds use: "
      . "CURLOPT_TIMEOUT_MS.<br />See !link.",
      [
        '!link' => l(
          'https://curl.se/libcurl/c/CURLOPT_TIMEOUT.html',
          'https://curl.se/libcurl/c/CURLOPT_TIMEOUT.html'
        ),
      ]
    ),
    '#element_validate' => ['element_validate_integer_positive'],
  ];
  return system_settings_form($form);
}
