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
    $submission = new Submission($local);
    if ($submission->doesNotExist()) {
      tpps_message(
        'Submission @accession not exists.',
        ['@accession' => $local], 'error'
      );
      continue;
    }
    $import_meta = $submission->getImportMeta();

    // Get Tripal Job status.
    if (!empty($submission->sharedState['job_id'])) {
      try {
        $job = new TripalJob();
        $job->load($submission->sharedState['job_id']);
        $job_status = $job->getJob()->status;
      }
      catch (Exception $e) {
        if ($e->getMessage() != "Cannot find a job with this ID provided.") {
          throw $e;
        }
      }
    }
    $rows[] = [
      l($local, 'tpps/submission/' . $local . '/view'),
      $submission->state['status'] ?? NULL,
      $job_status ?? '-',
      $import_meta[TPPS_SUBMISSION_FORM_VERSION_KEY] ?? NULL,
      $import_meta['current_version'] ?? NULL,
      $submission->uid ?? NULL,
      $import_meta['full_date'] ?? NULL,
      $import_meta['accession_link'] ?? NULL,
      l($import_meta['site'] ?? NULL, $import_meta['site'] ?? NULL),
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
      t('Submission<br /><strong>Status</strong>'),
      t('Tripal Job<br /><strong>Status</strong>'),
      t('Original') . $version_tags,
      t('Submitted') . $version_tags,
      t('UID'),
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
