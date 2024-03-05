<?php

/**
 * @file
 * Functions to manage TPPS Submission's files.
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
  $submission_interface = $state;
  tpps_submission_form_version_downgrade_to_latest($state, $submission_interface);

  $complete_form = $submission_interface['complete form'] ?? NULL;
  $saved_values = $submission_interface['saved_values'] ?? NULL;
  $values = $submission_interface['values'] ?? NULL;
  $interface_files = $submission_interface['files'] ?? [];
  $file_info = $submission_interface['file_info'] ?? NULL;

  module_load_include('inc', 'tpps', 'includes/form');
  $organism_number = tpps_form_get_organism_number($submission_interface);
  $result = [];
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Complete Form.
  $method_name = 'complete form';
  $fid_list = [];
  if ($complete_form) {
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
  $fid_list = array_filter($interface_files);
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
    if ($values) {
      foreach (tpps_submission_file_get_parents($organism_number) as $parents) {
        $fid_list[] = drupal_array_get_nested_value($values, $parents);
      }
    }
    if ($saved_values) {
      foreach (tpps_submission_file_get_parents($organism_number) as $parents) {
        $step = ($parents[0] == 'tree-accession') ? TPPS_PAGE_3 : TPPS_PAGE_4;
        if (!empty($saved_values[$step])) {
          // When form became outdated (6+ hours) and cron removed temporary
          // files then resubmitting form cause errors at site. Probably
          // correct solution is to redirect to submittion edit page
          // Note: Redirect using GET request not yet implemented. Only POST.
          $fid_list[] = drupal_array_get_nested_value($saved_values[$step], $parents);
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
      check_plain(implode(', ', $list)),
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
    $file = file_load($fid ?? '');
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
 * Use Submission Form Version 1 (= Submission Interface).
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
      [$organism_id, 'phenotype', 'iso'],
      [$organism_id, 'phenotype', 'file'],
      [$organism_id, 'phenotype', 'metadata'],
      // Genotype.
      [$organism_id, 'genotype', 'tripal_fasta', 'file', 'file_upload'],
      // @TODO Check this field:
      //[$organism_id, 'genotype', 'tripal_fasta', 'file_upload_existing'],
      [$organism_id, 'genotype', 'files', 'snps-assay'],
      [$organism_id, 'genotype', 'files', 'assay-design'],
      [$organism_id, 'genotype', 'files', 'snps-association'],
      [$organism_id, 'genotype', 'files', 'snps-pop-struct'],
      [$organism_id, 'genotype', 'files', 'snps-kinship'],
      [$organism_id, 'genotype', 'files', 'ssrs'],
      [$organism_id, 'genotype', 'files', 'ssrs_extra'],
      [$organism_id, 'genotype', 'files', 'indels'],
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

/**
 * Generates File Secure Token for Managed Drupal File.
 *
 * @param mixed $file
 *   Managed Drupal File.
 *   We need to use object insted of File Id because on Submission Import we
 *   have only array from remote server and couldn't get it by File Id.
 *
 * @return string
 *   Returns a base-64 encoded sha-256 hmac, with + replaced with -,
 *   / with _ and any = padding characters removed.
 */
function tpps_submission_file_token_get($file) {
  $file = (array) $file;
  if (empty($file)) {
    $message = "Can't generate File Secure Token for empty file.";
    tpps_message($message);
    return '';
  }
  unset($file['timestamp']);
  unset($file['url']);
  unset($file['status']);
  $data = json_encode($file);
  $key = variable_get('tpps_submission_file_salt', 'yFA5HRQ5k4AWbsBd');
  return drupal_hmac_base64($data, $key);
}

/**
 * Gets file content from remote server.
 *
 * @param string $url
 *   File URL.
 *
 * @return string
 *   Returns file content.
 */
function tpps_file_get_contents($url) {
  $staging_domain = 'https://tgwebdev.cam.uchc.edu';
  $username = variable_get('tpps_staging_http_username', '');
  $password = variable_get('tpps_staging_http_password', '');

  // @TODO Minor. Replace file_get_contents() with curl which is faster.
  $use_include_path = FALSE;
  $context = NULL;
  // Staging server has Basic HTTP Authorization.
  if (strpos($url, $staging_domain) === 0 && $username && $password) {
    $auth = base64_encode($username . ':' . $password);
    $context = stream_context_create(
      ['http' => ['header' => 'Authorization: Basic ' . $auth]]
    );
  }
  $content = file_get_contents($url, $use_include_path, $context);
  return $content;
}
