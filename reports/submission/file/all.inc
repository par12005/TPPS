<?php

/**
 * @file
 * Shows list of all submission's files.
 */

/**
 * Menu callback. Shows list of all submissions and their files.
 */
function tpps_report_submission_file_all() {
  $filter = explode('/', $_GET['q'])[5];
  return tpps_report_submission_file($filter);
}

/**
 * Shows filtered list of all submission's files.
 */
function tpps_report_submission_file($filter = NULL) {
  $debug_mode = FALSE;

  $data = SubmissionCache::getAll();
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $options = ['query' => ['token' => variable_get('tpps_api_token')]];
  $accession = explode('/', $_GET['q'])[6] ?? NULL;
  if (empty($is_single = (strpos($accession, 'TGDR') !== FALSE))) {
    $accession = NULL;
  }
  if ($filter == 'all' || $is_single) {
    $file_column_list = [
      Report::TREE_ACCESSION_FILES,
      Report::PHENOTYPE_FILES,
      Report::GENOTYPE_FILES,
      // Group Report::OTHER_FILES (which is 'other') will be used for files
      // which has fid but doesn't belong to any group.
      Report::OTHER_FILES,
    ];
  }
  elseif ($filter == Report::MISSING_FILES) {
    $file_column_list = [Report::MISSING_FILES];
  }
  elseif ($filter == 'none') {
    $file_column_list = [];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Format output.
  if ($is_single) {
    $report_data = [$accession => $data[$accession]];
  }
  else {
    if ($filter == 'all') {
      $report_data = $data;
    }
    elseif ($filter == Report::MISSING_FILES) {
      $report_data = [];
      foreach (array_keys($data) as $accession) {
        if (!empty($data[$accession]['file'][Report::MISSING_FILES])) {
          $report_data[$accession] = $data[$accession];
        }
      }
    }
    elseif ($filter == 'none') {
      $report_data = [];
      foreach (array_keys($data) as $accession) {
        if (empty($data[$accession]['file'][Report::FILE_ID_LIST])) {
          $report_data[$accession] = $data[$accession];
        }
      }
    }
  }

  $formatted_rows = [];
  // Line counter.
  $i = 0;
  foreach ($report_data as $key => $value) {
    if (empty($value)) {
      continue;
    }
    $formatted_rows[] = Report::themeRow('all', ['submission', 'file'], [
      'index' => ++$i,
      'key' => $key,
      'value' => $value,
      'file_column_list' => $file_column_list,
      'options' => $options,
      'report_filter' => $filter,
      'is_single' => $is_single,
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
    'empty' => t('No data'),
  ]) . SubmissionCache::showClearCacheWidget();
}

/**
 * Themes single row for 'submission_file_all' report.
 *
 * @param array $context
 *   Cached Submission's data (item of the SubmissionCache).
 *   See SubmissionCache::buildSingle() for the structure.
 *
 * @return array
 *   Returns row item for theme_table().
 */
function tpps_report_submission_file_all_theme_single_row(array $context) {
  $debug_mode = FALSE;
  $key = $context['key'];
  $value = $context['value'];

  if ($debug_mode ?? FALSE) {
    dpm(print_r($value, 1));
  }

  $final_file_row = [];
  $accession = $key;
  // Convert to strings.
  // New array must be used to have correct order of keys.
  //
  //dpm($context);
  if ($context['is_single'] || $context['report_filter'] == 'all') {
    foreach ($context['file_column_list'] as $group) {
      if (empty($value['file']['grouped_list'][$group])) {
        $final_file_row[] = '';
        continue;
      }
      foreach ($value['file']['grouped_list'][$group] as $organism_label => $field) {
        if (empty($field) || !is_array($field)) {
          continue;
        }
        foreach ($field as $field_name => $fid) {
          if (in_array($fid, $value['file'][Report::MISSING_FILES])) {
            // File is missing in filesystem.
            $value['file']['grouped_list'][$group][$organism_label][$field_name]
              = check_plain($fid);
          }
          else {
            // File exists.
            $value['file']['grouped_list'][$group][$organism_label][$field_name]
              = theme('file_id',
                ['fid' => $fid, 'attributes' => ['title' => t('View Info')]]
              ) . ' ' . theme('file_action_list',
                ['accession' => $accession, 'fid' => $fid]
              );
          }
        }
      }
      $final_file_row[] = TppsArray::printList($value['file']['grouped_list'][$group]);
    }
  }
  elseif ($context['report_filter'] == Report::MISSING_FILES) {
    if (!empty($value['file'][Report::MISSING_FILES])) {
      $final_file_row[] = theme('item_list',
        ['items' => $value['file'][Report::MISSING_FILES]]
      );
    }
  }
  return array_merge(
    [
      $context['index'],
      theme('submission_accession_link', [
        'accession' => $accession,
        'path' => 'admin/reports/tpps/submission/file/all/' . $accession,
        'show_version' => TRUE,
      ])
        . '<br />['
        . implode(', ', Submission::getPageList(['accession' => $accession]))
        . ']',
      theme('submission_submitter', $value),
      $value['stage'],
      $value['status'],
    ],
    $final_file_row
  );
}
