<?php

/**
 * @file
 * TPPS Page 1 Settings Form.
 */

/**
 * Generates settings form.
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
  $form['tpps_page_1_doi_lookup_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('DOI Lookup Debug Mode'),
    '#default_value' => variable_get('tpps_page_1_doi_lookup_debug_mode'),
    '#description' => t(
      'Default is unchecked.<br />'
      . 'When checked raw DOI Publication data will be show at page in JSON format.'
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
