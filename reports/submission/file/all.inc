<?php

/**
 * @file
 * Shows list of all submission's files.
 */

/**
 * Menu callback. Shows list of all submission's files.
 */
function tpps_report_submission_file_all($accession = NULL) {
  $debug_mode = FALSE;

  $reset = $_GET['reset'] ?? FALSE;
  $cid = Report::getCacheKey('all', ['submission', 'file']);
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $file_column_list = [
    'tree-accession',
    'phenotype',
    'genotype',
    // Group 'other' used for files whith has fid but not belong to any group.
    'other',
  ];
  $options = ['query' => ['token' => variable_get('tpps_api_token')]];


  if ($reset || empty($cache->data)) {
    // Get list of all managed files.
    $existing_fid_list = db_select('file_managed', 't')
      ->fields('t', ['fid'])
      ->orderBy('fid')
      ->execute()
      ->fetchCol();
    $rows = [];

    // @TODO Change order to have latest first.
    $submission_list = tpps_load_submission_multiple();
    // WARNIG: ksort do not respect keys and they will be lost but asort()
    // doesn't work with objects in array values...
    krsort($submission_list, SORT_NATURAL);
    foreach ($submission_list as $submission) {
      $data = Report::buildRow('all', ['submission', 'file'],
        ['submission' => $submission]
      );
      if (!empty($data)) {
        $rows[$submission->accession] = $data;
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $cache->data = $rows;
    cache_set($cid, $cache->data, $cache_bin);

    // Redirect to URL without 'reset' query item.
    drupal_goto($_GET['q']);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show warnings.
  $message = t(
    'Cached version of report is show. Please <a href="@update_link">update'
    . '</a> to get most recent updates.',
    ['@update_link' => url($_GET['q'], ['query' => ['reset' => 1]])]
  );
  drupal_set_message($message, 'warning');

  $message = t(
    'Only studies which has files are shown. Useful reports: <a href="@none_link">'
    . 'List of studies without files</a>, <a href="@missing_link">List of '
    . 'studies which files are missing in filesystem</a>.',
    [
      '@missing_link' => url('admin/reports/tpps/submission/file/missing'),
      '@none_link' => url('admin/reports/tpps/submission/file/none'),
    ]
  );
  drupal_set_message($message, 'warning');
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Format output.
  $formatted_rows = [];
  $report_data = $cache->data;
  if ($accession) {
    $report_data = [
      $accession => $cache->data[$accession],
    ];
  }
  // Line counter.
  $i = 0;
  foreach ($report_data as $key => $value) {
    $formatted_rows[] = Report::themeRow('all', ['submission', 'file'], [
      'index' => ++$i,
      'key' => $key,
      'value' => $value,
      'file_column_list' => $file_column_list,
      'options' => $options,
    ]);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show report table.
  return Report::printTable([
    // 'report_name' => 'submissions_all_files',
    'header' => array_merge(
      ['#', t('Accession'), t('UID'), t('Stage'), t('Status')],
      $file_column_list
    ),
    'rows' => $formatted_rows,
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
    l(t('D'), 'api/file/' . $fid . '/get',
      array_merge($options, ['attributes' => ['title' => t('Download File')]])
    ),
    l(t('U'), 'api/file/' . $fid . '/usage',
      array_merge($options, ['attributes' => ['title' => t('Usage')]])
    ),
    l(t('U+'), 'api/file/' . $fid . '/usage/add/' . $accession,
      array_merge($options, ['attributes' => ['title' => t('Add 1 Usage')]])
    ),
    l(t('U-'), 'api/file/' . $fid . '/usage/delete/' . $accession,
      array_merge($options, ['attributes' => ['title' => t('Delete 1 Usage')]])
    ),
  ]) . ']';
}

/**
 * Builds data for single submission in 'submission_file' report.
 *
 * See Report::buildRow().
 *
 * @param array $context
 *   Data to build single row of the report. Keys are:
 *   'submission' - object. (optional when 'accession' is used).
 *      TPPS Submission.
 *   'reset' - bool. (optional).
 *     When TRUE cache will be updated. Default is FALSE.
 *   'accession' - string. (optional when 'submission' is used).
 *     Study accession. E.g., 'TGDRxxxx'.
 *
 * @return array
 *   Returns data to build single row in report 'Submisssion Files All'.
 */
function tpps_report_submission_file_all_build_single_row(array $context) {

  $submission = $context['submission'] ?? NULL;
  if (empty($submission) && !empty($context['accession'])) {
    $submission = new Submission($context['accession']);
  }
  $reset = $context['reset'] ?? FALSE;

  $data = [];
  $debug_mode = FALSE;
  $accession = $submission->accession;
  if (empty($file_list = $submission->getFileIdList())) {
    // When submission has no files it shouldn't be shown in the report.
    return NULL;
  }
  if ($debug_mode ?? FALSE) {
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
    if (
      // !in_array($fid, $existing_fid_list) ||
      empty($file = file_load($fid))
    ) {
      $file_row['other'][] = $fid;
      continue;
    }
    $organism_label = check_plain($organism_index . '. ' . $organism_name);
    $field_name = check_plain(end($parents));
    $file_row[$group][$organism_label][$field_name] = $fid;
  }
  $data = [
    'uid' => $submission->uid,
    'stage' => $submission->state['stage'],
    'status' => $submission->getStatus(),
    'file_row' => $file_row,
  ];
  $cid = Report::getCacheKey('all', ['submission', 'file']);
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  if ($reset || !isset($cache->data[$accession])) {
    // Update cache.
    $cache->data[$accession] = $data;
    cache_set($cid, $cache->data, $cache_bin);
  }
  return $data;
}

/**
 * Themes single row for 'submission_file_all' report.
 *
 * @param array $context
 *   Keys are:
 *   'key' - string. Submission accession. E.g., 'TGDRxxxx'.
 *   'value' - array. Data to theme the row. Keys are:
 *     'uid' - int. User Id of the study submitter.
 *     'stage' - mixed. Last submitted TPPS form step.
 *     'status' - string. Status of Submission.
 *     'file_row' - array. Nested array of file ids per group
 *       (phenotype/genotype and etc) and per organism name.
 *
 * @return array
 *   Returns row item for theme_table().
 */
function tpps_report_submission_file_all_theme_single_row(array $context) {
  $key = $context['key'];
  $value = $context['value'];

  $final_file_row = [];
  $accession = $key;
  // Convert to strings.
  // New array must be used to have correct order of keys.
  foreach ($context['file_column_list'] as $group) {
    if (!empty($value['file_row'][$group])) {
      foreach ($value['file_row'][$group] as $organism_label => $field) {
        if (empty($field) || !is_array($field)) {
          continue;
        }
        foreach ($field as $field_name => $fid) {
          $value['file_row'][$group][$organism_label][$field_name]
            = tpps_build_file_api_link($accession, $fid, $context['options']);
        }
      }
      $final_file_row[] = tpps_array_print_list($value['file_row'][$group]);
    }
    else {
      $final_file_row[] = '';
    }
  }
  return array_merge(
    [
      $context['index'],
      l($accession, 'api/submission/' . $accession . '/view/shared_state'),
      $value['uid'],
      $value['stage'],
      $value['status'],
    ],
    $final_file_row
  );
}
