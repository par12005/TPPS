<?php

/**
 * @file
 * Shows list of all submission's files.
 */

/**
 * Menu callback. Shows list of all submission's files.
 */
function tpps_admin_submissions_all_files_report() {
  $reset = $_GET['reset'] ?? FALSE;
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $key = $cid;
  if ($reset || empty($cache) || empty($cache->data[$key])) {
    // Get new data.
    $i = 0;
    $fid_list = db_select('file_managed', 't')->fields('t', ['fid'])
      ->orderBy('fid')->execute()->fetchCol();
    $rows = [];
    $options = ['query' => ['token' => variable_get('tpps_api_token')]];

    $submission_list = tpps_load_submission_multiple();
    foreach ($submission_list as $submission) {
      if ($files = tpps_submission_file_get_id_list($submission->state)) {
        $missing = [];
        foreach ($files as $fid) {
          if (in_array($fid, $fid_list)) {
            $missing[] = l($fid, 'api/file/' . $fid . '/info', $options);
          }
          else {
            $missing[] = $fid;
          }
        }
        if (!empty($missing)) {
          $rows[] = [
            ++$i,
            l($submission->accession,
              'tpps/submission/' . $submission->accession . '/json/state'),
            implode(', ', $missing),
          ];

        }
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    unset($_GET['reset']);
    $cache->data[$key] = $rows;
    cache_set($cid, $cache->data, $cache_bin);
  }
  $rows = $cache->data[$key] ?? [];

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
