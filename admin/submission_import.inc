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
  $form['tpps_import_new_accession'] = [
    '#type' => 'textfield',
    '#title' => t('New Accession'),
    '#default_value' => $form_state['input']['tpps_import_new_accession']
    ?? 'TGDR' . tpps_init_project_get_next_accession(),
    '#required' => FALSE,
    '#description' => 'Leave empty to autogenerate accession',
  ];
  $form['tpps_import_overwrite_submission'] = [
    '#type' => 'checkbox',
    '#title' => 'Overwrite existing submission',
    '#default_value' => $form_state['input']['tpps_import_overwrite_submission'] ?? FALSE,
  ];
  $form['tpps_import_files_replace'] = [
    '#type' => 'checkbox',
    '#title' => 'Replace existing files',
    '#default_value' => variable_get('tpps_import_file_replace', TRUE),
    '#description' => t('Behavior when the destination file already exists:'
      . '<br /><strong>Checked</strong>: replace the existing file. '
      . 'If a managed file with the destination name exists then its '
      . 'database entry will be updated. '
      . 'If no database entry is found then a new one will be created.'
      . '<br /><strong>Unchecked</strong>: Append _{incrementing number} '
      . 'until the filename is unique.'),
  ];
  $form['tpps_import_debug_state'] = [
    '#type' => 'checkboxes',
    '#title' => 'Dump submission state parts:',
    '#options' => [
      'values' => 'values',
      'saved_values' => 'saved_values',
      'complete form' => 'complete form',
      'files' => 'files',
    ],
    '#default_value' => $form_state['input']['tpps_import_debug_state'] ?? [],
  ];
  // @TODO Idea. Use file upload field instead of plain code dump.
  $form['tpps_import_submission_state'] = [
    '#type' => 'textarea',
    '#title' => t('Submission State'),
    '#default_value' => $form_state['input']['tpps_import_submission_state'] ?? '',
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
  $new_accession = $form_state['values']['tpps_import_new_accession'];
  if (empty($form_state['values']['tpps_import_overwrite_submission'])) {
    if (tpps_api_check_accession($new_accession)) {
      form_set_error('tpps_import_new_accession', t('Accession already in use. '
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
  if (empty($form_state['values']['tpps_import_submission_state'])) {
    form_set_error('tpps_import_submission_state', t('Submission state is empty.'));
  }
}

/**
 * Resets TPPS Submission Import Form.
 */
function tpps_submission_import_form_reset(array $form, array &$form_state) {
  drupal_goto('admin/config/tpps/migration/import');
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/**
 * Form submitter. Shows submision dump with line numbers.
 */
function tpps_submission_import_review_dump(array $form, array &$form_state) {
  if ($state = trim($form_state['values']['tpps_import_submission_state'])) {
    // Note: $state is a string.
    // Show with line numbers.
    $state_array = preg_split("/\r\n|\n|\r/", $state);
    dpm(print_r($state_array, 1));
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
  variable_set('tpps_import_file_replace',
    ($form_state['values']['tpps_import_file_replace'] ?? FALSE)
  );
  // Remove useless data from the submission state.
  if ($state_raw = trim($form_state['values']['tpps_import_submission_state'])) {
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
      dpm(print_r($state, 1));
      return;
    }

    // Debug Report.
    $part_list = ($form_state['input']['tpps_import_debug_state'] ?? []);
    foreach ($part_list as $key => $value) {
      if (empty($value)) {
        continue;
      }
      elseif (!empty($state[$key])) {
        dpm(print_r($state[$key], 1), $value);
      }
      else {
        dpm('Submission State part "' . $value . '" is empty.');
        continue;
      }
    }

    // Note: $state['saved_values'] is required for normal form submission.
    // Fix Warning: end() expects parameter 1 to be array, null given in
    // file_managed_file_validate()
    // (line 607 of /var/www/Drupal/modules/file/file.module).
    $state['triggering_element']['#parents'] = ['submit'];
    // Fix Notice: Undefined index: #value in tpps_main_validate()
    // (line 1071 of /var/www/Drupal/sites/all/modules/TGDR/tpps.module).
    $state['triggering_element']['#value'] = t('Review and Submit');

    $new_accession = $form_state['values']['tpps_import_new_accession']
      ?? 'TGDR' . tpps_init_project_get_next_accession();

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Process files (if any).
    if (empty($state['tpps_export_files'])) {
      drupal_set_message('Submission has no files.');
    }
    else {
      foreach ($state['tpps_export_files'] as $fid => $file) {
        if (empty($file)) {
          continue;
        }
        $new_file = tpps_submission_import_download_file($file);
        // To avoid errors when page navigation used we need to have
        // temporary stored files until final form submit ('summarypage').
        // @TODO Minor. Think about reusage of function
        // tpps_submission_files_set_status().
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
          dpm($file, $fid);
          dpm(print_r($new_file, 1), 'new file');
        }
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Accession.
    if (!empty($form_state['values']['tpps_import_overwrite_submission'])) {
      // Remove submission and files.
      // We can't use tpps_update_submission() because it doesn't manage files
      // but only overwrites record in chado.tpps_submission table and
      // redord in chado.dbxref will be left unchanged.
      tpps_submission_purge($new_accession, $force = TRUE);
    }
    // $form_state['values']['frontpage']['accession'] = $new_accession;
    // Check also $form_state['input'] and $form_state['completed_form'].
    // Update accession value in $form_state.
    $state = tpps_array_replace(
      $state,
      $state['saved_values']['frontpage']['accession'],
      $new_accession
    );

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
      $state_array = preg_split("/\r\n|\n|\r/", $state);
      dpm(print_r($state_array, 1));
    }
    else {
      tpps_create_submission($state, $user->uid);
      // Recreate Submission.
      drupal_form_submit('tpps_main', $state);
      if (form_get_errors()) {
        // Show study info only when there was some validation errors
        // because if study was submitted successfully user will be
        // redirected to the study page with the same table of fields.
        drupal_set_message(tpps_table_display($state));
      }
      // Clear cached list of studies to have new items at 'frontpage'.
      $cache_bin = TPPS_CACHE_BIN ?? 'cache';
      // @TODO Minor. Clear only accession list.
      cache_clear_all('*', $cache_bin, TRUE);
    }
  }

  // Show useful links.
  drupal_set_message(theme('item_list', [
    'items' => [
      l('tppsc', 'tppsc'),
      l('tpps-admin-panel/' . $new_accession, 'tpps-admin-panel/' . $new_accession),
      l('tpps/details/' . $new_accession, 'tpps/details/' . $new_accession),
    ]
  ]));

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
    tpps_submission_import_get_folder($file['uri']) . '/' . $file['filename'],
    (
      variable_get('tpps_import_file_replace', TRUE) ?
      FILE_EXISTS_REPLACE : FILE_EXISTS_RENAME
    )
  );
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
