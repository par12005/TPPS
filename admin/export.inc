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
    $state = tpps_study_export_get_state($accession);
    $form['tpps_export_state'] = [
      '#type' => 'textarea',
      '#title' => t('State PHP Array'),
      '#value' => var_export($state, 1),
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
    //'#description' => t(
    //  'Open special page on remote site or use DB dump of certain study.'
    //  . '<br />Steps:'
    //  . '<br /> 1. Open: DOMAIN/phppgadmin/'
    //  . '<br /> 2. Choose table: "public"."tpps_submission"'
    //  . '<br /> 3. Choose "SQL-query".'
    //  . '<br /> 4. Run: SELECT submission_state FROM "public"."tpps_submission" WHERE accession = "###";'
    //  . '<br /> 5. Click "Download".'
    //  . '<br /> 6. Choose "CSV".'
    //  . "<br /> 7. Open file and paste it's content."
    //),
  ];
  $form['action_export'] = [
    '#type' => 'submit',
    '#value' => t('Export'),
    '#submit' => ['tpps_study_export_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_study_export_form_reset'],
  ];
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_study_export_form_reset() {
  drupal_goto('admin/config/tpps/migration/export');
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
  $state = unserialize(html_entity_decode($result));
  if (empty($state['files'])) {
    drupal_set_message('Study has no files.');
  }
  else {
    foreach ($state['files'] as $fid) {
      $file = file_load($fid);
      $file->url = file_create_url($file->uri);
      $state['tpps_export_files'][$fid] = (array) $file;
    }
  }
  return $state;
}
