<?php

/**
 * @file
 * DOI Lookup settings.
 *
 * See classes:
 * - /src/Doi.class.inc.
 */

/**
 * DOI Lookup Settings Form.
 */
function tpps_doi_settings_form(array $form, array &$form_state) {
  $form['tpps_doi_lookup_method'] = [
    '#type' => 'select',
    '#title' => t('Lookup method'),
    '#options' => [
      'gemini' => t('Gemini'),
      'google_scholar' => t('Scrab Google Scholar'),
    ],
    '#default_value' => variable_get('tpps_doi_lookup_method'),
    '#description' => t('Using AI allows to avoid risks caused by scrapping.')
      . '<br />' . l(t('Gemini AI Settings page'), 'admin/config/tpps/gemini'),
  ];

  return system_settings_form($form);
}
