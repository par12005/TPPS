<?php

/**
 * @file
 * Zenodo settings.
 *
 * Read more:
 * - Docs: https://developers.zenodo.org.
 * - Sandbox: https://sandbox.zenodo.org/.
 * - Live: https://zenodo.org/.
 *
 * See classes:
 * - /src/zenodo/Zenodo.class.inc.
 * - /src/zenodo/ZenodoDeposit.class.inc.
 * - /src/zenodo/ZenodoFiles.class.inc.
 * - /src/zenodo/ZenodoRecords.class.inc.
 */

/**
 * TPPS Zenodo API Settings Form.
 */
function tpps_zenodo_settings_form(array $form, array &$form_state) {
  $doc_url = 'https://developers.zenodo.org/?shell#testing';
  $form['api'] = [
    '#type' => 'fieldset',
    '#title' => t('Zenodo API Settings'),
  ];

  $form['api']['tpps_zenodo_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('Use Zenodo Sandbox server '),
    '#default_value' => variable_get('tpps_zenodo_debug_mode'),
    '#description' => t('When enabled then special URL will be used for '
      . 'Zenodo API requests (sandbox.zenodo.org). <br />This sandbox is '
      . 'for testing and development purposes. <br/> Read more: !link.',
      ['!link' => l(t('"Testing" on developers.zenodo.org.'), $doc_url)]
    ),
  ];

  $form['api']['tokens'] = [
    '#type' => 'fieldset',
    '#title' => t('Access Tokens'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  ];
  // @TODO Minor. Make API tokens required.
  $form['api']['tokens']['tpps_zenodo_access_token_live'] = [
    '#type' => 'textfield',
    '#title' => t('Live'),
    '#default_value' => variable_get('tpps_zenodo_access_token_live'),
  ];
  $form['api']['tokens']['tpps_zenodo_access_token_sandbox'] = [
    '#type' => 'textfield',
    '#title' => t('Sandbox'),
    '#default_value' => variable_get('tpps_zenodo_access_token_sandbox'),
  ];
  $form['api']['tpps_zenodo_new_implementation'] = [
    '#type' => 'checkbox',
    '#title' => t('Use the new TPPS implementation of the API'),
    '#default_value' => variable_get('tpps_zenodo_new_implementation'),
    '#description' => t('When enabled then new implementation will be used. '
      . 'New implementation uses OOP and implements more Zenodo API actions '
      . 'then previous one. For example "newversion" action and more files actions.'
      . '<br />Disable if new implementation fails.'
    ),
  ];
  return system_settings_form($form);
}
