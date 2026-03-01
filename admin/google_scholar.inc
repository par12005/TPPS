<?php

/**
 * @file
 * Google Scholar settings.
 *
 * See classes:
 * - /src/GoogleScholar.class.inc.
 */

/**
 * Google Scholar Settings Form.
 */
function tpps_google_scholar_settings_form(array $form, array &$form_state) {
  $form['tpps_google_scholar_method'] = [
    '#type' => 'select',
    '#title' => t('Use for requests:'),
    '#options' => [
      'gemini_ai' => t('Gemini AI'),
      'rest_api' => t('Rest API'),
    ],
    '#default_value' => variable_get('tpps_google_scholar_method'),
    '#description' => t('Using AI allows to avoid risks caused by scrapping.')
      . '<br />' . l(t('Gemini AI Settings page'), 'admin/config/tpps/gemini'),
  ];
  $form['rest_api'] = [
    '#type' => 'fieldset',
    '#title' => t('Rest API Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => (variable_get('tpps_google_scholar_method') == 'rest_api'),
  ];
  $form['rest_api']['tpps_google_scholar_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('Debug Mode'),
    '#default_value' => variable_get('tpps_google_scholar_debug_mode'),
    '#description' => t('When enabled no actual requests to Google Scholar '
      . 'will be performed.'),
  ];
  $form['rest_api']['tpps_google_scholar_delay'] = [
    '#type' => 'textfield',
    '#title' => t('Mininal delay between requests (in seconds)'),
    '#default_value' => variable_get(
      'tpps_google_scholar_delay', GoogleScholar::DEFAULT_DELAY
    ),
    '#description' => t('Introduce random delays (e.g., 2-10 seconds) '
      . 'between requests to avoid rate-limiting.'),
  ];

  return system_settings_form($form);
}
