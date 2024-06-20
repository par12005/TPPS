<?php

/**
 * @file
 * Shows list of Study missing files..
 */

/**
 * Menu callback. Shows list of files missing if filesystem.
 *
 * Files from 'tpps_submission' table which has no records in 'file_managed'
 * and no feal files if absolute path was used.
 */
function tpps_admin_study_missing_files_report() {
  $fid_list = db_select('file_managed', 't')
    ->fields('t', ['fid'])
    ->orderBy('fid')
    ->execute()->fetchCol();
  $result = [];

  //$conditions = ['status', 'Incomplete'];
  $conditions = [];
  $submission_list = tpps_load_submission_multiple($conditions);
  dpm(count($submission_list), 'count');
  foreach ($submission_list as $submission) {
    $files = tpps_submission_file_get_id_list($submission->state);
    if ($files) {
      // @TODO Check fid.
      $result[$submission->accession] = $files;
    }
    else {
      dpm('No files: ' . $submission->accession);
    }
  }
  dpm(count($result), 'count files');
  dpm(print_r($result, 1));

  // @TODO Implement.
  drupal_set_message(t('Not yet implemented'));
  return ' ';
}
