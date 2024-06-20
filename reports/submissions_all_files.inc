<?php

/**
 * @file
 * Shows list of all submission's files.
 */

/**
 * Menu callback. Shows list of all submission's files.
 */
function tpps_admin_submissions_all_files_report() {
  $per_page = variable_get('tpps_report_items_per_page', 20);
  $fid_list = db_select('file_managed', 't')->fields('t', ['fid'])
    ->orderBy('fid')->execute()->fetchCol();
  $rows = [];
  $options = ['query' => ['token' => variable_get('tpps_api_token')]];

  $submission_list = tpps_load_submission_multiple();
  foreach ($submission_list as $submission) {
    if ($files = tpps_submission_file_get_id_list($submission->state)) {
      $missing = [];
      foreach ($files as $fid) {
        if (in_array($fid, $fid_list)) {
          $missing[] = l($fid, 'api/file/' . $fid . '/info', $options);
        }
        else {
          $missing[] = $fid;
        }
      }
      if (!empty($missing)) {
        $rows[] = [
          l($submission->accession,
            'tpps/submission/' . $submission->accession . '/json/state'
          ),
          implode(', ', $missing),
        ];

      }
    }
  }
  drupal_set_message(t('Shown @number items from @total.',
    ['@number' => count($rows), '@total' => count($submission_list)]
  ));

  $header = [t('Accession'), t('List of File Ids')];
  // Initialize the pager.
  $current_page = pager_default_initialize(count($rows), $per_page);
  // Split your list into page sized chunks.
  $chunks = array_chunk($rows, $per_page, TRUE);
  return theme('pager', ['quantity', count($rows)])
    // Show the appropriate items from the list.
    . theme('table', ['header' => $header, 'rows' => $chunks[$current_page]])
    // Show the pager.
    . theme('pager', ['quantity', count($rows)]);
}
