<?php

/**
 * @file
 * TPPS REST API Class Settings Form.
 */

/**
 * TPPS REST API Class Settings Form.
 */
function tpps_rest_api_settings_form(array $form, array &$form_state) {
  // Re-use stored responses.
  //
  //
  // @TODO Checkbox to save:
  // Request/Response
  //   Each HTTP mehtod
  //   Each tag.
  $form['cache'] = [
    '#type' => 'fieldset',
    '#title' => t('Response caching'),
    '#collapsible' => TRUE,
  ];
  $form['cache']['tpps_rest_api_save_requests'] = [
    '#type' => 'checkbox',
    '#title' => t('Save Requests'),
    '#default_value' => variable_get('tpps_rest_api_save_requests'),
    '#description' => t("Default is checked. <br />When checked requests "
      . "will be stored in DB. "
      . "<br />When unchecked requests won't be saved in database."
      . "<br />WARNING: <strong>To store also responses this option must "
      . "be enabled</strong>."
      . "<br />Note: Each certain request could overwrite this setting. "
      . "See 'save_request' property of the RestAPI class."),
  ];
  $form['cache']['tpps_rest_api_save_responses'] = [
    '#type' => 'checkbox',
    '#title' => t('Save Responses'),
    '#default_value' => variable_get('tpps_rest_api_save_responses'),
    '#description' => t("Default is checked. <br />When checked any (valid "
      . "and invalid) responses will be stored in DB and could be re-used. "
      . "<br />When unchecked responses won't be saved in database."
      . "<br />Note: Each certain request could overwrite this setting. "
      . "See 'save_response' property of the RestAPI class."),
    '#states' => [
      'visible' => [
        ':input[name="tpps_rest_api_save_requests"]' => ['checked' => TRUE],
      ],
      'unchecked' => [
        ':input[name="tpps_rest_api_save_requests"]' => ['checked' => FALSE],
      ],
    ],
  ];
  $form['cache']['tpps_rest_api_use_cached'] = [
    '#type' => 'checkbox',
    '#title' => t('Use Cached Requests and Responses'),
    '#default_value' => variable_get('tpps_rest_api_use_cached'),
    '#description' => t("Only latest valid response could be re-used."
      . "<br />Note: Each request could overwrite this setting. "
      . "See 'useCached' property of the 'RestAPI' class."),
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
