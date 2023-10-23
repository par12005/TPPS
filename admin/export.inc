<?php

/**
 * @file
 * TPPS Study Import Form and related functions.
 */

/**
 * TPPS Study Import Form.
 */
function tpps_study_export_form(array $form, array &$form_state) {
  $accession = $form_state['input']['tpps_export_accession'] ?? '';
  $form['tpps_export_accession'] = [
    '#type' => 'textfield',
    '#title' => t('Accession'),
    '#default_value' => $accession,
    '#required' => TRUE,
    '#description' => t('Format is: TGDRxxx'),
  ];
  if (!empty($accession)) {
    $form['tpps_export_state'] = [
      '#type' => 'textarea',
      '#title' => t('State PHP Array'),
      '#value' => var_export(tpps_study_export_get_state($accession), 1),
      '#rows' => 10,
      '#resizable' => TRUE,
    ];
  }
  // @TODO Minor. Just print function's code:
  // https://stackoverflow.com/questions/7026690
  $form['tpps_export_code'] = [
    '#type' => 'textarea',
    '#title' => t('PHP Code to get data on remote server'),
    '#value' => "\$accession = '$accession';"
      . "\n\$sql = 'SELECT submission_state FROM tpps_submission "
      . "WHERE accession = '$accession' LIMIT 1;';"
      . "\n\$result = db_query(\$sql)->fetchField();"
      . "\ndpm(unserialize(html_entity_decode(\$result)));",
    '#rows' => 10,
    '#resizable' => TRUE,
  ];
  $form['#submit'][] = 'tpps_study_export_form_submit';
  // @TODO Add simple buttons.
  return system_settings_form($form);
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_study_export_form_submit(array $form, array &$form_state) {
  $form_state['rebuild'] = 1;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_study_export_get_state($accession) {
  $result = db_select('tpps_submission', 'ts')
    ->fields('ts', ['submission_state'])
    ->condition('accession', $accession)
    ->range(0, 1)
    ->execute()
    ->fetchField();
  return unserialize(html_entity_decode($result));
}
