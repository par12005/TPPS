<?php

/**
 * @file
 * TPPS Page 1 Settings Form.
 */

/**
 * TPPS Page 1 Settings Form.
 */
function tpps_form_page_1_settings_form(array $form, array &$form_state) {
  $form['tpps_page_1_doi_check_source'] = [
    '#type' => 'radios',
    '#title' => t('DB table to check if DOI is in use by other submission'),
    '#options' => [
      'public.tpps_submissions' => 'public.tpps_submissions',
      'chado.dbxref' => 'chado.dbxref',
    ],
    '#default_value' => variable_get(
      'tpps_page_1_doi_check_source',
      'public.tpps_submissions'
    ),
    '#description' => t(
      "Default is 'public.tpps_submissions'.<br />"
      . "Using 'chado.dbxref' seems to be faster but queries against "
      . "'public.tpps_submissions' table are cached and provide accession "
      . "number (TGDRxxx) which will be shown to user in warning message."
    ),
  ];
  $form['tpps_page_1_cache_ajax_responses'] = [
    '#type' => 'checkbox',
    '#title' => t('Cache AJAX responses'),
    '#default_value' => variable_get('tpps_page_1_cache_ajax_responses', TRUE),
    '#description' => t(
      'Default is checked.<br />'
      . "When checked request's data and related response will be cached using "
      . " custom 'TPPS' cache bin which could be cleared separately."
    ),
  ];
  $form['tpps_page_1_dryad_api_url'] = [
    '#type' => 'textfield',
    '#title' => t('Dryad API Endpoint URL'),
    '#default_value' => variable_get(
      'tpps_page_1_dryad_api_url',
      'http://datadryad.org/api/v2/datasets/'
    ),
    '#description' => t(
      'Default: http://datadryad.org/api/v2/datasets/<br />'
      . 'URL to get DOI publication information.'
    ),
  ];
  $form['tpps_page_1_ajax_debug'] = [
    '#type' => 'checkbox',
    '#title' => t('AJAX Debug Mode'),
    '#default_value' => variable_get('tpps_page_1_ajax_debug', FALSE),
    '#description' => t(
      'Default is unchecked.<br />'
      . 'When checked debug data will be added to AJAX response.'
    ),
  ];
  return system_settings_form($form);
}
