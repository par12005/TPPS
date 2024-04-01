<?php

/**
 * @file
 * TPPS Submission Import Form and related functions.
 *
 * @TODO Minor. There is no ability to specify accession. Only next one could
 * be used or existing one overritten.
 * @TODO Minor. Review function tpps_form_state_info() which could be used
 * to reduce size of the SIA.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');
module_load_include('inc', 'tpps', 'includes/imported_studies_list');

/**
 * TPPS Submission Import Form.
 */
function tpps_submission_import_form(array $form, array &$form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);
  $next_accession = 'TGDR' . tpps_init_project_get_next_accession();
  $accession = ($accession == 'new') ? $next_accession : $accession;

  if ($accession) {
    // @TODO Minor. Check if accession already in use.
    drupal_set_message(t('WARNING: <br />Existing study @accession will be overwriten.',
      ['@accession' => $accession]), 'warning'
    );
    $form['tpps_submission_import_notice'] = [
      '#markup' => t('New accession will be created: @accession.',
        ['@accession' => $accession]
      ),
    ];
    $form['tpps_submission_import_accession'] = [
      '#type' => 'hidden',
      '#value' => $accession,
    ];
  }
  else {
    // Select accession.
    $accession_list = tpps_submission_get_accession_list();
    $new_accession = [
      'new' => t('New (@accession)', ['@accession' => $next_accession]),
    ];
    $form['tpps_submission_import_accession'] = [
      '#type' => 'select',
      '#title' => t('Accession'),
      '#options' => array_merge($new_accession, $accession_list),
      '#default_value' => $accession ?? '',
      '#required' => TRUE,
    ];
  }
  $form['tpps_submission_import_state'] = [
    '#type' => 'textarea',
    '#title' => t('Submission State'),
    '#default_value' => $form_state['input']['tpps_submission_import_state'] ?? '',
  ];
  $form['tpps_submission_import_upgrade_version'] = [
    '#type' => 'checkbox',
    // @TODO Minor. Allow to select version.
    '#title' => t('Upgrade Version of the Submission Form'),
    '#default_value' => $form_state['input']['tpps_submission_import_upgrade_version'] ?? TRUE,
    '#description' => t('Default is checked.'
      . '<br /> Whech checked Submission Form Version will be upgraded to latest.'
      . '<br /> Whech unchecked Submission Form Version will NOT be upgraded but '
      . 'Submission will be imported as is (original version will be stored).'
    )
  ];
  $form['tpps_submission_import_status'] = [
    '#type' => 'select',
    '#title' => t('Status after import'),
    '#options' => [
      'Do not change' => t('Do not change'),
      'Pending Approval' => t('Pending Approval'),
      'Incomplete' => t('Incomplete'),
      // Just setting 'Approved' status don't allow to run Tripal Job just
      // after import. Required more work with code.
      'Approved' => t('Approved'),
      'Submission Job Running' => t('Submission Job Running'),
    ],
    '#default_value' => $form_state['input']
    ['tpps_submission_import_status'] ?? 'Do not change',
    '#description' => t('Default is "Approved" which allows to run Tripal job just after import.'),
  ];
  $form['#validate'][] = 'tpps_submission_import_form_validate';
  tpps_form_autofocus($form, ['tpps_submission_import_accession']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  tpps_submission_list_add_buttons($form);
  return $form;
}

/**
 * Validation for form 'tpps_submission_import_form'.
 */
