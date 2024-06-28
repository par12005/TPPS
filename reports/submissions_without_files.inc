<?php

/**
 * @file
 * Shows list of Submissions without files.
 */

/**
 * Menu callback. Lists submissions without files.
 */
function tpps_admin_submissions_without_files_report() {
  $rows = [];
  $submission_list = tpps_load_submission_multiple();
  $i = 0;
  foreach ($submission_list as $submission) {
    $files = tpps_submission_file_get_id_list($submission->state);
    if ($submission->state['stage'] >= 3 && !$files) {
      $rows[] = [
        ++$i,
        l($submission->accession, 'api/submission/'
        . $submission->accession . '/view/shared_state/saved_values/'),
        $submission->getStatus(),
        $submission->state['stage'],
      ];
    }
  }
  drupal_set_message(
    t('Number of submissions without files: @number from @total',
    ['@number' => count($rows), '@total' => count($submission_list)]
  ));
  return tpps_report_show_table([
    'report_name' => 'submissions_without_files',
    'header' => ['#', t('Accession'), t('Status'), t('Stage')],
    'rows' => $rows,
  ]);
}
