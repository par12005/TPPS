<?php

/**
 * @file
 * Shows list of studies which has the same Publication DOI.
 */

/**
 * Menu callback. Shows list of organisms per study.
 */
function tpps_admin_doi_duplicates_report($reset = FALSE) {

  $reset = $_GET['reset'] ?? FALSE;
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $key = $cid;
  if ($reset || empty($cache) || empty($cache->data[$key])) {
    // Get new data.

    // Get data.
    // @TODO Cache it.
    $data = [];
    $submission_list = tpps_load_submission_multiple();
    foreach ($submission_list as $accession => $submission) {
      $publication_doi = $submission->sharedState['saved_values'][TPPS_PAGE_1]['doi'] ?? NULL;
      $data[$publication_doi][] = [
        'accession' => $accession,
        'project_id' => $submission->sharedState['ids']['project_id'] ?? NULL,
        'status' => $submission->status ?? NULL,
      ];
    }
    $data = array_reverse($data);

    // Build report.
    $i = 0;
    foreach ($data as $publication_doi => $list) {
      $accession_list = [];
      if (count($list) >= 2) {
        foreach ($list as $item) {
          $accession_list[] = l(implode(' / ', $item), 'api/submission/'
            . $item['accession'] . '/view/shared_state/saved_values/1/doi'
          );
        }
        $rows[] = [
          ++$i,
          $publication_doi,
          theme('item_list', ['items' => $accession_list]),
        ];
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    unset($_GET['reset']);
    $cache->data[$key] = $rows ?? [];
    cache_set($cid, $cache->data, $cache_bin);
  }
  $rows = $cache->data[$key] ?? [];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $name = 'doi_duplicates';
  //$page_title = variable_get('tpps_report_' . $name . '_title', 'DOI Duplicates');
  //drupal_set_title($page_title);
  $header = ['#', t('Publication DOI'), t('Accession / Project Id / Status')];
  return tpps_report_show_table([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
  ]);
}
