<?php

/**
 * @file
 * Shows list of submissions which are missing files.
 */

/**
 * Menu callback. Shows list of submissions which are missing files.
 */
function tpps_admin_submissions_missing_files_report() {
  $fid_list = db_select('file_managed', 't')
    ->fields('t', ['fid'])
    ->orderBy('fid')
    ->execute()->fetchCol();
  $rows = [];

  $submission_list = tpps_load_submission_multiple();
  foreach ($submission_list as $submission) {
    if ($files = tpps_submission_file_get_id_list($submission->state)) {
      $missing = [];
      foreach ($files as $fid) {
        if (!in_array($fid, $fid_list)) {
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
  drupal_set_message(t('Shown @number items from @total',
    ['@number' => count($rows), '@total' => count($submission_list)]
  ));

  return theme('table', [
    'header' => [t('Accession'), t('List of File Ids')],
    'rows' => $rows,
  ]);
}
