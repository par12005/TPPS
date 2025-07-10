<?php

/**
 * @file
 * Shows list of submissions which are missing files.
 */

/**
 * Menu callback. Shows list of submissions which are missing files.
 */
function tpps_report_submission_file_missing() {
  $file_name = str_replace('.inc', '',
    Report::getFilePath('all', ['submission', 'file'])
  );
  module_load_include('inc', 'tpps', $file_name);
  return tpps_report_submission_file('missing');
}
