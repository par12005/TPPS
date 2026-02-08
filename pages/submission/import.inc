<?php

/**
 * @file
 * TPPS Submission Import Form and related functions.
 *
 * @TODO Minor. Implement ability to set accession number. Currently there is
 * no ability to specify accession. Only next one could be used
 * or existing one overritten.
 * @TODO Minor. Set Submission::STATUS_APPROVED status and do necessary changes.
 * Just setting Submission::STATUS_APPROVED status don't allow to run Tripal
 * Job just after import. Required more work with code.
 */

module_load_include('inc', 'tpps', 'includes/imported_studies_list');
define('TPPS_DO_NOT_CHANGE_STATUS', 'Do not change');

/**
 * TPPS Submission Import Form.
 */
function tpps_submission_import_form(array $form, array &$form_state, $accession = '') {
  TppsForm::validateAccession($accession);
  $next_accession = 'TGDR' . tpps_init_project_get_next_accession();
  $accession = ($accession == 'new') ? $next_accession : $accession;

  if ($accession) {
    // @TODO Minor. Check if accession already in use.
    drupal_set_message(t('WARNING: <br />'
      . 'If study @accession exists it  will be overwriten.',
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
    $accession_list = SubmissionCache::getAccessionList();
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
    '#title' => t('Upgrade Submission Form Version'),
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
    '#options' => array_merge(
      [TPPS_DO_NOT_CHANGE_STATUS => t(TPPS_DO_NOT_CHANGE_STATUS)],
      TppsForm::getSubmissionStatusList()
    ),
    '#default_value' => $form_state['input']['tpps_submission_import_status']
    ?? TPPS_DO_NOT_CHANGE_STATUS,
  ];
  $form['#validate'][] = 'tpps_submission_import_form_validate';
  TppsForm::autofocus($form, ['tpps_submission_import_accession']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  TppsForm::addSubmissionActionButtons($form);
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
    || (
      $values['tpps_submission_import_status'] != TPPS_DO_NOT_CHANGE_STATUS
      && !in_array(
        $values['tpps_submission_import_status'],
        Submission::getStatusList()
      )
    )
  ) {
    form_set_error('tpps_submission_import_status', t('Submission status is wrong.'));
  }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/**
 * Form submitter. Imports submission from the dump (with files).
 */
function tpps_submission_import_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'tpps', 'includes/common');

  $values = $form_state['values'] ?? NULL;
  $accession = $values['tpps_submission_import_accession'];
  $new_accession = tpps_submission_import([
    // Required:
    'accession' => $accession,
    'export' => trim($values['tpps_submission_import_state']),
    // Optional:
    'upgrade_version' => $values['tpps_submission_import_upgrade_version'] ?? NULL,
    // @TODO Add to import form as a checkbox.
    'new_status' => $values['tpps_submission_import_status'] ?? NULL,
    'submit_form' => $values['tpps_submission_import_submit_form'] ?? NULL,
  ]);

  drupal_goto('completed-submission/' . $new_accession);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Helper functions.

/**
 * Form submitter. Shows submission dump with line numbers.
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
    // Trailing filename allows to get original filename when downloading.
    $url = $site . '/api/file/' . $file['fid'] . '/get/' . $file['filename'];
    $url .= '?token=' . variable_get('tpps_api_token');
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

  // Imported study has no data loaded into db and we must clear loading time.
  $state['loaded'] = NULL;
  // Ids are filled during pipeline processing. Must be empty.
  $state['ids'] = [];
  $state['job_id'] = NULL;

  // $form_state['values']['frontpage']['accession'] = $new_accession;
  // Check also $form_state['input'] and $form_state['completed_form'].
  // Update accession value in $form_state.
  $old_accession = $state['saved_values']['frontpage']['accession'];

  $original_submission = new Submission();
  $original_submission->state = $state;

  // Protect local VCF files from changes to reuse the same files for
  // imported studies. If path to local VCF file contains study accession it
  // will be replaced with new accession and link became broken.
  // Let's replace absolute path with it's md5-sum and restore original path
  // after accession replacement.
  $local_vcf_list = [];
  for ($i = 1; $i <= $original_submission->getOrganismNumber(); $i++) {
    $original_path = $original_submission->getLocalVcf($i);
    if (empty($original_path)) {
      continue;
    }
    $local_vcf_list[$i] = $original_path;
    // Hide path to local VCF from change if it includes study accession.
    // @todo Minor. Hide only paths which has accession.
    $state = TppsArray::replace(
      $state,
      $original_path,
      md5($original_path)
    );
  }

  // Replace accession.
  $state = TppsArray::replace($state, $old_accession, $new_accession);

  // Restore Local VCF file's path.
  foreach ($local_vcf_list as $original_path) {
    if (empty($original_path)) {
      continue;
    }
    $state = TppsArray::replace(
      $state,
      md5($original_path),
      $original_path
    );
  }

  $saved_values = &$state['saved_values'];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Store imported (original) Submission Form Version.
  // Set Submission Form Version.
  // @TODO Minor. Use SubmissionVersion::get($state);
  $saved_values[SubmissionVersion::FORM_KEY]
    = $state['saved_values'][SubmissionVersion::FORM_KEY]
    ?? SubmissionVersion::V1;
  // @TODO Check if this is required.
  //unset($state['tpps_submission_export_site']);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // From tpps_init_project().
  $state['saved_values']['dbxref_id']
    = $state['dbxref_id']
      = $state['values']['dbxref_id']
        = tpps_init_project_get_dbxref_id($new_accession);
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
 *
 * @throw Exception
 *   Throws exception if new file is empty.
 */
function tpps_submission_import_process_files(array &$state, $new_accession) {
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
        $message = t('File @url is empty.', ['@url' => $file['url']]);
        throw new Exception($message);
      }
      // To avoid errors when page navigation used we need to have
      // temporary stored files until final form submit ('summarypage').
      // @TODO Minor. Could $submission->setFileStatus() be used?
      if ($new_file->status != 0) {
        $new_file->status = 0;
        file_save($new_file);
      }
      TppsFile::addUsage($new_file, $new_accession);
      // Change File ID in submission state.
      // WARNING:
      // If $fid matches other number it will be replaced too which is wrong.
      // @TODO Replace only files using list of parents instead of update
      // whole $state.
      $state = TppsArray::replace($state, $fid, $new_file->fid);

      // Report.
      $message = t(
        'Original file #@original_fid was replaced with new file #@new_fid',
        ['@new_fid' => $new_file->fid, '@original_fid' => $fid]
      );
      drupal_set_message($message);
    }
  }
}