function tpps_submission_import_form_validate(array &$form, array &$form_state) {
  $values = $form_state['values'] ?? NULL;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate accession.
  if (empty($values['tpps_submission_import_accession'])) {
    // Note: $accession could be 'new'.
    form_set_error('tpps_submission_import_accession', t('Empty Accession.'));
    // @TODO Minor. Validate accession. Check if 'TGDR' is present and so on.
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate Submission State.
  if (empty(trim($values['tpps_submission_import_state'] ?? ''))) {
    form_set_error('tpps_submission_import_state', t('Submission state is empty.'));
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission Status field is required.
  if (
    empty($values['tpps_submission_import_status'])
    || !in_array($values['tpps_submission_import_status'],
      [
        'Do not change',
        'Pending Approval',
        'Incomplete',
        'Approved',
        'Submission Job Running',
      ]
    )
  ) {
    //form_set_error('tpps_submission_import_status', t('Submission status is wrong.'));
  }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/**
 * Form submitter. Imports submission from the dump (with files).
 */
function tpps_submission_import_form_submit(array $form, array &$form_state) {
  global $user;
  module_load_include('inc', 'tpps', 'includes/common');

  $values = $form_state['values'] ?? NULL;
  $upgrade_version = $values['tpps_submission_import_upgrade_version'] ?? TRUE;
  // @TODO Add to import form as a checkbox.
  $submit_form = $values['tpps_submission_import_submit_form'] ?? FALSE;
  $submission_new_status = $values['tpps_submission_import_status'] ?? 'Do not change';

  // Get local accession.
  if ($values['tpps_submission_import_accession'] == 'new') {
    $accession = tpps_init_project_get_next_accession();
  }
  else {
    $accession = $values['tpps_submission_import_accession'];
  }
  // Since we allow both formats (TGDRxxx and xxx) we need to sync them.
  $accession = 'TGDR' . str_replace('TGDR', '', $accession);

  // Remove existing study (if any).
  if ($values['tpps_submission_import_accession'] != 'new') {
    // Remove submission and files.
    $submission = new Submission($accession);
    $submission->purge();
  }

  $state_raw = trim($values['tpps_submission_import_state']);

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
  // Now $state contains submitted submission state array.

  // Get original (old) accession from $state (Submission Form State Array).
  $submission = new Submission();
  $submission->state = $state;
  //$original_accession = tpps_form_get_accession($state);
  $original_accession = $submission->accession;

  // Modify before files processing to have updated import meta data.
  // Store original accession.
  // @TODO Minor. Remove 'saved_values' but be sure to restore those values
  // when submission reopened using '/tppsc' form.
  module_load_include('inc', 'tpps', 'includes/submission_form_version');
  $state['saved_values']['tpps_submission_import'] = [
    'original_accession' => $submission->accession,
    'imported' => REQUEST_TIME,
    'imported_date' => format_date(REQUEST_TIME, 'custom', 'Y-m-d h:s:i O'),
    'site' => $submission->state['tpps_submission_export_site'] ?? 'https://treegenesdb.org',
    TPPS_SUBMISSION_FORM_VERSION_KEY
      => $submission->state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] ?? 1,
  ];
  // Files.
  tpps_submission_import_process_files($submission->state, $accession, $form);
  // Main modification must be done after file processing complete.
  tpps_submission_import_state_mods($submission->state, $accession);
  // We need to restore original accession because function
  // tpps_submission_import_state_mods() overwrites it.
  $submission->state['saved_values']['tpps_submission_import']['original_accession']
    = $submission->accession;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add record to DB table. Status will be 'Incomplete'.
  // @TODO Create earlier. or better use save().
  $submission->create($submission->state, $user->uid);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Status.
  // Having 'Incomplete' status is safe if we then programmatically submit form
  // but this status requires a lot of extra work to run Tripal job so
  // better to have 'Pending Approval' status or maybe even 'Approved'
  //
  if ($submission_new_status != 'Do not change') {
    $submission->info['status']
      = $submission->state['status']
      = $submission->state['values']['status']
      = $submission->state['saved_values']['status']
      = $submission_new_status;
  }
  // Update a list of imported studies.
  tpps_imported_studies_list_add_item($accession, $submission->accession);
  // Clear cached list of studies to have new items at 'frontpage'.
  // @TODO Minor. Clear only accession list.
  cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);

  if ($upgrade_version) {
    // Upgrade to latest Submission Form Version.
    tpps_submission_form_version_upgrade_to_latest($submision->state, $submission->state);
    drupal_set_message(t('Submission Form Version was upgraded.'));
    // Note: SharedState will be stored to db.
    $submission->generateSharedState();
    $submission->save();
  }

  if ($submit_form) {
    // Recreate Submission.
    // Function drupal_form_submit() in case of success will redirect to
    // the confirmation page (/completed-submission/TGDRxxxx).
    // Note:
    // Submission Form Version will be upgraded here!
    // Function tpps_main_submit() will change version.

    // Form submission was disabled because submission version
    // upgraded and downgraded multiple times.
    // To submit form Drupal builds it again so tpps_form_build_main()
    // will be called before tpps_main_submit().
    // @TODO Check if incompleted submissions processed correctly.
    drupal_set_message(t('Submission was submitted.'));
    drupal_form_submit(tpps_form_get_id($state), $state);
  }

  drupal_goto('completed-submission/' . $accession);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Helper functions.

/**
 * Form submitter. Shows submision dump with line numbers.
 */
function tpps_submission_import_review_dump(array $form, array &$form_state) {
  if ($state = trim($form_state['values']['tpps_submission_import_state'])) {
    // Note: $state is a string.
    // Show with line numbers.
    $state_array = preg_split("/\r\n|\n|\r/", $state);
    drupal_set_message('<pre>' . print_r($state_array, 1) . '</pre>');
  }
  $form_state['rebuild'] = 1;
}

/**
 * Downloads remote file and add to Drupal.
 *
 * @param array $state
 *   Submission Form State Array.
 *   Used to get import metadata information.
 * @param array $file
 *   File's metadate.
 *   Required items: 'filename' and 'url'.
 *
 * @return object
 *   Returns Managed File Object.
 */
function tpps_submission_import_download_file(array $state, array $file) {
  $staging_domain = 'https://tgwebdev.cam.uchc.edu';
  $username = variable_get('tpps_staging_http_username', '');
  $password = variable_get('tpps_staging_http_password', '');
  $url = $file['url'] ?? NULL;
  $site = $state['saved_values']['tpps_submission_import']['site'] ?? NULL;
  if (
    variable_get('tpps_file_use_api', FALSE)
    // Manually uploaded files. Those files are stored in shared folder
    // which could be accessible by production and dev-servers (not tested).
    || (($url[0] == '/' || strpos($url, 'http') === FALSE) && !empty($site))
    || empty($url)
  ) {
    $url = $site . '/tpps/file/' . $file['fid'] . '/get/'
      // Secure Token.
      . tpps_file_generate_token($file)
      // This part is not required but allows to get correct
      // filename of downloaded file.
      . '/' . $file['filename'];
  }
  if (empty($content = tpps_file_get_contents($url))) {
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Store imported (original) Submission Form Version.
  module_load_include('inc', 'tpps', 'includes/submission_form_version');
  // Set Submission Form Version.
  $saved_values[TPPS_SUBMISSION_FORM_VERSION_KEY]
    = $state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] ?? 1;
  // @TODO Check if this is required.
  //unset($state['tpps_submission_export_site']);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // From tpps_init_project().
  $state['saved_values']['dbxref_id']
    = $state['dbxref_id']
      = $state['values']['dbxref_id']
        = tpps_init_project_get_dbxref_id($new_accession);

  module_load_include('php', 'tpps', 'forms/build/page_1');
  $state['ids']['project_id'] = tpps_get_project_id($state['dbxref_id']);
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
      $new_file = tpps_submission_import_download_file($state, $file);
      if (empty($new_file)) {
        drupal_set_message(
          t('File @url is empty.', ['@url' => $file['url']]), 'error'
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
