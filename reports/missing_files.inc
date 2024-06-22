<?php

/**
 * @file
 * Generates list of the imported studies.
 */

/**
 * Menu callback. Shows list of files missing if filesystem.
 *
 * Files from 'file_managed' table which has no real file in filesystem.
 *
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 */
function tpps_admin_missing_files_report($reset = FALSE) {
  $reset = $_GET['reset'] ?? FALSE;
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $key = $cid;
  if ($reset || empty($cache) || empty($cache->data[$key])) {
    // Get new data.
    $i = 0;
    $file_list = db_select('file_managed', 't')
      ->fields('t', ['fid', 'uri'])->orderBy('fid', 'DESC')
      ->execute()->fetchAllKeyed();
    $rows = [];
    foreach ($file_list as $fid => $uri) {
      // Get absolute path.
      if (
        strpos($uri, 'public://') !== FALSE
        || strpos($uri, 'temporary://') !== FALSE
      ) {
        // Managed file.
        if (empty($path = drupal_realpath($uri))) {
          $path = $uri;
        }
      }
      else {
        // Manually uploaded file.
        $path = $uri;
      }
      if (!file_exists($path)) {
        $rows[] = [++$i, $fid, $path];
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
  $header = ['#', t('File Id'), t('Absolute Path')];
  $current_page = pager_default_initialize(count($rows), $per_page);
  $chunks = array_chunk($rows, $per_page, TRUE);
  return theme('pager', ['quantity', count($rows)])
    . theme('table', ['header' => $header, 'rows' => $chunks[$current_page]])
    . theme('pager', ['quantity', count($rows)]);
}