/**
 * Creates new submission from spesial export array.
 *
 * @param array $data
 *   Keys are:
 *   - 'accession' string
 *       Required. Submission accession. Format: 'TGDRxxxx' (or 'xxxx').
 *   - 'export'
 *       Required. Special export array. See Submission::export().
 *   - 'upgrade_version' mixed
 *       Optional. Default is TRUE.
 *       When TRUE Submission Version must be upgraded.
 *       When FALSE must not.
 *       NULL when wasn't set.
 *   - 'new_status' string
 *       Optional. Default is TPPS_DO_NOT_CHANGE_STATUS.
 *   - 'submit_form'
 *       Optional. Default is FALSE.
 *
 * @return string
 *   Returns new accession.
 */
function tpps_submission_import(array $data) {
  global $user;
  // Get local accession.
  if ($data['accession'] == 'new') {
    $accession = tpps_init_project_get_next_accession();
  }
  else {
    $accession = $data['accession'];
  }
  // Since we allow both formats (TGDRxxx and xxx) we need to sync them.
  $accession = 'TGDR' . str_replace('TGDR', '', $accession);
  // Remove existing study (if any).
  if ($data['accession'] != 'new') {
    // Remove submission and files.
    tpps_submission_load($accession)->purge();
  }
  // Get special export array.
  $state_raw = trim($data['export']);
  $new_status = $data['new_status'] ?? TPPS_DO_NOT_CHANGE_STATUS;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Fix 'stdClass' in the submission's dump which was created by var_export().
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

  // Modify before files processing to have updated import meta data.
  // Store original accession.
  // @TODO Minor. Remove 'saved_values' but be sure to restore those values
  // when submission reopened using '/tppsc' form.
  $submission->state['saved_values']['tpps_submission_import'] = [
    'original_accession' => $submission->accession,
    'imported' => REQUEST_TIME,
    'imported_date' => format_date(REQUEST_TIME, 'custom', 'Y-m-d h:s:i O'),
    'site' => $submission->state['tpps_submission_export_site'] ?? 'https://treegenesdb.org',
    SubmissionVersion::FORM_KEY => SubmissionVersion::get($submission->state),
  ];
  // Files.

  tpps_submission_import_process_files($submission->state, $accession);
  // Main modification must be done after file processing complete.
  tpps_submission_import_state_mods($submission->state, $accession);
  // We need to restore original accession because function
  // tpps_submission_import_state_mods() overwrites it.
  $submission->state['saved_values']['tpps_submission_import']['original_accession']
    = $submission->accession;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $submission->create($submission->state, $user->uid);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Status.
  //
  // Having Submission::STATUS_INCOMPLETE status is safe if we then
  // programmatically submit form but this status requires a lot of
  // extra work to run Tripal job so better to have status
  // Submission::STATUS_PENDING_APPROVAL
  // or maybe even Submission::STATUS_APPROVED.
  if (in_array($new_status, Submission::getStatusList())) {
    $submission->status = $new_status;
  }
  else {
    $submission->status = $submission->get(['status']);
  }
  // Update a list of imported studies.
  ImportedSubmissionList::add($accession, $submission->accession);
  // Clear cached list of studies to have new items at 'frontpage'.
  tpps_cache_clear_all(['tpps_submission_get_accession_list']);

  if ($data['upgrade_version'] ?? TRUE) {
    // Upgrade to latest Submission Form Version.
    $submission->state = SubmissionVersion::upgrade($submission->state);
    drupal_set_message(t('Submission Form Version was upgraded.'));
    // Note: SharedState will be stored to db.
  }
  $submission->generateSharedState();
  $submission->save();

  if ($data['submit_form'] ?? FALSE) {
    // Recreate Submission.
    // Function drupal_form_submit() in case of success will redirect to
    // the confirmation page (/completed-submission/TGDRxxxx).
    // Note:
    // Submission Form Version will be upgraded here!
    // Function tpps_main_submit() will change version.

    // Form submission was disabled because submission version
    // upgraded and downgraded multiple times.
    // To submit form Drupal builds it again so TppsForm::buildMain()
    // will be called before tpps_main_submit().
    // @TODO Check if incompleted submissions processed correctly.
    drupal_set_message(t('Submission was submitted.'));
    drupal_form_submit(TppsForm::getId($submission->state), $state);
  }
  else {
    // If we don't submit form we need manually change status of attached files
    // from temporaty to permanent to avoid it's automatic removement.
    $submission->setFileStatus(FILE_STATUS_PERMANENT, []);
  }
  SubmissionCache::updateSingle($submission);
  return $accession;
}
