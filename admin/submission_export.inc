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
//module_load_include('inc', 'tpps', 'includes/submissions');
//module_load_include('inc', 'tpps', 'includes/submission_file');

/**
 * TPPS Study Export Form.
 */
function tpps_submission_export_form(array $form, array &$form_state) {
  $accession = (($form_state['input']['op'] ?? NULL) == t('Reset')
    ? '' : ($form_state['input']['tpps_submission_export_accession'] ?? '')
  );
  $form['tpps_submission_export_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => tpps_submission_get_accession_list(),
    '#default_value' => $accession,
    '#required' => TRUE,
  ];
  $show_report = variable_get('tpps_submission_export_show_file_search_report', FALSE);
  $form['tpps_submission_export_show_file_search_report'] = [
    '#type' => 'checkbox',
    '#title' => 'Show File Search Report?',
    '#default_value' => $show_report,
  ];
  if (!empty($accession)) {
    // Get Submission State.
    module_load_include('inc', 'tpps', 'includes/submissions');
    $state = tpps_submission_get_state($accession);
    tpps_submission_export_fixes($state);
    //drupal_set_message(tpps_table_display($state));
    // Files.
    $fid_list = tpps_submission_file_get_id_list($state, $show_report);
    $files = tpps_submission_export_get_files_info($fid_list);
    tpps_submission_export_file_report($fid_list, $files);
    $state['tpps_submission_export_files'] = $files;

    $form['tpps_submission_export_state'] = [
      '#type' => 'textarea',
      '#title' => t('State PHP Array'),
      '#value' => var_export($state, 1),
      '#rows' => 10,
      '#resizable' => TRUE,
      '#attributes' => ['readonly' => 'readonly'],
    ];
  }
  $form['action_export'] = [
    '#type' => 'submit',
    '#value' => t('Export'),
    '#submit' => ['tpps_submission_export_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_submission_export_form_reset'],
  ];
  tpps_add_css_js('submission_export', $form);
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_submission_export_form_reset() {
  $form_state['input']['tpps_submission_export_accession'] = '';
  $form_state['values']['tpps_submission_export_accession'] = '';
  drupal_goto('admin/config/tpps/submission/export');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_export_form_submit(array $form, array &$form_state) {
  variable_set('tpps_submission_export_show_file_search_report',
    ($form_state['values']['tpps_submission_export_show_file_search_report'] ?? FALSE)
  );
  $form_state['rebuild'] = 1;
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
    $file = file_load($fid ?? '');
    if ($file && $file->filesize) {
      $file->url = file_create_url($file->uri);
      $files[$fid] = (array) $file;
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
 */
function tpps_submission_export_file_report(array &$fid_list = [], array &$files = []) {
  if (!empty($fid_list)) {
    if (empty($files)) {
      drupal_set_message(
        t('Study has files but they not found at server. '
        . 'Probably those files must be uploaded again.'), 'warning'
      );
      drupal_set_message(t('List of File IDs: <pre>@list</pre>',
        ['@list' => print_r($fid_list, 1)]
      ));
    }
    else {
      drupal_set_message(t('Files was found and processed.'));
      if (variable_get('tpps_submission_export_show_file_search_report', FALSE)) {
        drupal_set_message(t(
          'List of files prepared for import: <pre>@list</pre>',
          ['@list' => print_r($files, 1)]
        ));
      }
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
