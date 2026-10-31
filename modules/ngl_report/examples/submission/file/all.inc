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
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);

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
  else {
    // It's file type.
    switch ($filter) {
      // Genotype files:
      case AssayDesign::DOM_NAME:
      case OtherMarker::DOM_NAME:
      case SnpAssay::DOM_NAME:
      case SnpAssociation::DOM_NAME:
      case SnpsKinship::DOM_NAME:
      case SnpsPopulationStructure::DOM_NAME:
      // Phenotype
      case PhenotypeData::DOM_NAME:
      case PhenotypeIso::DOM_NAME:
      case PhenotypeMeta::DOM_NAME:
        $file_column_list = [
          Report::TREE_ACCESSION_FILES,
          Report::PHENOTYPE_FILES,
          Report::GENOTYPE_FILES,
          // Group Report::OTHER_FILES (which is 'other') will be used for files
          // which has fid but doesn't belong to any group.
          Report::OTHER_FILES,
        ];
        break;
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Format output.
  if ($is_single) {
    $report_data = [$accession => $data[$accession]];
  }
  else {
    krsort($data);
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
    else {
      // It's file type.
      switch ($filter) {
        // Genotype files:
        case AssayDesign::DOM_NAME:
        case OtherMarker::DOM_NAME:
        case SnpAssay::DOM_NAME:
        case SnpAssociation::DOM_NAME:
        case SnpsKinship::DOM_NAME:
        case SnpsPopulationStructure::DOM_NAME:
          // Phenotype.
        case PhenotypeData::DOM_NAME:
        case PhenotypeIso::DOM_NAME:
        case PhenotypeMeta::DOM_NAME:
          $report_data = [];
          foreach (array_keys($data) as $accession) {
            $grouped_list = TppsArray::get($data,
              [$accession, 'file', SubmissionCache::FILE_GROUPED_LIST]
            );
            if (empty($grouped_list)) {
              continue;
            }
            foreach ($grouped_list as $organism_list) {
              foreach ($organism_list as $file_list) {
                if (!in_array($filter, array_keys($file_list))) {
                  continue;
                }
                $report_data[$accession] = $data[$accession];
              }
            }
          }
          break;
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
  $header = array_merge(
    ['#', t('Accession'), t('UID'), t('Stage'), t('Status')],
    $file_column_list
  );
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show report table.
  return Report::print([
    'name' => 'submission_file_all',
    'header' => $header,
    'rows' => $formatted_rows,
    'pager' => 'both',
    'submission_cache' => TRUE,
  ], $reset);
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
  $filter = ($context['is_single']) ? 'all' : $context['report_filter'];

  $total_filesize = 0;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  switch ($filter) {
    case Report::MISSING_FILES:
      if (!empty($value['file'][Report::MISSING_FILES])) {
        $final_file_row[] = theme('item_list',
          ['items' => $value['file'][Report::MISSING_FILES]]
        );
      }
      break;

    case 'all':
      // Genotype files.
    case AssayDesign::DOM_NAME:
    case OtherMarker::DOM_NAME:
    case SnpAssay::DOM_NAME:
    case SnpAssociation::DOM_NAME:
    case SnpsKinship::DOM_NAME:
    case SnpsPopulationStructure::DOM_NAME:
      // Phenotype.
    case PhenotypeData::DOM_NAME:
    case PhenotypeIso::DOM_NAME:
    case PhenotypeMeta::DOM_NAME:
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
            $file_size = $value['file']['id_list'][$fid] ?? 0;
            $total_filesize += $file_size;
            if (in_array($fid, $value['file'][Report::MISSING_FILES])) {
              // File is missing in filesystem.
              $value['file']['grouped_list'][$group][$organism_label][$field_name]
                = check_plain($fid);
            }
            else {
              // File exists.
              if (is_numeric($fid)) {
                $file_label = theme('file_id',
                  ['fid' => $fid, 'attributes' => ['title' => t('View Info')]]
                );
                $value['file']['grouped_list'][$group][$organism_label][$field_name]
                  = $file_label
                  . ' ' . theme('file_size', ['size' => $file_size])
                  . ' ' . theme('file_action_list',
                    ['accession' => $accession, 'fid' => $fid]
                  );
              }
              else {
                $file_label = theme('file_local_vcf',
                  ['accession' => $accession, 'path' => $fid]
                );
              }
              $value['file']['grouped_list'][$group][$organism_label][$field_name]
                = $file_label . ' ' . theme('file_size', ['size' => $file_size]);
              // Actions only for managed files (not for local VCF files).
              if (is_numeric($fid)) {
                $value['file']['grouped_list'][$group][$organism_label][$field_name]
                  .= ' ' . theme('file_action_list',
                    ['accession' => $accession, 'fid' => $fid]
                  );
              }
            }
          }
        }
        $final_file_row[] = TppsArray::print($value['file']['grouped_list'][$group]);
      }
      break;
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  return array_merge(
    [
      $context['index'],
      theme('submission_accession_link', [
        'accession' => $accession,
        'path' => 'admin/reports/tpps/submission/file/all/' . $accession,
        'show_version' => TRUE,
        'show_load_time' => TRUE,
      ])
      . ' ' . theme('file_size', ['size' => $total_filesize])
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
