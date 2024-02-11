<?php

/**
 * @file
 * Generates list of the imported studies.
 */

/**
 * Menu callback. Shows list of imported studies.
 */
function tpps_admin_imported_studies_report() {
  module_load_include('inc', 'tpps', 'includes/imported_studies_list');
  module_load_include('inc', 'tpps', 'includes/submissions');
  $list = tpps_imported_studies_list_get();
  $rows = [];
  foreach ($list as $local => $remote) {
    $import_meta = tpps_submission_get_import_meta([], $local);
    $rows[] = [
      l($local, 'tpps/submission/' . $local . '/view'),
      $import_meta['accession_link'],
      $import_meta['full_date'],
      l($import_meta['site'], $import_meta['site']),
    ];
  }
  return theme('table', [
    'header' => [
      t('Local Accession'),
      t('Remote Accession'),
      t('Import Date'),
      t('Site'),
    ],
    'rows' => $rows,
  ]);
}
