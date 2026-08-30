<?php

/**
 * @file
 * Shows list of the files which do not belong to any submission.
 */

/**
 * Menu callback.
 *
 * Shows list of the files which do not belong to any submission.
 *
 * Files from 'file_managed' table which has no real file in filesystem.
 *
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 */
function tpps_report_file_orphan($reset = FALSE) {
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $cache_key = $cid;
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  if ($reset || empty($cache) || empty($cache->data[$cache_key])) {
    // Get new data.

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get list of the all files which belings to submissions.
    $used_list = [];
    $cached = SubmissionCache::getAll();
    foreach ($cached as $accession => $item) {
      if (empty($item['file']['id_list'])) {
        // Skip submissions withtout files.
        continue;
      }
      foreach (array_keys($item['file']['id_list']) as $fid) {
        if (!is_numeric($fid)) {
          // Get only file id.
          continue;
        }
        $used_list[] = $fid;
      }
    }
    if (empty($used_list)) {
      return '';
    }

    //dpm(print_r($used_list, 1));

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $i = 0;
    $query = db_select('file_managed', 'fm')
      ->fields('fm', ['fid', 'uri'])
      ->condition('fm.fid', $used_list, 'NOT IN')
      ->orderBy('fid', 'DESC');
    $query->leftJoin('file_usage', 'fu', 'fm.fid = fu.fid');
    $query->addField('fu', 'id');
    $file_list = $query->execute()->fetchAll();
    // Note $file_list will have ALL files!
    // dpm(reset($file_list));
    $rows = [];
    foreach ($file_list as $item) {
      $path = tpps_realpath($item->uri);
      $accession = '';
      if (!empty($item->id)) {
        $accession = 'TGDR' . $item->id;
        $submission = new Submission($accession);
        if (!$submission->doesNotExist()) {
          $accession = l($accession,
            'api/submission/' . $accession . '/view/shared_state/saved_values/',
            ['attributes' => ['title' => 'Submission exists']]
          );
        }
        else {
          $accession =
            '<div title="Submission not found.">'
            . $accession . '</div>';
        }
      }
      // File path.
      if (file_exists($path)) {
        $path = l($path,
          'api/file/' . $item->fid . '/info',
          [
            'query' => ['token' => variable_get('tpps_api_token')],
            'attributes' => ['title' => 'File exists'],
          ]
        );
      }
      else {
        $path = '<div title="File not found.">' . $path . '</div';
      }
      // Build table's row.
      $rows[] = [
        ++$i,
        $item->fid,
        $path,
        $accession,
      ];
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    unset($_GET['reset']);
    $cache->data[$cache_key] = $rows;
    cache_set($cid, $cache->data, $cache_bin);
  }
  $rows = $cache->data[$cache_key] ?? [];

  $header = [
    '#',
    t('File Id'),
    t('Absolute Path'),
    t('Accession')
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  return Report::print([
    'name' => 'missing_files',
    'header' => $header,
    'rows' => $rows,
    'pager' => 'both',
  ], $reset);
}
