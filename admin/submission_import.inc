<?php

/**
 * @file
 * TPPS Submission Import Form and related functions.
 *
 * @TODO Minor. Use function to get rid of useless data from $form_state
 * function tpps_form_state_info(array &$new, array &$old);
 */

/**
 * TPPS Submission Import Form.
 */
function tpps_submission_import_form(array $form, array &$form_state) {
  $form['tpps_submission_import_new_accession'] = [
    '#type' => 'textfield',
    '#field_prefix' => 'TGDR',
    '#title' => t('New Accession'),
    '#default_value' => $form_state['input']['tpps_submission_import_new_accession']
    ?? tpps_init_project_get_next_accession(),
    '#required' => FALSE,
    '#description' => 'Leave empty to autogenerate accession',
  ];
  $form['tpps_submission_import_overwrite_submission'] = [
    '#type' => 'checkbox',
    '#title' => 'Overwrite existing submission',
    '#default_value' => $form_state['input']['tpps_submission_import_overwrite_submission'] ?? FALSE,
  ];
  $form['tpps_submission_import_file_replace'] = [
    '#type' => 'checkbox',
    '#title' => 'Replace existing files',
    '#default_value' => variable_get('tpps_submission_import_file_replace', TRUE),
    '#description' => t('Behavior when the destination file already exists:'
      . '<br /><strong>Checked</strong>: replace the existing file. '
      . 'If a managed file with the destination name exists then its '
      . 'database entry will be updated. '
      . 'If no database entry is found then a new one will be created.'
      . '<br /><strong>Unchecked</strong>: Append _{incrementing number} '
      . 'until the filename is unique.'),
  ];
  // @TODO Idea. Use file upload field instead of plain code dump.
  $form['tpps_submission_import_submission_state'] = [
    '#type' => 'textarea',
    '#title' => t('Submission State'),
    '#default_value' => $form_state['input']['tpps_submission_import_submission_state'] ?? '',
  ];
  $form['action_import'] = [
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => ['tpps_submission_import_form_submit'],
  ];
  $form['action_review_dump'] = [
    '#type' => 'submit',
    '#value' => t('Review Dump'),
    '#submit' => ['tpps_submission_import_review_dump'],
  ];
  $form['action_dry_run'] = [
    // Submission won't be submitted but files will be downloaded
    // and added to site.
    '#type' => 'submit',
    '#value' => t('Dry Run'),
    '#submit' => ['tpps_submission_import_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_submission_import_form_reset'],
  ];
  $form['#validate'][] = 'tpps_submission_import_form_validate';
  return $form;
}

/**
 * Validation for form 'tpps_submission_import_form'.
 */
