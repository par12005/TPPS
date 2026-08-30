<?php

/**
 * @file
 * Shows list of the all studies which has Dataset DOI.
 */

/**
 * Menu callback. Shows list of organisms per study.
 */
function tpps_report_doi_submission_dataset($reset = FALSE) {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $rows = [];
  $i = 0;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  foreach (SubmissionCache::getAll($reset) as $accession => $cached_submission) {
    $dataset_doi = $cached_submission['publication']['dataset_doi'] ?? NULL;
    if (empty($dataset_doi)) {
      continue;
    }
    $title = implode(' / ',
      [
        $accession ?? NULL,
        $cached_submission['project_id'] ?? NULL,
        $cached_submission['status'] ?? NULL,
      ]
    );
    $rows[] = [
      ++$i,
      // Accession link.
      theme('submission_accession_link', [
        'accession' => $accession,
        'path' => 'tpps/details/' . $accession,
        'show_version' => TRUE,
        'show_load_time' => TRUE,
      ])
      . theme('submission_pages', [
        'accession' => $accession,
      ]),
      theme('doi_link', ['doi' => $dataset_doi]),
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $name = 'doi_submission_dataset';
  $header = [
    '#',
    t('Accession / Project Id / Status'),
    t('Dataset DOI'),
  ];
  return Report::print([
    'name' => $name,
    'header' => $header,
    'rows' => $rows,
    'submission_cache' => TRUE,
  ], $reset);
}
