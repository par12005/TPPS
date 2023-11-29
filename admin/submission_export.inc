<?php

/**
 * @file
 * TPPS Submission Export Form and related functions.
 *
 * @TODO Minor. Get status of submission in
 * https://tgwebdev.cam.uchc.edu/admin/reports/db/public.tripal_jobs.
 * @TODO Check $form_state['saved_values'] for files but it's structure differs
 * from $form_state['value'] and splitted by pages. But some studies at
 * dev-server (maybe broken) has no 'values' but only 'saves_values' but in
 * reviewed studies files was detected correclty using 'comolete form'.
 */

module_load_include('inc', 'tpps', 'includes/common');
module_load_include('inc', 'tpps', 'admin/submission_list');

/**
 * TPPS Study Export Form.
 */
function tpps_submission_export_form(array $form, array &$form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);
  // Get Submission State.
  module_load_include('inc', 'tpps', 'includes/submissions');
  $state = tpps_submission_get_state($accession);
  tpps_submission_export_fixes($state);
  //drupal_set_message(tpps_table_display($state));
  // Files.
  $fid_list = tpps_submission_file_get_id_list($state, $form);
  $files = tpps_submission_export_get_files_info($fid_list);
  tpps_submission_export_file_report($fid_list, $files, $form);
  $state['tpps_submission_export_files'] = $files;

  $form['tpps_submission_export_state'] = [
    '#type' => 'textarea',
    '#title' => t('State PHP Array'),
    '#value' => var_export($state, 1),
    '#rows' => 10,
    '#resizable' => TRUE,
    '#attributes' => ['readonly' => 'readonly'],
  ];
  tpps_submission_list_add_buttons($form, $add_action = FALSE);
  tpps_add_css_js('submission_export', $form);
  return $form;
}

/**
 * Gets Submission file's metadata.
 *
 * @param array $fid_list
 *   List of File IDs.
 *
 * @return array
 *   Returns associative array where key is File Id and value is an array
 *   with File Metadata returned by file_load() and additionally new
 *   item 'url' which is File's absolute URL.
 */
function tpps_submission_export_get_files_info(array &$fid_list) {
  $files = [];
  foreach ($fid_list as $fid) {
    if ($fid) {
      $file = file_load($fid);
      if ($file) {
        $file->url = file_create_url($file->uri);
        $files[$fid] = (array) $file;
      }
    }
  }
  return $files;
}

/**
 * Show file detection report.
 *
 * @param array $fid_list
 *   List of File Ids.
 * @param array $files
 *   List of File metadata.
 * @param array $form
 *   Drupal Form API array to add report to.
 */
function tpps_submission_export_file_report(array &$fid_list = [], array &$files = [], array &$form = []) {
  if (!empty($fid_list)) {
    if (empty($files)) {
      drupal_set_message(
        t('Study has files but they not found at server.'
          . '<br/>Probably those files must be uploaded again.'
          . '<br />List of File IDs: @list',
          ['@list' => implode(', ', $fid_list)]
        ),
        'warning'
      );
    }
    else {
      drupal_set_message(t('Files was found and processed. See report below.'));
      tpps_submission_form_add_report($form, [
        'files_prepared_for_import' => [
          'title' => t('List of files prepared for import'),
          'data' => t('<pre>@list</pre>', ['@list' => print_r($files, 1)]),
        ],
      ]);
    }
  }
  else {
    drupal_set_message(
      t('No files found in this submission. Please double-check.'),
      'warning'
    );
  }
}

/**
 * Removes some minor elements which could broke import.
 *
 * The main goal is to export all the data without changes at all but
 * when it's not possible we need to remove them to allow import of more
 * important parts to be completed.
 *
 * @param array $state
 *   Submission state.
 */
function tpps_submission_export_fixes(array &$state) {
  // This is form element which has a lot of HTML code which breaks
  // parsing of state on import. See TGDR1089.
  unset($state['complete form']['comments']['#prefix']);
}
