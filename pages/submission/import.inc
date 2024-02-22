<?php

/**
 * @file
 * TPPS Submission Import Form and related functions.
 *
 * @TODO Minor. There is no ability to specify accession. Only next one could
 * be used or existing one overritten.
 * @TODO Minor. Use function to get rid of useless data from $form_state
 * function tpps_form_state_info(array &$new, array &$old);
 */

module_load_include('inc', 'tpps', 'includes/common');
module_load_include('inc', 'tpps', 'pages/submission/list');
module_load_include('inc', 'tpps', 'includes/form');
module_load_include('inc', 'tpps', 'includes/imported_studies_list');

/**
 * TPPS Submission Import Form.
 */
function tpps_submission_import_form(array $form, array &$form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);
  $next_accession = 'TGDR' . tpps_init_project_get_next_accession();

  // Show warning about using existing accession.
  if ($accession != 'new') {
    drupal_set_message(t('WARNING: <br />Study @accession will be overwriten.',
      ['@accession' => $accession]), 'warning'
    );
  }
  elseif ($accession == 'new') {
    $form['tpps_submission_import_notice'] = [
      '#markup' => t('New accession will be created: @next_accession.',
        ['@next_accession' => $next_accession]
      ),
    ];
    $form['tpps_submission_import_accession'] = [
      '#type' => 'hidden',
      '#value' => $accession,
    ];
  }
  else {
    // Accession.
    $accession_list = tpps_submission_get_accession_list();
    $form['tpps_submission_import_accession'] = [
      '#type' => 'select',
      '#title' => t('Accession'),
      '#options' => array_merge(
        [
          'new' => t('New (@next_accession)',
            ['@next_accession' => $next_accession]
          ),
        ],
        $accession_list
      ),
      '#default_value' => $accession ?? array_key_first($accession_list) ?? '',
      '#required' => TRUE,
    ];
  }
  $form['tpps_submission_import_submission_state'] = [
    '#type' => 'textarea',
    '#title' => t('Submission State'),
    '#default_value' => $form_state['input']['tpps_submission_import_submission_state'] ?? '',
  ];
  $form['#validate'][] = 'tpps_submission_import_form_validate';
  tpps_form_autofocus($form, 'tpps_submission_import_accession');
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  tpps_submission_list_add_buttons($form);
  return $form;
}

/**
 * Validation for form 'tpps_submission_import_form'.
 */
