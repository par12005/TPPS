<?php

/**
 * @file
 * TPPS Study Import Form and related functions.
 *
 *
 * @TODO Minor. Use function to get rid of useless data from $form_state
 * function tpps_form_state_info(array &$new, array &$old);
 */

/**
 * TPPS Study Import Form.
 */
function tpps_study_import_form(array $form, array &$form_state) {
  $form['tpps_import_new_accession'] = [
    '#type' => 'textfield',
    '#title' => t('New Accession'),
    '#default_value' => $form_state['input']['tpps_import_new_accession']
    ?? 'TGDR' . tpps_init_project_get_next_accession(),
    '#required' => FALSE,
  ];
  // @TODO Idea. Use file upload field instead of plain code dump.
  $form['tpps_import_submission_state'] = [
    '#type' => 'textarea',
    '#title' => t('Submission State'),
    '#default_value' => $form_state['input']['tpps_import_submission_state'] ?? '',
    '#required' => TRUE,
  ];
  $form['action_import'] = [
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => ['tpps_study_import_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_study_import_form_reset'],
  ];
  $form['#validate'][] = 'tpps_study_import_form_validate';
  return $form;
}

/**
 * Validation for form 'tpps_study_import_form'.
 */
function tpps_study_import_form_validate(array $form, array &$form_state) {
  $new_accession = $form_state['values']['tpps_import_new_accession'];
  if (tpps_api_check_accession($new_accession)) {
    form_set_error('tpps_import_new_accession', t('Accession already in use. '
      . 'Click "Reset" to get next available value.')
    );
  }
}

/**
 * Resets TPPS Import Form.
 */
function tpps_study_import_form_reset(array $form, array &$form_state) {
  drupal_goto('admin/config/tpps/migration/import');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_study_import_form_submit(array $form, array &$form_state) {
  global $user;
  module_load_include('inc', 'tpps', 'includes/common');
  // Remove useless data from the submission state.
  if ($state_raw = $form_state['values']['tpps_import_submission_state']) {
    eval('$state = ' . $state_raw . ';');

    $state['values'] = $state['saved_values'];
    // Note: $state['saved_values'] is required for normal form submission.
    // Fix Warning: end() expects parameter 1 to be array, null given in
    // file_managed_file_validate()
    // (line 607 of /var/www/Drupal/modules/file/file.module).
    $state['triggering_element']['#parents'] = ['submit'];
    // Fix Notice: Undefined index: #value in tpps_main_validate()
    // (line 1071 of /var/www/Drupal/sites/all/modules/TGDR/tpps.module).
    $state['triggering_element']['#value'] = t('Review and Submit');

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Change accession.
    // @TODO Major. Check if new accession is not in use.
    if (!empty($new_accession = $form_state['values']['tpps_import_new_accession'])) {
      // $form_state['values']['frontpage']['accession'] = $new_accession;
      // Check also $form_state['input'] and $form_state['completed_form'].
      // Update accession value in $form_state.
      $state = tpps_array_replace(
        $state,
        $state['values']['frontpage']['accession'],
        $new_accession
      );
    }
    // From tpps_init_project().
    $state['values']['dbxref_id'] = tpps_init_project_get_dbxref_id($new_accession);
    $state['values']['status'] = 'Incomplete';
    tpps_create_submission($state, $user->uid);

    // Process files.
    foreach ($state['tpps_export_files'] as $fid => $file) {
      $new_file = tpps_study_import_download_file($file);
      // Change fid.
      $state = tpps_array_replace($state, $fid, $new_file->fid);
    }
    // Recreate Study.
    drupal_form_submit('tpps_main', $state);
  }
  $form_state['rebuild'] = 1;
}

/**
 * Downloads remote file and add to Drupal.
 *
 * @param array $file
 *   File's metadate.
 *   Required items: 'filename' and 'url'.
 *
 * @return object
 *   Returns Managed File Object.
 */
function tpps_study_import_download_file(array $file) {
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $file['url']);
  curl_setopt($ch, CURLOPT_VERBOSE, 1);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_AUTOREFERER, FALSE);
  curl_setopt($ch, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_setopt($ch, CURLOPT_HEADER, 0);
  $content = curl_exec($ch);
  curl_close($ch);
  // Move file to Drupal.
  $file = file_save_data(
    $content,
    'public://tpps_phenotype/' . $file['filename'],
    FILE_EXISTS_REPLACE
  );
  return $file;
}
