<?php

/**
 * @file
 * Shows list of Submissions without files.
 */

/**
 * Menu callback. Lists submissions without files.
 */
function tpps_report_submission_file_none() {
  $file_name = str_replace('.inc', '',
    Report::getFilePath('all', ['submission', 'file'])
  );
  module_load_include('inc', 'tpps', $file_name);
  return tpps_report_submission_file('none');
}
