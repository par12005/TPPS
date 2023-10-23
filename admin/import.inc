<?php

/**
 * @file
 * TPPS Study Import Form and related functions.
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
    '#description' => t(
      'Default: files/<br />'
      . "Path to folder where study's files are stored."
    ),
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
    if (!empty($new_accession = $form_state['values']['tpps_import_new_accession'])) {
      $form_state['values']['frontpage']['accession'] = $new_accession;
      // Check also $form_state['input'] and $form_state['completed_form'].
    }

    foreach ($state['tpps_export_files'] as $fid => $file) {
      //dpm($file, $fid);
      // Download file.
      $new_file = tpps_study_import_download_file($file);
      //dpm($new_file, $new_file->fid);

      // Change fid.
      $replacement = ['old' => $fid, 'new' => $new_file->fid];
      $state = tpps_import_update_fid($state, $replacement);
    }
    // Recreate Study.
    drupal_form_submit('tpps_main', $state);
    // dpm(print_r($state, 1));
  }
  $form_state['rebuild'] = 1;
}

/**
 * Replaces  File Id with new in array recurcively.
 *
 * Will be updated both keys and values.
 *
 * @param array $array
 *   Array to be updated.
 * @param array $replacement
 *   Replacement array. Keys are:
 *   'old' - old value/key,
 *   'new' - new value/key.
 * @return array
 *   Returns updated array.
 */
function tpps_import_update_fid(array $array, array $replacement) {
  foreach ($array as $key => $value) {
    if (!empty($value) && is_array($value)) {
      $array[$key] = tpps_import_update_fid($value, $replacement);
    }
    elseif ((int) $value == (int) $replacement['old']) {
      $array[$key] = $replacement['new'];
    }
    if ((int) $key == (int) $replacement['old']) {
      $array[$replacement['new']] = $array[$key];
      unset($array[$key]);
    }
  }
  return $array;
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
