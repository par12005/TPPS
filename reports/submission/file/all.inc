<?php

/**
 * @file
 * Shows list of all submission's files.
 */

/**
 * Menu callback. Shows list of all submission's files.
 */
function tpps_report_submission_file_all() {
  $debug_mode = FALSE;

  $reset = $_GET['reset'] ?? FALSE;
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $cache_key = $cid;
  $file_column_list = [
    'tree-accession',
    'phenotype',
    'genotype',
    // Group 'other' used for files whith has fid but not belong to any group.
    'other',
  ];

  if ($reset || empty($cache) || !isset($cache->data[$cache_key])) {
    // Line counter.
    $i = 0;
    // Get list of all managed files.
    $existing_fid_list = db_select('file_managed', 't')
      ->fields('t', ['fid'])
      ->orderBy('fid')
      ->execute()
      ->fetchCol();
    $rows = [];
    $options = ['query' => ['token' => variable_get('tpps_api_token')]];

    // @TODO Change order to have latest first.
    $submission_list = tpps_load_submission_multiple();
    // WARNIG: ksort do not respect keys and they will be lost but asort()
    // doesn't work with objects in array values...
    krsort($submission_list, SORT_NATURAL);
    foreach ($submission_list as $submission) {
      $accession = $submission->accession;
      if (empty($file_list = $submission->getFileIdList())) {
        continue;
      }
      if ($debug_mode) {
        dpm(print_r($file_list, 1), $submission->accession);
      }

      // List of files per group.
      $file_row = [];
      // Rendered lists of files.
      $final_file_row = [];

      // @todo Minor. Check 'vcf' and 'local vcf' in $file->uri.
      // Process local VCF files.
      for ($n = 1; $n <= $submission->getOrganismNumber(); $n++) {
        if ($submission->hasLocalVcf($n)) {
          $organism_name = $submission->getOrganismName($n);
          $organism_label = check_plain($n . '. ' . $organism_name);
          $file_row['other'][$organism_label]['local_vcf'] = $submission->getLocalVcf($n);
        }
      }

      foreach ($file_list as $fid => $parents) {
        $step = $parents[0];
        // 3: 'tree-accession';
        // 4: 'phenotype', 'genotype', '...'
        switch ($step) {
          case TPPS_PAGE_3:
            $group = $parents[1];
            $organism_index = str_replace('species-', '', $parents[2]);
            break;

          case TPPS_PAGE_4:
            $group = $parents[2];
            $organism_index = str_replace('organism-', '', $parents[1]);
            break;

          default:
            $group = 'other';
            $organism_index = NULL;
            break;
        }
        $organism_name = $submission->getOrganismName($organism_index);

        // Missing files goes to the 'other' group..
        if (!in_array($fid, $existing_fid_list) || empty($file = file_load($fid))) {
          $file_row['other'][] = $fid;
          continue;
        }
        $organism_label = check_plain($organism_index . '. ' . $organism_name);
        $field_name = check_plain(end($parents));

        $file_row[$group][$organism_label][$field_name] = tpps_build_file_api_link(
          $accession,
          $fid,
          $options
        );
      }

      // Convert to strings.
      // New array must be used to have correct order of keys.
      foreach ($file_column_list as $group) {
        if (!empty($file_row[$group])) {
          $final_file_row[] = tpps_array_print_list($file_row[$group]);
        }
        else {
          $final_file_row[] = '';
        }
      }

      $rows[] = array_merge([
        ++$i,
        l($submission->accession,
          'api/submission/' . $submission->accession . '/view/shared_state'),
        $submission->uid,
        $submission->state['stage'],
        $submission->getStatus(),
      ], $final_file_row);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $cache->data[$cache_key] = $rows;
    cache_set($cid, $cache->data, $cache_bin);

    // Redirect to URL without 'reset' query item.
    drupal_goto($_GET['q']);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  // Show report table.
  return Report::printTable([
    // 'report_name' => 'submissions_all_files',
    'header' => array_merge(
      ['#', t('Accession'), t('UID'), t('Stage'), t('Status')],
      $file_column_list
    ),
    'rows' => ($cache->data[$cache_key] ?? []),
  ])
  . theme('item_list', [
    'items' => [
      l(t('To reset cached report'), $_GET['q'], ['query' => ['reset' => 1]]),
      l(t('Clear TPPS Cache'), 'admin/config/tpps/cache'),
      t('WARNING: Only studies with files were shown. '
        . 'To get list of studies without file see report: ') .
      l(t('Submissions without Files'),
          'admin/reports/tpps/submission/file/missing'),
    ],
  ]);
}

/**
 * Builds set of links to File API pages.
 *
 * @param string $accession
 *   Submission accession. E.g., 'TGDRxxxx'.
 * @param int $fid
 *   Drupal's managed file id.
 * @param array $options
 *   Links' options.
 *
 * @return string
 *   Returns set of the HTML links which points to File API actions.
 */
function tpps_build_file_api_link($accession, $fid, array $options) {
  $fid = check_plain($fid);
  $accession = check_plain($accession);

  return l($fid, 'api/file/' . $fid . '/info',
    array_merge($options, ['attributes' => ['title' => t('View Info')]])
  )
  . ' [' . implode('|', [
    l(t('G'), 'api/file/' . $fid . '/get',
      array_merge($options, ['attributes' => ['title' => t('Get File')]])
    ),
    l(t('U'), 'api/file/' . $fid . '/usage',
      array_merge($options, ['attributes' => ['title' => t('Usage')]])
    ),
    l(t('A'), 'api/file/' . $fid . '/add/' . $accession,
      array_merge($options, ['attributes' => ['title' => t('Add Usage')]])
    ),
    l(t('D'), 'api/file/' . $fid . '/delete/' . $accession,
      array_merge($options, ['attributes' => ['title' => t('Delete Usage')]])
    ),
  ]) . ']';
}
