<?php

/**
 * @file
 * Shows list of Submissions without files.
 */

/**
 * Menu callback. Lists submissions without files.
 */
function tpps_admin_submissions_without_files_report() {
  $items = [];
  $submission_list = tpps_load_submission_multiple();
  foreach ($submission_list as $submission) {
    $files = tpps_submission_file_get_id_list($submission->state);
    if ($submission->state['stage'] >= 3 && !$files) {
      $items[] = l($submission->accession, 'tpps/submission/'
        . $submission->accession . '/json/state/saved_values/');
    }
  }
  drupal_set_message(
    t('Number of submissions without files: @number from @total',
    ['@number' => count($items), '@total' => count($submission_list)]
  ));
  return theme('item_list', ['items' => $items]);
}
