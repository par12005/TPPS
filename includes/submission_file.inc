<?php

/**
 * @file
 * Functions to manage TPPS Submission's files.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/submission_file');
 */

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/**
 * Gets form submission state, prepares files for import.
 *
 * @param array $state
 *   TPPS Submission State.
 * @param bool $show_report
 *   When TRUE report will be shown using drupal_set_message().
 *
 * @return array
 *   Returns list of File IDs or empty array.
 */
function tpps_submission_file_get_id_list(array $state, $show_report = FALSE) {
  $result = [];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Complete Form.
  $method_name = 'complete form';
  $fid_list = [];
  if (!empty($state['complete form'])) {
    module_load_include('inc', 'tpps', 'includes/form');
    $files = tpps_form_search_field($state['complete form'], 'managed_file');
    foreach ($files as $parents) {
      $fid_list[] = drupal_array_get_nested_value($state['values'], $parents);
    }
  }
  $fid_list = array_filter($fid_list);
  $report[$method_name] = $fid_list;
  if (!empty($fid_list)) {
    $result = array_merge($result, $fid_list);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Use $form_state['files'] if any.
  $method_name = '$form_state["files"]';
  $fid_list = [];
  $fid_list = array_filter($state['files'] ?? []);
  $report[$method_name] = $fid_list;
  if (!empty($fid_list)) {
    $result = array_merge($result, $fid_list);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Use $form_state['file_info'] if any.
  $method_name = '$form_state["file_info"]';
  $fid_list = [];
  $fid_list = array_merge(
    array_keys($state['file_info'][1] ?? []),
    array_keys($state['file_info'][2] ?? []),
    array_keys($state['file_info'][3] ?? []),
    array_keys($state['file_info'][4] ?? [])
  );
  $fid_list = array_filter($fid_list);
  $report[$method_name] = $fid_list;
  if (!empty($fid_list)) {
    $result = array_merge($result, $fid_list);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Last chance. Use hardcoded path to file fields.
  $method_name = 'hardcoded';
  $fid_list = [];
  if (!empty($state['values']) || !empty($state['saved_values'])) {
    module_load_include('inc', 'tpps', 'includes/form');
    $organism_number = tpps_form_get_organism_number($state);
    if (!empty($state['values'])) {
      foreach (tpps_submission_file_get_parents($organism_number) as $parents) {
        $fid_list[] = drupal_array_get_nested_value($state['values'], $parents);
      }
    }
    if (!empty($state['saved_values'])) {
      foreach (tpps_submission_file_get_parents($organism_number) as $parents) {
        $step = ($parents[0] == 'tree-accession') ? 3 : 4;
        if (!empty($state['saved_values'][$step])) {
          $fid_list[] = drupal_array_get_nested_value(
            $state['saved_values'][$step],
            $parents
          );
        }
      }
    }
  }
  $fid_list = array_filter($fid_list ?? []);
  $fid_list = array_unique($fid_list);
  $report[$method_name] = $fid_list;
  if (!empty($fid_list)) {
    $result = array_merge($result, $fid_list);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show Report.
  if ($show_report) {
    drupal_set_message(t("Submisssion's Files detection report"));
    foreach ($report as $method => $list) {
      $count = count($list);
      drupal_set_message(t('Method "@method": @status. @list',
        [
          '@method' => $method,
          '@status' => ((bool) $count
            ? t('Found @count', ['@count' => $count])
            : t('Not found')),
          '@list' => ($count ? 'List: ' . implode(', ', $list) . '.' : ''),
        ]
      ));
    }
  }
  $result = array_unique($result);
  return $result;
}

/**
 * Changes status of all the submission files.
 *
 * If file has no usage yet it will be added.
 *
 * @param array $form_state
 *   Drupal Form API State array.
 * @param int $status
 *   File status. 1 = permanent,
 *   0 - temporary. Will be removed by cron in 6h.
 */
function tpps_submission_file_set_status(array $form_state, $status = FILE_STATUS_PERMANENT) {
  $fid_list = tpps_submission_file_get_id_list($form_state, FALSE);
  foreach ($fid_list as $fid) {
    $file = file_load($fid);
    if ($file && $file->status != $status) {
      $file->status = $status;
      file_save($file);
      tpps_file_add_usage($file, $form_state['accession']);
    }
  }
}

/**
 * Gets hardcoded list of file field parents.
 *
 * @param int $organism_number
 *   Number of species in submission.
 *
 * @return array
 *   Returns empty array if number of organisms is zero.
 *   Returns list of parents (from $form_state['values']);
 */
function tpps_submission_file_get_parents($organism_number = 1) {
  $result = [];

  for ($i = 1; $i <= $organism_number; $i++) {
    $organism_id = 'organism-' . $i;
    $species_id = 'species-' . $i;
    // Page 3. Accession file.
    $result = array_merge($result, [
      ['tree-accession', $species_id, 'file'],
    ]);
    // Page 4. Phenotype and Genotype.
    // @TODO Add environmental files?
    $result = array_merge($result, [
      // Phenotype.
      [$organism_id, 'phenotype', 'metadata'],
      [$organism_id, 'phenotype', 'file'],
      // Genotype.
      [$organism_id, 'genotype', 'tripal_fasta', 'file', 'file_upload'],
      [$organism_id, 'genotype', 'files', 'snps-assay'],
      [$organism_id, 'genotype', 'files', 'assay-design'],
      [$organism_id, 'genotype', 'files', 'snps-association'],
      [$organism_id, 'genotype', 'files', 'snps-pop-struct'],
      [$organism_id, 'genotype', 'files', 'snps-kinship'],
      [$organism_id, 'genotype', 'files', 'ssrs'],
      [$organism_id, 'genotype', 'files', 'ssrs_extra'],
      [$organism_id, 'genotype', 'files', 'other'],
      [$organism_id, 'genotype', 'files', 'vcf'],
    ]);
  }
  // Analysis.
  // @TODO Prepare and check.
  // 'analysis' = [
  //   'diversity_file' => 0,
  //   'population_structure_file' => 0,
  //   'association_genetics_file' => 0,
  //   'landscape_genomics_file' => 0,
  //   'phenotype_environment_file' => 0,
  // ];
  return $result;
}
