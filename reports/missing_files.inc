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

    $query = db_select('file_managed', 'fm')->fields('fm', ['fid', 'uri']);
    $query->join('file_usage', 'fu', 'fm.fid = fu.fid');
    $query->addField('fu', 'id');
    $query->orderBy('fid', 'DESC');
    $file_list = $query->execute()->fetchAll();
    // Note $file_list will have ALL files!
    // dpm(reset($file_list));
    $rows = [];
    $options = ['query' => ['token' => variable_get('tpps_api_token')]];
    foreach ($file_list as $item) {
      $path = tpps_realpath($item->uri);
      $accession = 'TGDR' . $item->id;
      $submission = new Submission($accession);
      if (!file_exists($path)) {
        $rows[] = [
          ++$i,
          l($item->fid, 'api/file/' . $item->fid . '/info', $options),
          $path,
          $submission->doesNotExist() ? $accession : l($accession,
          'api/submission/' . $accession . '/view/shared_state/saved_values/'),
        ];
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
  return tpps_report_show_table([
    'report_name' => 'missing_files',
    'header' => ['#', t('File Id'), t('Absolute Path'), t('Accession')],
    'rows' => $rows,
  ]);
}
