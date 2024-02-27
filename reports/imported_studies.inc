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
    $interface = tpps_submission_interface_load($local) ?? [];
    $state = tpps_load_submission_state($local) ?? [];

    $rows[] = [
      l($local, 'tpps/submission/' . $local . '/view'),
      $import_meta[TPPS_SUBMISSION_FORM_VERSION_KEY],
      $import_meta['current_version'],
      (isset($state['saved_values'][1]['publication']['publication_doi']) ? 2 : 1),
      (isset($interface['saved_values'][1]['publication']['publication_doi']) ? 2 : 1),
      $import_meta['full_date'],
      $import_meta['accession_link'],
      l($import_meta['site'], $import_meta['site']),
      // Actions:
      implode(' | ', [
        // @TODO Use CSS class instead of HTML tags.
        '<strong>' . l(t('View'), 'tpps/submission/' . $local . '/view') . '</strong>',
        l(t('Edit'), 'tppsc/' . $local),
        l(t('Export'), 'tpps/submission/' . $local . '/export'),
        l(t('Compare'), 'tpps/submission/' . $local . '/compare'),
        l(t('Purge'), 'tpps/submission/' . $local . '/purge', $params),
      ]),
      // Pages:
      implode(' | ', [
        l(t('Admin Panel'), 'tpps-admin-panel/' . $local),
        l(t('Details (map)'), 'tpps/details/' . $local),
        l(t('Completed Submission'), 'completed-submission/' . $local),
        l(t('Files'), 'tpps-admin-panel/file-diagnostics/' . $local),
      ]),
    ];
  }
  $output = '<h3>'
    . l(t('Import New Submission (@next_accession)',
      ['@next_accession' => 'TGDR' . tpps_init_project_get_next_accession()]),
      'tpps/submission/new/import', ['attributes' => ['target' => '_blank']]
    )
    . ' | ' . l(t('Purge Multiple'),
      'tpps/submission/purge_multiple',
      ['attributes' => ['target' => '_blank']]
    )
    . '</h3>';
  $version_tags = '<br /><strong>' . t('Version') . '</strong>';
  $output .= theme('table', [
    'header' => [
      t('Imported<br /><strong>Accession</strong>'),
      t('Original') . $version_tags,
      t('Submitted') . $version_tags,
      t('State') . $version_tags,
      t('Interface') . $version_tags,
      t('Import Date'),
      t('Original <br /><strong>Accession</strong>'),
      t('Original Site'),
      t('Actions'),
      t('Pages'),
    ],
    'rows' => $rows,
  ]);
  return $output;
}
