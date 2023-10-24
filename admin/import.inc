<?php

/**
 * @file
 * TPPS Study Import Form and related functions.
 *
 *
 * @TODO Use function to get rid of useless data from $form_state
 * function tpps_form_state_info(array &$new, array &$old);
 */

/**
 * TPPS Study Import Form.
 */
function tpps_study_import_form(array $form, array &$form_state) {
  $form['tpps_import_new_accession'] = [
    '#type' => 'textfield',
    '#title' => t('New Accession'),
    '#default_value' => $form_state['input']['tpps_import_new_accession'] ?? '',
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
  return $form;
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
  // Remove useless data from the submission state.
  if ($state_raw = $form_state['values']['tpps_import_submission_state']) {
    eval('$state = ' . $state_raw . ';');

    $state['values'] = $state['saved_values'];
    // We need to remove 'save_values' because some code checks if this
    // item exists and blocks somehow form submission.
    unset($state['saved_values']);

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

    // Process files.
    foreach ($state['tpps_export_files'] as $fid => $file) {
      //dpm($file, $fid);
      // Download file.
      $new_file = tpps_study_import_download_file($file);
      //dpm($new_file, $new_file->fid);

      // Change fid.
      $state = tpps_array_replace($state, $fid, $new_file->fid);
    }
    // Recreate Study.
    // drupal_form_submit('tpps_main', $state);
  }

  dpm(print_r($state, 1));
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
    'public://tpps_phenotype/' . REQUEST_TIME. '_' . $file['filename'],
    FILE_EXISTS_REPLACE
  );
  return $file;
}
