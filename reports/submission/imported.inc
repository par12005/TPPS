<?php

/**
 * @file
 * Generates list of the imported studies.
 */

/**
 * Menu callback. Shows list of imported studies.
 */
function tpps_report_submission_imported() {
  module_load_include('inc', 'tpps', 'includes/imported_studies_list');
  module_load_include('inc', 'tpps', 'includes/submissions');
  module_load_include('inc', 'tpps', 'includes/submission_form_version');
  // Get list of all imported submissions.
  // Keys are accessions for the new (imported) studies and values are
  // source submission accessions.
  $list = ImportedSubmissionList::get();
  $rows = [];
  $params = [
    'query' => ['destination' => 'admin/reports/tpps/submission/imported'],
    'html' => TRUE,
  ];

  foreach ($list as $local => $remote) {
    $submission = new Submission($local);
    if ($submission->doesNotExist()) {
      tpps_message(
        'Submission @accession not exists.',
        ['@accession' => $local], 'error'
      );
      ImportedSubmissionList::remove($local);
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
    $accession_title = theme('submission_accession_link',
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
        theme('submission_accession_link', ['accession' => $local]),
        'tpps/submission/' . $local . '/view',
        $params
      ),
      $submission->state['status'] ?? NULL,
      $job_status ?? '-',
      $import_meta[TPPS_SUBMISSION_FORM_VERSION_KEY] ?? NULL,
      $import_meta['current_version'] ?? NULL,
      $submission->uid ?? NULL,
      $import_meta['full_date'] ?? NULL,
      implode(', ',
        Submission::getPageList([
          'accession' => $local,
          'destination' => 'admin/reports/tpps/submission/imported',
        ])
      ),
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
      t('Pages'),
    ],
    'rows' => $rows,
  ]);
  tpps_add_css_js('main');
  return $output;
}
