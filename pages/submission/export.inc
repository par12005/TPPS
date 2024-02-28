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

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * TPPS Study Export Form.
 */
function tpps_submission_export_form(array $form, array &$form_state, $accession = '') {
  global $base_url;
  tpps_submission_list_validate_accession($accession);

  $accession_list = tpps_submission_get_accession_list();
  $form['tpps_submission_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => array_merge([
      'new' => t(
        'New (@next_accession)',
        ['@next_accession' => 'TGDR' . tpps_init_project_get_next_accession()]
      ),
    ], $accession_list),
    '#default_value' => $accession ?? array_key_first($accession_list) ?? '',
    '#required' => TRUE,
  ];
  // Get Submission State.
  $state = tpps_load_submission_state($accession);
  tpps_submission_export_fixes($state);

  // Files.
  $fid_list = tpps_submission_file_get_id_list($state, $form);
  $files = tpps_submission_export_get_files_info($fid_list);
  if (empty($form_state['input'])) {
    // This check allows to avoid double processing files when new accession
    // was selected and button 'Export' clicked to update data.
    tpps_submission_export_file_report($fid_list, $files, $form);
  }
  $state['tpps_submission_export_files'] = $files;
  $state['tpps_submission_export_site'] = $base_url;

  $form['tpps_submission_export_state'] = [
    '#type' => 'textarea',
    '#title' => t('State PHP Array'),
    '#value' => var_export($state, 1),
    '#rows' => 10,
    '#resizable' => TRUE,
    '#attributes' => ['readonly' => 'readonly'],
    '#states' => [
      'visible' => [
        ':input[name="tpps_submission_accession"]' => ['value' => $accession],
      ],
    ],
  ];
  tpps_submission_list_add_buttons($form);
  // Add extra 'Manual Export' button.
  $form['actions']['manual_export'] = [
    '#type' => 'submit',
    '#name' => 'manual_export',
    '#value' => t('Manual Export'),
    '#submit' => ['tpps_submission_export_manual_submit'],
  ];
  tpps_form_autofocus($form, 'tpps_submission_export_state');
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('submission_export', $form);
  tpps_add_css_js('theme', $form);
  $form['report']['#states'] = [
    'visible' => [
      ':input[name="tpps_submission_accession"]' => ['value' => $accession],
    ],
  ];
  return $form;
}

/**
 * Form submitter. Redirects to export page when user selected other accession.
 */
function tpps_submission_export_form_submit(array $form, array $form_state) {
  $accession = $form_state['values']['tpps_submission_accession'];
  tpps_submission_list_validate_accession($accession);
  drupal_goto('tpps/submission/' . $accession . '/export');

}

/**
 * Form submitter. Redirects to manual export page.
 */
function tpps_submission_export_manual_submit(array $form, array $form_state) {
  $accession = $form_state['values']['tpps_submission_accession'];
  tpps_submission_list_validate_accession($accession);
  drupal_goto('tpps/submission/' . $accession . '/manual_export');
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
      $message = t('Study has files but they not found at server.'
        . '<br/>Probably those files must be uploaded again.'
        . '<br />List of File IDs: @list',
        ['@list' => implode(', ', $fid_list)]
      );
      drupal_set_message($message, 'warning');
    }
    else {
      drupal_set_message(t('Files was found and processed. See report below.'));
      tpps_form_add_report($form, [
        'files_prepared_for_import' => [
          'title' => t('List of files prepared for import'),
          'data' => t('<pre>@list</pre>', ['@list' => print_r($files, 1)]),
        ],
      ]);
    }
  }
  else {
    $message = t('No files found in this submission. Please double-check.');
    drupal_set_message($message, 'warning');
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
