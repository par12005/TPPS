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
function tpps_report_submission_zenodo($reset = FALSE) {
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
      ZenodoDeposit::searchHost(['deposit_id' => $deposit_id]);
      $host = ZenodoHost::isLive() ? 'live' : 'sandbox';
      $publication_doi = $submission->getPublicationDOI();
      $dataset_doi = $submission->getDatasetDOI();
      $rows[] = [
        'number' => ++$i,
        'accession' => l(
          $accession,
          'api/submission/' . $accession . '/view/state'
        ),
        'project_id' => $submission->get(['ids', 'project_id']),
        'status' => $submission->status ?? NULL,
        'host' => ZenodoHost::isLive() ? t('Live') : t('Sandbox'),
        'deposit_id' => l($deposit_id,
          'admin/reports/tpps/zenodo/deposition/' . $host . '/' . $deposit_id
        ),
        'publication_doi' => $publication_doi,
        'dataset_doi' => $dataset_doi,
        'deposition_files' => l(t('View Files'),
          'tpps/submission/' . $accession . '/zenodo/files'
        ),
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
  $header = [
    '#',
    t('Accession'),
    t('Project Id'),
    t('Status'),
    t('Zenodo Host'),
    t('Zenodo Deposit Id'),
    t('Publication DOI'),
    t('Dataset DOI'),
    t('Deposition Files'),
  ];
  return Report::printTable([
    'header' => $header,
    'rows' => $rows,
  ]);
}
