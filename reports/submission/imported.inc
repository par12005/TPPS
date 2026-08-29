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
  // Get list of all imported submissions.
  // Keys are accessions for the new (imported) studies and values are
  // source submission accessions.
  $list = ImportedSubmissionList::get();
  $data = SubmissionCache::getAll();

  // Reset single item.
  if (isset($_GET['reset'])) {
    // List of imported studies is always smaller then list of all submissions
    // so update only items from the imported studies list.
    foreach ($list as $local => $remote) {
      $submission = new Submission($local);
      $data[$local] = SubmissionCache::buildSingle($submission);
    }
    SubmissionCache::set($data);
    // Redirect to URL without 'reset' query item.
    drupal_goto($_GET['q']);
  }

  $rows = [];
  $params = [
    'query' => ['destination' => 'admin/reports/tpps/submission/imported'],
    'html' => TRUE,
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get report's content.
  foreach ($list as $local => $remote) {
    if (!isset($data[$local])) {
      ImportedSubmissionList::remove($local);
      $message = t("Submission @accession doesn't exist and was removed from "
        . "list of imported studies.", ['@accession' => $local]
      );
      drupal_set_message($message, 'error');
      continue;
    }
    $import_meta = $data[$local]['import_meta'];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Original study.
    if ($import_meta['site'] ?? NULL) {
      $original_accession_link = theme('submission_accession_link',
        [
          'accession' => $import_meta['original_accession'],
          'path' => $import_meta['site']
            . '/tpps/submission/' . $import_meta['original_accession'] . '/export',
        ]
      ) . (($import_meta['site'] == 'https://treegenesdb.org')
          ? '@<strong>LIVE</strong>' : '@dev');
    }
    else {
      // Plain text if there is no information about original site.
      $original_accession_link = theme('submission_accession_link',
        ['accession' => $import_meta['original_accession']]
      );
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $rows[] = [
      'original_accession' => $original_accession_link ?? NULL,
      'imported_accession' => l(
        theme('submission_accession_link', ['accession' => $local]),
        'tpps/submission/' . $local . '/view', $params
      ),
      'pages' => implode(', ', Submission::getPageList([
        'accession' => $local,
        'destination' => 'admin/reports/tpps/submission/imported',
      ])),
      'submission_status' => $data[$local]['status'] ?? NULL,
      'job_status' => $data[$local]['tripal_job']['status'] ?? '-',
      'original_version' => $import_meta[SubmissionVersion::FORM_KEY] ?? NULL,
      'submitted_version' => $import_meta['current_version'] ?? NULL,
      'uid' => $data[$local]['submitter']['uid'] ?? NULL,
      'import_date' => $import_meta['full_date'] ?? NULL,
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

  $header = [
    [
      'data' => t('Original <br /><strong>Accession</strong>'),
      'field' => 'original_accession',
    ],
    [
      'data' => t('Imported<br /><strong>Accession</strong>'),
      'field' => 'imported_accession',
    ],
    // Note: no sorting is allowed.
    ['data' => t('Pages')],
    [
      'data' => t('Submission<br /><strong>Status</strong>'),
      'field' => 'submission_status',
    ],
    [
      'data' => t('Tripal Job<br /><strong>Status</strong>'),
      'field' => 'job_status',
    ],
    [
      'data' => t('Original') . $version_tags,
      'field' => 'original_version',
    ],
    [
      'data' => t('Submitted') . $version_tags,
      'field' => 'submitted_version',
    ],
    [
      'data' => t('UID'),
      'field' => 'uid',
    ],
    [
      'data' => t('Import Date'),
      'field' => 'import_date',
    ],
  ];

  return Report::print([
    'name' => 'submission_imported',
    'header' => $header,
    'rows' => $rows,
    'pager' => 'both',
    'submission_cache' => TRUE,
  ]);
}
