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
  $data = SubmissionCache::get();

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
      drupal_set_message(t("Submission @accession doesn't exist and was "
        . "removed from list of imported studies.", ['@accession' => $local],
        'error'
      ));
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
      $original_accession_link ?? NULL,
      l(theme('submission_accession_link', ['accession' => $local]),
        'tpps/submission/' . $local . '/view', $params
      ),
      $data[$local]['status'] ?? NULL,
      $data[$local]['tripal_job']['status'] ?? '-',
      $import_meta[Submission::FORM_VERSION_KEY] ?? NULL,
      $import_meta['current_version'] ?? NULL,
      $data[$local]['submitter']['uid'] ?? NULL,
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
    'empty' => t('No data'),
  ]);
  tpps_add_css_js('main');
  return $output . SubmissionCache::showClearCacheWidget();
}
