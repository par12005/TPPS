<?php

/**
 * @file
 * TPPS Study Export Form and related functions.
 *
 *
 * @TODO Minor. Get status of study in
 * https://tgwebdev.cam.uchc.edu/admin/reports/db/public.tripal_jobs.
 * @TODO Check $form_state['saved_values'] for files but it's structure differs
 * from $form_state['value'] and splitted by pages. But some studies at
 * dev-server (maybe broken) has no 'values' but only 'saves_values' but in
 * reviewed studies files was detected correclty using 'comolete form'.
 */

/**
 * TPPS Study Import Form.
 */
function tpps_study_export_form(array $form, array &$form_state) {
  $accession = (($form_state['input']['op'] ?? NULL) == t('Reset')
    ? '' : ($form_state['input']['tpps_export_accession'] ?? '')
  );
  $form['tpps_export_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => tpps_submission_get_accession_list(),
    '#default_value' => $accession,
    '#required' => TRUE,
  ];
  $form['tpps_export_show_file_search_report'] = [
    '#type' => 'checkbox',
    '#title' => 'Show File Search Report?',
    '#default_value' => variable_get('tpps_export_show_file_search_report', FALSE),
  ];
  if (!empty($accession)) {
    $state = tpps_study_export_get_state($accession);
    $form['tpps_export_state'] = [
      '#type' => 'textarea',
      '#title' => t('State PHP Array'),
      '#value' => var_export($state, 1),
      '#rows' => 10,
      '#resizable' => TRUE,
      '#attributes' => ['readonly' => 'readonly'],
    ];
  }
  $form['action_export'] = [
    '#type' => 'submit',
    '#value' => t('Export'),
    '#submit' => ['tpps_study_export_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_study_export_form_reset'],
  ];
  tpps_add_css_js($form, 'study_export');
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_study_export_form_reset() {
  $form_state['input']['tpps_export_accession'] = '';
  $form_state['values']['tpps_export_accession'] = '';
  drupal_goto('admin/config/tpps/migration/export');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_study_export_form_submit(array $form, array &$form_state) {
  variable_set('tpps_export_show_file_search_report',
    ($form_state['values']['tpps_export_show_file_search_report'] ?? FALSE)
  );
  $form_state['rebuild'] = 1;
}

/**
 * Gets form submission state, prepares files for import.
 *
 * @param string $accession
 *   Study submission. Format: 'TGDRxxxx'.
 *
 * @return array
 *   Returns Drupal Form API State array with extra 'tpps_export_files'
 *   element which contains files information necessary for re-import on
 *   other server.
 */
function tpps_study_export_get_state($accession) {
  // Get Submission State.
  $result = db_select('tpps_submission', 'ts')
    ->fields('ts', ['submission_state'])
    ->condition('accession', $accession)
    ->range(0, 1)
    ->execute()
    ->fetchField();
  $state = unserialize(html_entity_decode($result));
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Process files.
  // File URL will be added to $form_state to allow to download files on
  // other server and resubmit study with files.
  //
  $state['tpps_export_fid'] = [];
  // Complete Form.
  $method_name = 'complete form';
  if (!empty($state['complete form'])) {
    $files = tpps_form_search_field($state['complete form'], 'managed_file');
    //dpm(var_export($files, 1));
    foreach ($files as $parents) {
      $fid_list[] = drupal_array_get_nested_value($state['values'], $parents);
    }
  }
  $fid_list = array_filter($fid_list ?? []);
  $methods[$method_name] = $fid_list;
  if (!empty($fid_list)) {
    $state['tpps_export_fid'] = array_merge($state['tpps_export_fid'], $fid_list);
  }

  // Use $form_state['files'] if any.
  $method_name = 'state files';
  $fid_list = array_filter($state['files'] ?? []);
  $methods[$method_name] = $fid_list;
  if (!empty($fid_list)) {
    $state['tpps_export_fid'] = array_merge($state['tpps_export_fid'], $fid_list);
  }

  // Last chance. Use hardcoded path to file fields.
  $method_name = 'hardcoded';
  if (!empty($state['values'])) {
    module_load_include('inc', 'tpps', 'includes/form');
    $organism_number = tpps_form_get_organism_number($state);
    foreach (tpps_study_export_get_file_parents($organism_number) as $parents) {
      $fid_list[] = drupal_array_get_nested_value($state['values'], $parents);
    }
  }
  $fid_list = array_filter($fid_list ?? []);
  $methods[$method_name] = $fid_list;
  if (!empty($fid_list)) {
    $state['tpps_export_fid'] = array_merge($state['tpps_export_fid'], $fid_list);
  }
  $state['tpps_export_fid'] = array_unique($state['tpps_export_fid']);
  if (variable_get('tpps_export_show_file_search_report', FALSE)) {
    tpps_study_export_show_method_report($methods);
  }

  // Add file's metadata and URL.
  if (!empty($state['tpps_export_fid'])) {
    tpps_study_export_get_files_info($state);
    if (empty($state['tpps_export_files'])) {
      drupal_set_message(
        t('Study has files but they not found at server. '
        . 'Probably those files must be uploaded again.'), 'warning'
      );
      drupal_set_message(t('List of File IDs: <pre>@list</pre>',
        ['@list' => print_r($state['tpps_export_fid'], 1)]
      ));
    }
    else {
      drupal_set_message(t('Files was found and processed.'));
      if (variable_get('tpps_export_show_file_search_report', FALSE)) {
        drupal_set_message(t(
          'List of files prepared for import: <pre>@list</pre>',
          ['@list' => print_r($state['tpps_export_files'], 1)]
        ));
      }
    }
  }
  else {
    drupal_set_message(
      t('No files found in this study. Please double-check.'),
      'warning'
    );
  }
  unset($state['tpps_export_fid']);
  return $state;
}

/**
 * Adds files metadata to the Submission state array.
 *
 * @param array $state
 *   Drupal Form API State Array.
 */
function tpps_study_export_get_files_info(array &$state) {
  foreach ($state['tpps_export_fid'] as $fid) {
    if ($fid) {
      $file = file_load($fid);
      if ($file) {
        $file->url = file_create_url($file->uri);
        $state['tpps_export_files'][$fid] = (array) $file;
      }
    }
  }
}

/**
 * Searches for a field of given type in array.
 *
 * @param array $array
 *   Drupal Form API array.
 *   This could be a $form or $form_state['complete form']
 *   or any other form array.
 * @param string $field_type
 *   Type of Form API field to be found in given $array.
 *
 * @return array
 *   Returns list of parents to element of requested type.
 *   Returns empty array if no elements found.
 *   Each item of array is a list of parents which could be used by
 *   drupal_array_get_nested_value() and similar.
 *
 *   Example of returned array:
 *   Array
 *   (
 *        [0] => Array
 *            (
 *                [0] => organism-1
 *                [1] => genotype
 *                [2] => tripal_fasta
 *                [3] => file
 *                [4] => file_upload
 *            )
 *
 *         [1] => Array
 *             (
 *                [0] => organism-1
 *                [1] => genotype
 *                [2] => files
 *                [3] => snps-assay
 *           )
 *     ....
 *   );
 */
function tpps_form_search_field(array $array, $field_type = 'managed_file') {
  $result = [];
  if (empty($field_type)) {
    return $result;
  }
  foreach ($array as $key => $value) {
    $type = ($value['#type'] ?? NULL);
    if (substr($key, 0, 1) == '#' || empty($type)) {
      // Elements with leading '#' in name are for internal use Drupal Form API.
      continue;
    }
    if (in_array($type, ['fieldset', 'container'])) {
      foreach (call_user_func(__FUNCTION__, $value, $field_type) as $item) {
        $result[] = array_merge([$key], $item);
      }
    }
    // Check if element is 'file'.
    elseif ($type == $field_type) {
      $result[] = [$key];
    }
  }
  return $result;
}

/**
 * Gets hardcoded list of file field parents.
 *
 * @param int $organism_number
 *   Number of species in study.
 *
 * @return array
 *   Returns empty array if number of organisms is zero.
 *   Returns list of parents (from $form_state['values']);
 */
function tpps_study_export_get_file_parents($organism_number = 1) {
  $result = [];

  for ($i = 1; $i <= $organism_number; $i++) {
    $organism_id = 'organism-' . $i;
    $species_id = 'species-' . $i;
    // Page 3. Accession file.
    $result = array_merge($result, [
      ['tree-accession', $species_id, 'file']
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

/**
 * Shows status of search for each method.
 *
 * @param array $report
 *   List of methods and their result. TRUE - success and FALSE - fail.
 */
function tpps_study_export_show_method_report(array $report) {
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
