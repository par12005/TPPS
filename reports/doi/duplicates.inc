<?php

/**
 * @file
 * Shows list of studies which has the same Publication DOI.
 */

/**
 * Menu callback. Shows list of organisms per study.
 */
function tpps_report_doi_duplicates($reset = FALSE) {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $rows = [];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $report_data = [];
  foreach (SubmissionCache::getAll($reset) as $accession => $cached_submission) {
    $publication_doi = $cached_submission['publication']['publication_doi'] ?? NULL;
    if (empty($publication_doi)) {
      continue;
    }
    $report_data[$publication_doi][] = [
      'accession' => $accession,
      'project_id' => $cached_submission['project_id'] ?? NULL,
      'status' => $cached_submission['status'] ?? NULL,
    ];
  }
  $report_data = array_reverse($report_data);
  // Build report.
  $i = 0;
  foreach ($report_data as $publication_doi => $list) {
    $accession_list = [];
    if (count($list) >= 2) {
      foreach ($list as $item) {
        $accession_list[] = l(implode(' / ', $item), 'api/submission/'
          . $item['accession'] . '/view/shared_state/saved_values/'
          . TPPS_PAGE_1 . '/doi'
        );
      }
      $rows[] = [
        ++$i,
        PublicationDoi::print($publication_doi),
        theme('item_list', ['items' => $accession_list]),
      ];
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  // @TODO Minor. Add sorting.
  $header = [
    '#',
    t('Publication DOI'),
    t('Accession / Project Id / Status'),
  ];
  return Report::print([
    'name' => 'doi_duplicates',
    'header' => $header,
    'rows' => $rows,
    'pager' => 'both',
    'submission_cache' => TRUE,
  ], $reset);
}
