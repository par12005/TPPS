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
 * - /src/zenodo/ZenodoFile.class.inc.
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
    '#title' => t('Use Zenodo Sandbox Host'),
    '#default_value' => variable_get('tpps_zenodo_debug_mode'),
    '#description' => t('When enabled then special URL will be used for '
      . 'Zenodo API requests (sandbox.zenodo.org). <br />This sandbox is '
      . 'for testing and development purposes. <br/> Read more: !link.',
      ['!link' => l(t('"Testing" on developers.zenodo.org.'), $doc_url)]
    ),
  ];
  $form['api']['tpps_zenodo_host_greedy_search'] = [
    '#type' => 'checkbox',
    '#title' => t('Use "greedy" Zenodo Host search'),
    '#default_value' => variable_get('tpps_zenodo_host_greedy_search'),
    '#description' => t('When enabled then Deposit Id will be searched on both '
      . 'Live and Sandbox Hosts. <br />Useful for debugging and development.'
      . 'But it means at least one extra API-request for each operation which '
      . "could take 300-5000 milliseconds and if Deposition wan't found on "
      . "default Host number of extra requests could be bigger. "
      . "<br />If only one Zenodo Host is used or for production site it's "
      . 'better to disable this option.'
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

  $form['submission'] = [
    '#type' => 'fieldset',
    '#title' => t('Submission'),
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  ];
  $form['submission']['tpps_zenodo_overwrite_dataset_doi'] = [
    '#type' => 'checkbox',
    '#title' => t('Overwrite existing Dataset DOI'),
    '#default_value' => variable_get('tpps_zenodo_overwrite_dataset_doi'),
    '#description' => t('When enabled then existing value of the Dataset DOI '
      . 'in both (Shared State and State) will be overwritten. When disabled '
      . 'existing value will be left without change and warning message with '
      . 'new DOI and which fields was updated/skipped will be shown.'
      . '<br />Note: Zenodo Deposit Id will be stored anyway.'
    ),
  ];
  return system_settings_form($form);
}

/**
 * Validator for 'tpps_zenodo_settings_form' form.
 */
function tpps_zenodo_settings_form_validate(array $form, array &$form_state) {
  // Validate access tokens.
  foreach (['live', 'sandbox'] as $host) {
    $variable_name = 'tpps_zenodo_access_token_' . $host;
    $access_token = $form_state['input'][$variable_name] ?? NULL;
    if (empty($access_token)) {
      $message = t(
        "Note: Access token for Zenodo '@host_name' Host wasn't set.",
        ['@host_name' => ucfirst($host)]
      );
      drupal_set_message($message, 'warning');
      continue;
    }
    $old_access_token = variable_get($variable_name);
    // Update variable to be able to make API requests.
    // Validate token even stored in DB is the same just to be sure because
    // variable could be manually updated.
    variable_set($variable_name, $access_token);
    // Do request to check.
    ZenodoHost::set($host);
    try {
      $response = ZenodoDeposit::list([]);
      $message = t('Access token for Zenodo "<strong>@host_name</strong>" '
        . 'Host is <strong>valid</strong>.', ['@host_name' => ucfirst($host)]
      );
      drupal_set_message($message);
    }
    catch (Exception $e) {
      $message = t('Access token for Zenodo "<strong>@host_name</strong>" '
        . 'Host is <strong>invalid</strong>.', ['@host_name' => ucfirst($host)]
      );
      form_set_error('tpps_zenodo_access_token_' . $host, $message);
      variable_set($variable_name, $old_access_token);
    }
  }
}
