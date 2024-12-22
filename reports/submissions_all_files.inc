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
  $file_column_list = [
    'tpps_accession',
    'tpps_phenotype',
    'tpps_genotype',
    'other',
  ];


  if ($reset || empty($cache) || empty($cache->data[$key])) {
    // Get new data.
    $i = 0;
    // Get list of all managed files.
    $existing_fid_list = db_select('file_managed', 't')
      ->fields('t', ['fid'])
      ->orderBy('fid')
      ->execute()
      ->fetchCol();
    $rows = [];
    $options = ['query' => ['token' => variable_get('tpps_api_token')]];

    $submission_list = tpps_load_submission_multiple();
    foreach ($submission_list as $submission) {
      if ($files = tpps_submission_file_get_id_list($submission->state)) {
        $file_row = [];
        $final_file_row = [];
        foreach ($files as $fid) {
          if (in_array($fid, $existing_fid_list) && $file = file_load($fid)) {
            $api_file_link = l($fid, 'api/file/' . $fid . '/info', $options);
            $found = FALSE;
            foreach ($file_column_list as $group) {
              if ($group == 'other' || $found) {
                continue;
              }
              if (strpos($file->uri, $group) !== FALSE) {
                $found = TRUE;
                $file_row[$group][] = $api_file_link;
                break;
              }
            }
            if (!$found) {
              $file_row['other'][] = $api_file_link;
            }
          }
          else {
            $file_row['other'][] = $fid;
          }
        }
        // Convert to strings.
        // New array must be used to have correct order of keys.
        foreach ($file_column_list as $group) {
          if (empty($file_row[$group])) {
            $final_file_row[] = '';
            continue;
          }
          $final_file_row[] = implode(', ', $file_row[$group]);
        }

        $rows[] = array_merge([
          ++$i,
          l($submission->accession,
            'api/submission/' . $submission->accession . '/view/shared_state'),
          $submission->getStatus(),
        ], $final_file_row);
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
  // Show report table.
  return tpps_report_show_table([
    'report_name' => 'submissions_all_files',
    'header' => array_merge(
      ['#', t('Accession'), t('Status')],
      $file_column_list
    ),
    'rows' => $rows,
  ]);
}
