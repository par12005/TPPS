<?php

/**
 * @file
 * Shows list of submissions which are missing files.
 */

/**
 * Menu callback. Shows list of submissions which are missing files.
 */
function tpps_report_submission_file_missing() {
  $reset = $_GET['reset'] ?? FALSE;
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $cache_key = $cid;
  if ($reset || empty($cache) || empty($cache->data[$cache_key])) {
    // Get new data.
    $i = 0;
    $fid_list = db_select('file_managed', 't')
      ->fields('t', ['fid'])
      ->orderBy('fid')
      ->execute()->fetchCol();
    $rows = [];

    $submission_list = tpps_load_submission_multiple();
    foreach ($submission_list as $submission) {
      if ($file_list = array_keys($submission->getFileIdList()) {
        $missing = [];
        foreach ($file_list as $fid) {
          if (!in_array($fid, $fid_list)) {
            $missing[] = $fid;
          }
        }
        if (!empty($missing)) {
          $rows[] = [
            ++$i,
            l($submission->accession,
              'tpps/submission/' . $submission->accession . '/json/state'
            ),
            implode(', ', $missing),
          ];
        }
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    unset($_GET['reset']);
    $cache->data[$cache_key] = $rows;
    cache_set($cid, $cache->data, $cache_bin);
  }
  $rows = $cache->data[$cache_key] ?? [];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $per_page = variable_get('tpps_report_items_per_page', 20);
  $header = ['#', t('Accession'), t('List of File Ids')];
  $current_page = pager_default_initialize(count($rows), $per_page);
  $chunks = array_chunk($rows, $per_page, TRUE);
  return theme('pager', ['quantity', count($rows)])
    . theme('table', ['header' => $header, 'rows' => $chunks[$current_page]])
    . theme('pager', ['quantity', count($rows)]);
}