function tpps_submission_import_form_validate(array &$form, array &$form_state) {
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate accession.
  if (empty($form_state['values']['tpps_submission_import_accession'])) {
    // Note: $accession could be 'new'.
    form_set_error('tpps_submission_import_accession', t('Empty Accession.'));
    // @TODO Minor. Validate accession. Check if 'TGDR' is present and so on.
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate Submission State.
  if (empty($form_state['values']['tpps_submission_import_submission_state'])) {
    form_set_error('tpps_submission_import_submission_state', t('Submission state is empty.'));
  }
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

  module_load_include('inc', 'tpps', 'includes/common');
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
    // Get original (old) accession from $state (Submission Form State Array).
    $original_accession = tpps_form_get_accession($state);
    // Get new acession.
    $new_accession = $form_state['values']['tpps_submission_import_accession'];
    if ($new_accession == 'new') {
      $is_new = TRUE;
      $new_accession = tpps_init_project_get_next_accession();
    }

    // Since we allow both formats (TGDRxxx and xxx) we need to sync them.
    // @TODO Update code to use only 'xxx' format.
    $new_accession = 'TGDR' . str_replace('TGDR', '', $new_accession);
    // Update a list of imported studies.
    tpps_imported_studies_list_add_item($new_accession, $original_accession);
    // Files.
    tpps_submission_import_process_files($state, $new_accession, $form);
    if (!$is_new) {
      // Remove submission and files.
      // We can't use tpps_update_submission_state() because it doesn't manage files
      // but only overwrites record in chado.tpps_submission table and
      // redord in chado.dbxref will be left unchanged.
      tpps_submission_purge($new_accession, $force = TRUE);
    }
    tpps_submission_import_state_mods($state, $new_accession);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Add record to DB table.
    tpps_create_submission($state, $user->uid);
    // Clear cached list of studies to have new items at 'frontpage'.
    // @TODO Minor. Clear only accession list.
    cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
    // Recreate Submission.
    // Function drupal_form_submit() in case of success will redirect to
    // the confirmation page (/completed-submission/TGDRxxxx).
    // Note:
    // Submission Form Version will be upgraded here!
    // Function tpps_main_submit() will change version.
    drupal_form_submit(tpps_form_get_id($state), $state);
  }

  // When import failed we show useful links.
  // Note: This code will not be executed in case of success.
  $url_list = [
    'tppsc',
    "tpps-admin-panel/$new_accession",
    "tpps/details/$new_accession",
  ];
  foreach ($url_list as $url) {
    $items[] = l($url, $url);
  }
  drupal_set_message(t('Submission import failed. Useful links: !links',
    ['!links' => theme('item_list', ['items' => $items])]
  ));
  $form_state['rebuild'] = TRUE;
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
  $staging_domain = 'https://tgwebdev.cam.uchc.edu';
  $username = variable_get('tpps_staging_http_username', '');
  $password = variable_get('tpps_staging_http_password', '');
  if (strpos($file['url'], $staging_domain) === 0 && $username && $password) {
    $auth = base64_encode($username . ':' . $password);
    $content = file_get_contents($file['url'], FALSE, stream_context_create(
      ['http' => ['header' => 'Authorization: Basic ' . $auth]])
    );
  }
  else {
    // No HTTP Basic Auth.
    $content = file_get_contents($file['url']);
  }

  if (empty($content)) {
    return NULL;
  }
  // Move file to Drupal.
  $path = tpps_submission_import_get_folder($file['uri']) . '/' . $file['filename'];
  $file = file_save_data($content, $path, FILE_EXISTS_RENAME);
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

/**
 * Required modifications on Submission State array to import study.
 *
 * @param array $state
 *   Submission State.
 * @param string $new_accession
 *   New study accession. For example: TGDRxxxx.
 */
function tpps_submission_import_state_mods(array &$state, $new_accession) {
  // Note: $state['saved_values'] is required for normal form submission.
  // Fix Warning: end() expects parameter 1 to be array, null given in
  // file_managed_file_validate()
  // (line 607 of /var/www/Drupal/modules/file/file.module).
  $state['triggering_element']['#parents'] = ['submit'];
  // Fix Notice: Undefined index: #value in tpps_main_validate()
  // (line 1071 of /var/www/Drupal/sites/all/modules/TGDR/tpps.module).
  $state['triggering_element']['#value'] = t('Review and Submit');

  // $form_state['values']['frontpage']['accession'] = $new_accession;
  // Check also $form_state['input'] and $form_state['completed_form'].
  // Update accession value in $form_state.
  $old_accession = $state['saved_values']['frontpage']['accession'];
  $state = tpps_array_replace($state, $old_accession, $new_accession);
  $saved_values = &$state['saved_values'];

  // Store original accession.
  // @TODO Minor. Remove 'saved_values' but be sure to restore those values
  // when submission reopened using '/tppsc' form.
  $saved_values['tpps_submission_import'] = [];
  $submission_import = &$saved_values['tpps_submission_import'];
  $submission_import['original_accession'] = $old_accession;
  $submission_import['imported'] = REQUEST_TIME;
  $submission_import['imported_date']
    = format_date(REQUEST_TIME, 'custom', 'Y-m-d h:s:i O');
  $submission_import['site']
    = $state['tpps_submission_export_site'] ?? 'https://treegenesdb.org';
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Store imported (original) Submission Form Version.
  module_load_include('inc', 'tpps', 'includes/submission_form_version');
  // Set Submission Form Version.
  $saved_values[TPPS_SUBMISSION_FORM_VERSION_KEY]
    = $state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] ?? 1;
  $submission_import[TPPS_SUBMISSION_FORM_VERSION_KEY]
    = $state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // From tpps_init_project().
  $state['saved_values']['dbxref_id']
    = $state['dbxref_id']
      = $state['values']['dbxref_id']
        = tpps_init_project_get_dbxref_id($new_accession);

  module_load_include('php', 'tpps', 'forms/build/page_1');
  $state['ids']['project_id'] = tpps_get_project_id($state['dbxref_id']);
  $state['values']['status'] = 'Incomplete';
  $state['values'] = $state['saved_values'];
}

/**
 * Prepare files for import.
 *
 * Files will be downloaded, added to drupal's managed files. File Ids in
 * Submssion State will be updated.
 *
 * @param array $state
 *   Submission State.
 * @param string $new_accession
 *   New (local) study accession.
 * @param array $form
 *   Drupal Form API array which will be used to add report.
 */
function tpps_submission_import_process_files(array &$state, $new_accession, array &$form = []) {
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

      // Report.
      $message = t(
        'Original file #@original_fid was replaced with new file #@new_fid',
        ['@new_fid' => $new_file->fid, '@original_fid' => $fid]
      );
      drupal_set_message($message);
      tpps_form_add_report($form, [
        'import_file_' . $fid => [
          'title' => $message,
          'data' => 'Original file: <pre>' . print_r($file, 1) . '</pre>'
            . '<hr />New file: <pre>' . print_r($new_file, 1) . '</pre>',
        ],
      ]);
    }
  }
}
