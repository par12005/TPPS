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
  module_load_include('inc', 'tpps', 'includes/submission_form_version');
  $list = tpps_imported_studies_list_get();
  $rows = [];
  $params = [
    'query' => ['destination' => 'admin/reports/tpps/imported-studies'],
  ];
  foreach ($list as $local => $remote) {
    $import_meta = tpps_submission_get_import_meta([], $local);

    $rows[] = [
      l($local, 'tpps/submission/' . $local . '/view'),
      $import_meta[TPPS_SUBMISSION_FORM_VERSION_KEY],
      $import_meta['current_version'],
      $import_meta['full_date'],
      $import_meta['accession_link'],
      l($import_meta['site'], $import_meta['site']),
      // Actions:
      implode(' | ', [
        l(t('Edit'), 'tppsc/' . $local),
        l(t('Admin Panel'), 'tpps-admin-panel/' . $local),
        l(t('Details (map)'), 'tpps/details/' . $local),
        l(t('View'), 'tpps/submission/' . $local . '/view'),
        l(t('Export'), 'tpps/submission/' . $local . '/export'),
        l(t('Compare'), 'tpps/submission/' . $local . '/compare'),
        l(t('Purge'), 'tpps/submission/' . $local . '/purge', $params),
        l(t('Completed Submission'), 'completed-submission/' . $local),
        l(t('Files'), 'tpps-admin-panel/file-diagnostics/' . $local),
      ]),
    ];
  }
  $output = '<h3>'
    . l(t('Import New Submission'), 'tpps/submission/new/import') . '</h3>';
  $output .= theme('table', [
    'header' => [
      t('Local <br /><strong>Accession</strong>'),
      t('Imported <br/><strong>Version</strong>'),
      t('Current <br /><strong>Version</strong>'),
      t('Import Date'),
      t('Remote <br /><strong>Accession</strong>'),
      t('Remote Site'),
      t('Actions'),
    ],
    'rows' => $rows,
  ]);
  return $output;
}
