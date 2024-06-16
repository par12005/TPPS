<?php

/**
 * @file
 * Functions to manage TPPS Submission's files (not files in common).
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/submission_file');
 */

/**
 * Gets form submission state, prepares files for import.
 *
 * @param array $state
 *   TPPS Submission State which will be downgraded to
 *   Submission Form Version 1 to get File Ids.
 * @param array $form
 *   If not empty then report fieldset will be added to form.
 *   We need $form here to add File Detection Report.
 *
 * @return array
 *   Returns list of File IDs or empty array.
 */
function tpps_submission_file_get_id_list(array $state, array &$form = []) {
  // We use Submission Form Version 1 to avoid updates on each form change.
  $submission = new Submission();
  $submission->state = $state;

  $complete_form = $submission->state['complete form'] ?? NULL;
  $saved_values = $submission->sharedState['saved_values'] ?? NULL;
  // Note: Shared State doesn't have 'values' element but State could have
  // newer version of form and other paths.
  $values = $submission->state['values'] ?? NULL;
  $shared_state_files = $submission->state['files'] ?? [];
  $file_info = $submission->sharedState['file_info'] ?? NULL;

  module_load_include('inc', 'tpps', 'includes/form');
  $organism_number = $submission->getOrganismNumber();
  $result = [];
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Complete Form.
  $method_name = 'complete form';
  $fid_list = [];
  if ($complete_form && $values) {
    $files = tpps_form_search_field($complete_form, 'managed_file');
    foreach ($files as $parents) {
      $fid_list[] = drupal_array_get_nested_value($values, $parents);
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
  $fid_list = array_filter($shared_state_files);
  $report[$method_name] = $fid_list;
  if (!empty($fid_list)) {
    $result = array_merge($result, $fid_list);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Use $form_state['file_info'] if any.
  $method_name = '$form_state["file_info"]';
  $fid_list = [];
  $fid_list = array_merge(
    array_keys($file_info[1] ?? []),
    array_keys($file_info[2] ?? []),
    array_keys($file_info[3] ?? []),
    array_keys($file_info[4] ?? [])
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
  if ($values || $saved_values) {
    $parent_list = tpps_submission_file_get_parents($organism_number);
    if ($values) {
      foreach ($parent_list as $parents) {
        $fid = drupal_array_get_nested_value($values, $parents);
        // Page 4/Genotype/Other (managed file field) has subfields and
        // returns array so we need to skip this to avoid:
        // Warning: array_flip(): Can only flip STRING and INTEGER values! in
        // DrupalDefaultEntityController->load()
        // (line 175 of /var/www/Drupal/includes/entity.inc).
        if (!is_array($fid)) {
          $fid_list[] = $fid;
        }
      }
    }
    if ($saved_values) {
      foreach ($parent_list as $parents) {
        $step = ($parents[0] == 'tree-accession') ? TPPS_PAGE_3 : TPPS_PAGE_4;
        if (!empty($saved_values[$step])) {
          // When form became outdated (6+ hours) and cron removed temporary
          // files then resubmitting form cause errors at site. Probably
          // correct solution is to redirect to submittion edit page
          // Note: Redirect using GET request not yet implemented. Only POST.
          $fid = drupal_array_get_nested_value($saved_values[$step], $parents);
          if (!is_array($fid)) {
            $fid_list[] = $fid;
          }
        }
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Summary page.
    // It seems we have unlimited number of extra files which could be
    // added to study but dump has 10 items.
    for ($i = 1; $i <= 10; $i++) {
      $parent_list[] = ['summarypage', 'files', $i];
    }
    if ($values) {
      foreach ($parent_list as $parents) {
        $fid = drupal_array_get_nested_value($values, $parents);
        // Page 4/Genotype/Other (managed file field) has subfields and
        // returns array so we need to skip this to avoid:
        // Warning: array_flip(): Can only flip STRING and INTEGER values! in
        // DrupalDefaultEntityController->load()
        // (line 175 of /var/www/Drupal/includes/entity.inc).
        if (!is_array($fid)) {
          $fid_list[] = $fid;
        }
      }
    }
    if ($saved_values) {
      foreach ($parent_list as $parents) {
        // When form became outdated (6+ hours) and cron removed temporary
        // files then resubmitting form cause errors at site. Probably
        // correct solution is to redirect to submittion edit page
        // Note: Redirect using GET request not yet implemented. Only POST.
        $fid = drupal_array_get_nested_value($saved_values, $parents);
        if (!is_array($fid)) {
          $fid_list[] = $fid;
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
  // Report.
  $rows = [];
  foreach ($report as $method => $list) {
    $count = count($list);
    $rows[] = [
      check_plain($method),
      ($count ? check_plain($count) : t('Not found')),
      ($count ? '' : check_plain(implode(', ', $list))),
    ];
  }
  if ($rows) {
    tpps_form_add_report($form, [
      'files_detection_methods' => [
        'title' => t('Files Detection Methods'),
        'data' => theme('table', [
          'header' => [t('Method'), t('Status'), t('List')],
          'rows' => $rows,
        ]),
      ],
    ]);
  }

  $result = array_unique($result);
  return $result;
}

/**
 * Changes status of all the submission files.
 *
 * If file has no usage yet it will be added.
 *
 * @param array $state
 *   Drupal Form API State array.
 * @param int $status
 *   File status. 1 = permanent,
 *   0 - temporary. Will be removed by cron in 6h.
 */
function tpps_submission_file_set_status(array $state, $status = FILE_STATUS_PERMANENT) {
  $form_version = tpps_submission_form_version_get($state);
  $fid_list = tpps_submission_file_get_id_list($state);
  foreach ($fid_list as $fid) {
    $file = tpps_file_load($fid);
    if ($file && $file->status != $status) {
      $file->status = $status;
      file_save($file);
      tpps_file_add_usage($file, $state['accession']);
    }
  }
}

/**
 * Gets hardcoded list of file field parents.
 *
 * WARNING:
 * Use Submission Shared State (v.1 not Submission State).
 *
 * @param int $organism_number
 *   Number of species in submission.
 *
 * @return array
 *   Returns empty array if number of organisms is zero.
 *   Returns list of parents
 *   (from  $form_state['values'] or/and $form_state['saved_values']).
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
      [$organism_id, 'phenotype', 'iso'],
      [$organism_id, 'phenotype', 'file'],
      [$organism_id, 'phenotype', 'metadata'],
      // Genotype.
      [$organism_id, 'genotype', 'tripal_fasta', 'file', 'file_upload'],
      // @TODO Check this field:
      //[$organism_id, 'genotype', 'tripal_fasta', 'file_upload_existing'],
      [$organism_id, 'genotype', 'files', 'snps-assay'], // Shared State.
      [$organism_id, 'genotype', 'files', 'assay-design'],
      [$organism_id, 'genotype', 'files', 'snps-association'],
      [$organism_id, 'genotype', 'files', 'snps-pop-struct'],
      [$organism_id, 'genotype', 'files', 'snps-kinship'],
      [$organism_id, 'genotype', 'files', 'ssrs'],
      [$organism_id, 'genotype', 'files', 'ssrs_extra'],
      [$organism_id, 'genotype', 'files', 'indels'],
      [$organism_id, 'genotype', 'files', 'vcf'],

      [$organism_id, 'genotype', 'files', 'other'],
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