function tpps_submission_import_form_validate(array &$form, array &$form_state) {
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Skip validation if 'Reset' was clicked.
  if ($form_state['triggering_element']['#value'] == t('Reset')) {
    return;
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate accession.
  $new_accession = $form_state['values']['tpps_submission_import_new_accession'];
  // Since we allow both formats (TGDRxxx and xxx) we need to sync them.
  // @TODO Update code to use only 'xxx' format.
  $new_accession = 'TGDR' . str_replace('TGDR', '', $new_accession);
  if (empty($form_state['values']['tpps_submission_import_overwrite_submission'])) {
    if (tpps_api_check_accession($new_accession)) {
      form_set_error('tpps_submission_import_new_accession', t('Accession already in use. '
        . theme('item_list', [
          'items' => [
            'Check "<strong>Overwrite existing submission</strong>" to reuse accession.',
            'Click "<strong>Reset</strong>" button to get next available accesson.',
          ],
        ])
        )
      );
    }
  }
  else {
    // @TODO Minor. Validate accession. Check if 'TGDR' is present and so on.
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate Submission State.
  if (empty($form_state['values']['tpps_submission_import_submission_state'])) {
    form_set_error('tpps_submission_import_submission_state', t('Submission state is empty.'));
  }
}

/**
 * Resets TPPS Submission Import Form.
 */
function tpps_submission_import_form_reset(array $form, array &$form_state) {
  drupal_goto('admin/config/tpps/submission/import');
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/**
 * Form submitter. Shows submision dump with line numbers.
 */
function tpps_submission_import_review_dump(array $form, array &$form_state) {
  if ($state = trim($form_state['values']['tpps_submission_import_submission_state'])) {
    // Note: $state is a string.
    // Show with line numbers.
    $state_array = preg_split("/\r\n|\n|\r/", $state);
    drupal_set_message('<pre>' . print_r($state_array, 1) . '</pre>');
  }
  $form_state['rebuild'] = 1;
}

/**
 * Form submitter. Imports submission from the dump (with files).
 */
function tpps_submission_import_form_submit(array $form, array &$form_state) {
  global $user;
  $dry_run = (bool) ($form_state['triggering_element']['#value'] == t('Dry Run'));
  module_load_include('inc', 'tpps', 'includes/common');
  variable_set('tpps_submission_import_file_replace',
    ($form_state['values']['tpps_submission_import_file_replace'] ?? FALSE)
  );
  // Remove useless data from the submission state.
  if ($state_raw = trim($form_state['values']['tpps_submission_import_submission_state'])) {
    // Fix 'stdClass' in the submistion dump createdy by var_export().
    // We shouldn't do this on export to have unchanged data.
    // Though var_export() is designed to "outputs or return a parsable
    // string representation of a variable", the output of stdClass
    // objects is not "a parsable string".
    tpps_submission_import_fix_stdclass($state_raw);

    try {
      eval('$state = ' . $state_raw . ';');
    }
    catch (Exception $e) {
      drupal_set_message($e->errorMessage());
      drupal_set_message('<pre>' . print_r($state, 1) . '</pre>');
      return;
    }

    // Note: $state['saved_values'] is required for normal form submission.
    // Fix Warning: end() expects parameter 1 to be array, null given in
    // file_managed_file_validate()
    // (line 607 of /var/www/Drupal/modules/file/file.module).
    $state['triggering_element']['#parents'] = ['submit'];
    // Fix Notice: Undefined index: #value in tpps_main_validate()
    // (line 1071 of /var/www/Drupal/sites/all/modules/TGDR/tpps.module).
    $state['triggering_element']['#value'] = t('Review and Submit');

    $new_accession = $form_state['values']['tpps_submission_import_new_accession']
      ?? tpps_init_project_get_next_accession();
    // Since we allow both formats (TGDRxxx and xxx) we need to sync them.
    // @TODO Update code to use only 'xxx' format.
    $new_accession = 'TGDR' . str_replace('TGDR', '', $new_accession);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Process files (if any).
    if (empty($state['tpps_submission_export_files'])) {
      drupal_set_message('Submission has no files.', 'warning');
    }
    else {
      foreach ($state['tpps_submission_export_files'] as $fid => $file) {
        if (empty($file)) {
          continue;
        }
        $new_file = tpps_submission_import_download_file($file);
        if (empty($new_file)) {
          drupal_set_message(t('File @url is empty.',
            ['@url' => l($file['url'], $file['url'])]), 'error'
          );
          $form['rebuild'] = 1;
          return;
        }
        // To avoid errors when page navigation used we need to have
        // temporary stored files until final form submit ('summarypage').
        // @TODO Minor. Think about reusage of function
        // tpps_submission_file_set_status().
        if ($new_file->status != 0) {
          $new_file->status = 0;
          file_save($new_file);
        }
        tpps_file_add_usage($new_file, $new_accession);
        // Change File ID in submission state.
        $state = tpps_array_replace($state, $fid, $new_file->fid);
        drupal_set_message(t('Original file "<strong>@old_fid</strong>" '
          . 'was replaced with new "<strong>@new_fid</strong>".',
          ['@old_fid' => $fid, '@new_fid' => $new_file->fid]
        ));
        if ($dry_run) {
          drupal_set_message($fid . ': <pre>' . print_r($file, 1) . '</pre>');
          drupal_set_message('New file: <pre>' . print_r($new_file, 1) . '</pre>');
        }
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Accession.
    if (
      $dry_run
      || !empty($form_state['values']['tpps_submission_import_overwrite_submission'])
    ) {
      // Remove submission and files.
      // We can't use tpps_update_submission() because it doesn't manage files
      // but only overwrites record in chado.tpps_submission table and
      // redord in chado.dbxref will be left unchanged.
      tpps_submission_purge($new_accession, $force = TRUE);
    }
    // $form_state['values']['frontpage']['accession'] = $new_accession;
    // Check also $form_state['input'] and $form_state['completed_form'].
    // Update accession value in $form_state.
    $old_accession = $state['saved_values']['frontpage']['accession'];
    $state = tpps_array_replace($state, $old_accession, $new_accession);
    // Store original accession.
    $state['tpps_submission_import']['original_accession'] = $old_accession;
    $state['tpps_submission_import']['imported'] = REQUEST_TIME;
    $state['tpps_submission_import']['imported_date']
      = format_date(REQUEST_TIME, 'custom', 'Y-m-d h:s:i T');

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // From tpps_init_project().
    $state['saved_values']['dbxref_id']
      = $state['dbxref_id']
        = $state['values']['dbxref_id']
          = tpps_init_project_get_dbxref_id($new_accession);

    $state['values']['status'] = 'Incomplete';
    $state['values'] = $state['saved_values'];
    if ($dry_run) {
      // Show with line numbers.
      $state_array = preg_split("/\r\n|\n|\r/", var_export($state, 1));
      drupal_set_message('<pre>' . print_r($state_array, 1) . '</pre>');
    }
    else {
      tpps_create_submission($state, $user->uid);
      // Clear cached list of studies to have new items at 'frontpage'.
      // @TODO Minor. Clear only accession list.
      cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
      // Recreate Submission.
      drupal_form_submit(($state['build_info']['form_id'] ?? 'tpps_main'), $state);
      // This code will not be executed because drupal_form_submit() will
      // redirect in case of success.
      if (form_get_errors()) {
        // Show study info only when there was some validation errors
        // because if study was submitted successfully user will be
        // redirected to the study page with the same table of fields.
        drupal_set_message(tpps_table_display($state));
      }
    }
  }

  if (!$dry_run) {
    $url_list = [
      'tppsc', "tpps-admin-panel/$new_accession", "tpps/details/$new_accession",
    ];
    foreach ($url_list as $url) {
      $items[] = l($url, $url);
    }
    drupal_set_message(
      t('Useful links:') . '<br />' . theme('item_list', ['items' => $items])
    );
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
function tpps_submission_import_download_file(array $file) {
  if (empty($content = file_get_contents($file['url']))) {
    return NULL;
  }
  // Move file to Drupal.
  $path = tpps_submission_import_get_folder($file['uri']) . '/' . $file['filename'];
  $action = (
    variable_get('tpps_submission_import_file_replace', TRUE) ?
    // Replace = 1, Rename = 0.
    FILE_EXISTS_REPLACE : FILE_EXISTS_RENAME
  );
  $file = file_save_data($content, $path, $action);
  return $file;
}

/**
 * Fixes issue with 'stdClass' on import of dump created with var_export().
 *
 * It's PHP's bug (not Drupal). Read more:
 * https://www.drupal.org/project/devel/issues/215375
 * https://bugs.php.net/bug.php?id=67918
 *
 * Solution:
 * https://github.com/laravel/framework/pull/17976/commits/b6f1944bc24d6deb1cb30898b3b3eb660019bc99
 *
 * Other solution from 'micro' Drupal's module:
 * https://www.drupal.org/files/issues/macro.module_0.patch
 * https://www.drupal.org/project/devel/issues/215375 - issue with other options.
 *
 * @param string $state_raw
 *   Data to be parsed.
 */
function tpps_submission_import_fix_stdclass(&$state_raw) {
  $state_raw = str_replace('stdClass::__set_state', '(object)', $state_raw);
}

/**
 * Gets folder from File URI.
 *
 * @param string $file_uri
 *   File URI.
 *   Example: 'public://tpps_genotype/TGDR1139_Rhizophora_SSR_1.csv'.
 *
 * @return string
 *   Returns folder without trailing slash used in File URI.
 *   Example: 'public://tpps_genotype'.
 *   Returns default folder when URI is empty.
 */
function tpps_submission_import_get_folder($file_uri = NULL) {
  return $file_uri ? dirname($file_uri) : 'public://';
}
