<?php

/**
 * @file
 * Shows list of all submission's files.
 */

/**
 * Menu callback. Shows list of all submission's files.
 */
function tpps_report_submission_file_all() {
  $reset = $_GET['reset'] ?? FALSE;
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $cache_key = $cid;
  $file_column_list = [
    'tpps_accession',
    'tpps_phenotype',
    'tpps_genotype',
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
      $organism_number = $submission->sharedState['saved_values'][TPPS_PAGE_1]['organism']['number'] ?? 1;
      $accession = $submission->accession;
      if ($files = $submission->getFileIdList()) {
        $file_row = [];
        $final_file_row = [];
        foreach ($files as $fid) {
          if (in_array($fid, $existing_fid_list) && $file = file_load($fid)) {
            $api_file_link =
              l($fid, 'api/file/' . $fid . '/info',
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
            $found = FALSE;
            foreach ($file_column_list as $group) {
              // @todo Minor. Change format of the output and add organism name.
              // Format: 'org_name' : fid1, fid2, ... fidN.
              if ($group == 'other' || $found) {
                continue;
              }
              if (strpos($file->uri, $group) !== FALSE) {
                $found = TRUE;
                $token = NULL;
                // Accession files.
                if ($group == 'tpps_accession') {
                  $page3_tree_accession = $submission->sharedState['saved_values']
                    [TPPS_PAGE_3]['tree-accession'] ?? NULL;
                  for ($n = 1; $n <= $organism_number; $n++) {
                    $species_accession_fid = $page3_tree_accession['species-'
                      . $n]['file'] ?? NULL;
                    if ($fid == $species_accession_fid) {
                      // Get organism name from Page 1.
                      $token = $submission->sharedState['saved_values']
                        [TPPS_PAGE_1]['organism'][$n]['name'] ?? NULL;
                      $token = ' (' . $token . ')';
                      break;
                    }
                  }
                }
                // Genotype.
                elseif ($group == 'tpps_genotype') {
                  for ($n = 1; $n <= $organism_number; $n++) {
                    $submission_file_list = $submission->sharedState['saved_values']
                      [TPPS_PAGE_4]['organism-' . $n]['genotype']['files'] ?? [];
                    if ($token = array_search($fid, $submission_file_list)) {
                      $token = ' (' . $token . ')';
                      break;
                    }
                  }
                }
                // Phenotype.
                elseif ($group == 'tpps_phenotype') {
                  for ($n = 1; $n <= $organism_number; $n++) {
                    $phenotype = $submission->sharedState['saved_values']
                      [TPPS_PAGE_4]['organism-' . $n]['phenotype'] ?? NULL;
                    if ($fid == ($phenotype['file'] ?? NULL)) {
                      $token = ' (data)';
                      break;
                    }
                    elseif ($fid == ($phenotype['metadata'] ?? NULL)) {
                      $token = ' (meta)';
                      break;
                    }
                  }
                }
                $api_file_link .= $token ?? '';
                $file_row[$group][] = $api_file_link;
                break;
              }
              else {
                // @todo Minor. Check 'vcf' and 'local vcf' in $file->uri.
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
        // Process local VCF files.
        for ($n = 1; $n <= $organism_number; $n++) {
          if ($submission->hasLocalVcf($n)) {
            $file_row['other'][] = $submission->getOrganismName($n) . ': ' .
              $submission->getLocalVcf($n);
          }
        }
        // Convert to strings.
        // New array must be used to have correct order of keys.
        foreach ($file_column_list as $group) {
          if (empty($file_row[$group])) {
            $final_file_row[] = '';
            continue;
          }
          $final_file_row[] = theme('item_list', ['items' => $file_row[$group]]);
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
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    unset($_GET['reset']);
    $cache->data[$cache_key] = $rows;
    cache_set($cid, $cache->data, $cache_bin);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  // Show report table.
  return tpps_report_show_table([
    // 'report_name' => 'submissions_all_files',
    'header' => array_merge(
      ['#', t('Accession'), t('UID'), t('Stage'), t('Status')],
      $file_column_list
    ),
    'rows' => ($cache->data[$cache_key] ?? []),
  ])
  . theme('item_list', [
    'items' => [
      l(t('Clear TPPS Cache'), 'admin/config/tpps/cache'),
      t('WARNING: Only studies with files were shown. '
        . 'To get list of studies without file see report: ') .
      l(t('Submissions without Files'),
          'admin/reports/tpps/submission/file/missing'),
    ],
  ]);
}
