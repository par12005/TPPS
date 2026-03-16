<?php

/**
 * @file
 * Shows list of the all studies which has Publication DOI.
 */

/**
 * Menu callback. Shows list of organisms per study.
 */
function tpps_report_doi_submission_publication($reset = FALSE) {
  $rows = [];
  $i = 0;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  foreach (SubmissionCache::getAll($reset) as $accession => $cached_submission) {
    $publication_doi = $cached_submission['publication']['publication_doi'] ?? NULL;
    if (empty($publication_doi)) {
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
      theme('doi_link', ['doi' => $publication_doi]),
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $name = 'doi_submission_publication';
  $header = [
    '#',
    t('Accession / Project Id / Status'),
    t('Publication DOI'),
  ];
  return Report::printTable([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No data'),
  ]) . SubmissionCache::showClearCacheWidget();
}
