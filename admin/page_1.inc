<?php

/**
 * @file
 * TPPS Page 1 Settings Form.
 */

/**
 * TPPS Page 1 Settings Form.
 */
function tpps_page_1_settings_form(array $form, array &$form_state) {
  $form['doi_check_source'] = [
    '#type' => 'radios',
    '#title' => t('DB table to check if DOI is in use by other submission'),
    '#options' => [
      'public.tpps_submissions' => 'public.tpps_submissions',
      'chado.dbxref' => 'chado.dbxref',
    ],
    '#default_value' => 'public.tpps_submissions',
  ];
  return system_settings_form($form);
}
