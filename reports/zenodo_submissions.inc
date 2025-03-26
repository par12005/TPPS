<?php

/**
 * @file
 * Shows list of studies which has Zenodo Depositions.
 *
 * See
 * - /src/zenodo/ZenodoDeposit.class.inc.
 * - https://developers.zenodo.org/?shell#depositions.
 */

/**
 * Menu callback. Shows list of submissions which has Zenodo Depositions.
 */
function tpps_admin_zenodo_submissions_report($reset = FALSE) {
  $reset = $_GET['reset'] ?? FALSE;
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $cache_key = $cid;
  if ($reset || empty($cache) || empty($cache->data[$cache_key])) {
    // Get new data.
    $i = 0;
    $submission_list = tpps_load_submission_multiple();
    foreach ($submission_list as $accession => $submission) {
      $deposit_id = $submission->getZenodoDepositId();
      if (empty($deposit_id)) {
        continue;
      }
      $publication_doi = $submission->get(
        ['saved_values', TPPS_PAGE_1, 'doi']
      ) ?? $submission->get(
        ['saved_values', TPPS_PAGE_1, 'publication_doi']
      ) ?? $submission->sharedState['saved_values'][TPPS_PAGE_1]['doi'] ?? '';
      $rows[] = [
        'number' => ++$i,
        'accession' => l(
          $accession,
          'api/submission/' . $accession . '/view/state'
        ),
        'project_id' => $submission->get(['ids', 'project_id']),
        'status' => $submission->status ?? NULL,
        'deposit_id' => (empty($publication_doi) ? $deposit_id :
          l($deposit_id, 'api/submission/' . $accession . '/zenodo/deposit')
        ),
        'publication_doi' => $publication_doi,
        'deposition_files' => (empty($publication_doi) ? '-' : l(
          t('View Files'), 'api/submission/' . $accession . '/zenodo/files'
        )),
      ];
    }
    // Reverse order of elements.
    //$data = array_reverse($data);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    unset($_GET['reset']);
    $cache->data[$cache_key] = $rows ?? [];
    cache_set($cid, $cache->data, $cache_bin);
  }
  $rows = $cache->data[$cache_key] ?? [];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $name = 'zenodo_submissions';
  $header = [
    '#',
    t('Accession'),
    t('Project Id'),
    t('Status'),
    t('Zenodo Deposit Id'),
    t('Publication DOI'),
    t('Deposition Files'),
  ];
  return tpps_report_show_table([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
  ]);
}
