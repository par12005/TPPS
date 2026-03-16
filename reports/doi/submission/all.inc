<?php

/**
 * @file
 * Shows list of the all studies which has Publication or Dataset DOI.
 */

/**
 * Menu callback. Shows list of organisms per study.
 */
function tpps_report_doi_submission_all($reset = FALSE) {
  $rows = [];
  $i = 0;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  foreach (SubmissionCache::getAll($reset) as $accession => $cached_submission) {
    $publication_doi = $cached_submission['publication']['publication_doi'] ?? NULL;
    $dataset_doi = $cached_submission['publication']['dataset_doi'] ?? NULL;
    if (empty($publication_doi) && empty($dataset_doi)) {
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
      l($title, 'api/submission/'
        . $accession . '/view/shared_state/saved_values/'
        . TPPS_PAGE_1 . '/doi'
      ),
      $publication_doi ?? NULL,
      $dataset_doi ?? NULL,
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $name = 'doi_submission_all';
  $header = [
    '#',
    t('Accession / Project Id / Status'),
    t('Publication DOI'),
    t('Dataset DOI'),
  ];
  return Report::printTable([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No data'),
  ]) . SubmissionCache::showClearCacheWidget();
}
