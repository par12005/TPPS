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
    'html' => TRUE,
  ];

  // Build labels once to avoid extra work inside the loop.
  $labels = [
    // Actions:
    'view' => theme('fcbold', ['string' => t('View')]),
    'edit' => theme('fcbold', ['string' => t('Edit')]),
    'export' => theme('fcbold', ['string' => t('Export')]),
    'compare' => theme('fcbold', ['string' => t('Compare')]),
    'purge' => theme('fcbold', ['string' => t('Purge')]),
    // Pages:
    'admin_panel' => theme('fcbold', ['string' => t('Admin Panel')]),
    'details' => theme('fcbold', ['string' => t('Details (map)')]),
    'completed_submission' => theme('fcbold',
      ['string' => t('Completed Submission')]
    ),
    'files' => theme('fcbold', ['string' => t('Files')]),
  ];

  foreach ($list as $local => $remote) {
    $submission = new Submission($local);
    if ($submission->doesNotExist()) {
      tpps_message(
        'Submission @accession not exists.',
        ['@accession' => $local], 'error'
      );
      tpps_imported_studies_list_remove_item($local);
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

    // Original accession link.
    $accession_title = theme('study_accession',
      ['accession' => $import_meta['original_accession']]
    );
    if ($import_meta['site'] ?? NULL) {
      $accession_url = $import_meta['site']
        . '/tpps/submission/' . $import_meta['original_accession'] . '/export';
      $original_accession_link = l($accession_title, $accession_url, $params)
        . (($import_meta['site'] == 'https://treegenesdb.org')
          ? '@<strong>LIVE</strong>' : '@dev');
    }
    else {
      // Plain text if there is no information about original site.
      $original_accession_link = $accession_title;
    }
    $rows[] = [
      $original_accession_link ?? NULL,
      l(
        theme('study_accession', ['accession' => $local]),
        'tpps/submission/' . $local . '/view',
        $params
      ),
      $submission->state['status'] ?? NULL,
      $job_status ?? '-',
      $import_meta[TPPS_SUBMISSION_FORM_VERSION_KEY] ?? NULL,
      $import_meta['current_version'] ?? NULL,
      $submission->uid ?? NULL,
      $import_meta['full_date'] ?? NULL,
      // Actions:
      implode(' | ', [
        l($labels['view'], 'tpps/submission/' . $local . '/view', $params),
        l($labels['edit'], 'tppsc/' . $local, $params),
        l($labels['export'], 'tpps/submission/' . $local . '/export', $params),
        l($labels['compare'], 'tpps/submission/' . $local . '/compare', $params),
        l($labels['purge'], 'tpps/submission/' . $local . '/purge', $params),
      ]),
      // Pages:
      implode(' | ', [
        l($labels['admin_panel'], 'tpps-admin-panel/' . $local, $params),
        l($labels['details'], 'tpps/details/' . $local, $params),
        l(
          $labels['completed_submission'],
          'completed-submission/' . $local,
          $params
        ),
        l(
          $labels['files'],
          'tpps-admin-panel/file-diagnostics/' . $local,
          $params
        ),
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
      t('Original <br /><strong>Accession</strong>'),
      t('Imported<br /><strong>Accession</strong>'),
      t('Submission<br /><strong>Status</strong>'),
      t('Tripal Job<br /><strong>Status</strong>'),
      t('Original') . $version_tags,
      t('Submitted') . $version_tags,
      t('UID'),
      t('Import Date'),
      t('Actions'),
      t('Pages'),
    ],
    'rows' => $rows,
  ]);
  tpps_add_css_js('main');
  return $output;
}
