<?php

/**
 * @file
 * TPPS Study Import Form and related functions.
 *
 *
 * @TODO Minor. Get status of study in
 * https://tgwebdev.cam.uchc.edu/admin/reports/db/public.tripal_jobs.
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
      . "\ndpm(var_export(tpps_study_export_get_state(\$accession), 1));",
    '#rows' => 10,
    '#resizable' => TRUE,
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
  // @TODO Replace hardcoded '3'.
  //if (empty($state['saved_values']['file_info'][3])) {
  //  drupal_set_message('Study has no step 3 files.');
  //}
  //else {
  //  foreach (array_keys($state['saved_values']['file_info'][3]) as $fid) {
  //    $file = file_load($fid);
  //    $file->url = file_create_url($file->uri);
  //    $state['tpps_export_files'][$fid] = (array) $file;
  //  }
  //}
  return $state;
}
