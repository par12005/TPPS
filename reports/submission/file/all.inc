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

// @TODO Update hook_menu.

  $reset = $_GET['reset'] ?? FALSE;
  $data = SubmissionList::get($reset);
  // Redirect to URL without 'reset' query item.
  if (isset($_GET['reset'])) {
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
  $options = ['query' => ['token' => variable_get('tpps_api_token')]];
  $accession = explode('/', $_GET['q'])[6] ?? NULL;
  if (empty($is_single = (strpos($accession, 'TGDR') !== FALSE))) {
    $accession = NULL;
  }
  if ($filter == 'all' || $is_single) {
    $file_column_list = [
      'tree-accession',
      'phenotype',
      'genotype',
      // Group 'other' used for files whith has fid but not belong to any group.
      'other',
    ];
  }
  elseif ($filter == 'missing') {
    $file_column_list = ['missing'];
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
    elseif ($filter == 'missing') {
      $report_data = [];
      foreach ($data as $accession => $cached_submission) {
        if (!empty($cached_submission['file']['missing'])) {
          $report_data[$accession] = $cached_submission;
        }
      }
    }
    elseif ($filter == 'none') {
      $report_data = [];
      foreach ($data as $accession => $cached_submission) {
        if (empty($cached_submission['file']['id_list'])) {
          $report_data[$accession] = $cached_submission;
        }
      }
    }
  }

  $formatted_rows = [];
  // Line counter.
  $i = 0;
  foreach ($report_data as $key => $value) {
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
 * Themes single row for 'submission_file_all' report.
 *
 * @param array $context
 *   Cached Submission's data (item of the SubmissionList).
 *   See SubmissionList::buildSingle() for the structure.
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
    if ($context['is_single'] || $context['report_filter'] == 'all') {
      if (empty($value['file']['grouped_list'][$group])) {
        $final_file_row[] = '';
        continue;
      }
      foreach ($value['file']['grouped_list'][$group] as $organism_label => $field) {
        if (empty($field) || !is_array($field)) {
          continue;
        }
        foreach ($field as $field_name => $fid) {
          if (in_array($fid, $value['file']['missing'])) {
            // File is missing in filesystem.
            $value['file']['grouped_list'][$group][$organism_label][$field_name]
              = check_plain($fid);
          }
          else {
            // File exists.
            $value['file']['grouped_list'][$group][$organism_label][$field_name]
              = tpps_build_file_api_link($accession, $fid, $context['options']);
          }
        }
      }
      $final_file_row[] = tpps_array_print_list($value['file']['grouped_list'][$group]);
    }
    elseif ($context['report_filter'] == 'missing') {
      if (!empty($value['file']['missing'])) {
        $final_file_row[] = theme('item_list',
          ['items' => $value['file']['missing']]
        );
      }
    }
  }
  return array_merge(
    [
      $context['index'],
      l($accession, 'api/submission/' . $accession . '/view/shared_state'),
      $value['submitter']['uid'],
      $value['stage'],
      $value['status'],
    ],
    $final_file_row
  );
}
