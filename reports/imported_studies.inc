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
      $import_meta['full_date'],
      $import_meta['accession_link'],
      l($import_meta['site'], $import_meta['site']),
      // Actions:
      implode(' | ', [
        l(t('Admin Panel'), 'tpps-admin-panel/' . $local),
        l(t('View'), 'tpps/submission/' . $local . '/view'),
        l(t('Export'), 'tpps/submission/' . $local . '/export'),
        l(t('Compare'), 'tpps/submission/' . $local . '/compare'),
        l(t('Purge'), 'tpps/submission/' . $local . '/purge'),
        l(t('Completed Submission'), 'completed-submission/' . $local),
        l(t('Files'), 'tpps-admin-panel/file-diagnostics/' . $local),
      ]),
    ];
  }
  return theme('table', [
    'header' => [
      t('Local Accession'),
      t('Import Date'),
      t('Remote Accession'),
      t('Remote Site'),
      t('Actions'),
    ],
    'rows' => $rows,
  ]);
}
