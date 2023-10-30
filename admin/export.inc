<?php

/**
 * @file
 * TPPS Study Import Form and related functions.
 *
 *
 * @TODO Minor. Get status of study in
 * https://tgwebdev.cam.uchc.edu/admin/reports/db/public.tripal_jobs.
 */

/**
 * TPPS Study Import Form.
 */
function tpps_study_export_form(array $form, array &$form_state) {
  $accession = $form_state['input']['tpps_export_accession'] ?? '';
  $form['tpps_export_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => tpps_submission_get_accession_list(),
    '#default_value' => $accession,
    '#required' => TRUE,
  ];
  if (!empty($accession)) {
    $state = tpps_study_export_get_state($accession);
    $form['tpps_export_state'] = [
      '#type' => 'textarea',
      '#title' => t('State PHP Array'),
      '#value' => var_export($state, 1),
      '#rows' => 10,
      '#resizable' => TRUE,
    ];
  }
  // @TODO Minor. Just print function's code:
  // https://stackoverflow.com/questions/7026690
  if (0) {
    $form['tpps_export_code'] = [
      '#type' => 'textarea',
      '#title' => t('PHP Code to get data on remote server'),
      '#value' => "\$accession = '$accession';"
        . "\ndpm(var_export(tpps_study_export_get_state(\$accession), 1));",
      '#rows' => 10,
      '#resizable' => TRUE,
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
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_study_export_form_reset() {
  drupal_goto('admin/config/tpps/migration/export');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_study_export_form_submit(array $form, array &$form_state) {
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
  // Process files.
  // File URL will be added to $form_state to allow to download files on
  // other server and resubmit study with files.
  $files = tpps_form_search_field($state['complete form'], 'managed_file');
  foreach ($files as $parents) {
    $fid = drupal_array_get_nested_value($state['values'], $parents);
    if ($fid && $file = file_load($fid)) {
      $file->url = file_create_url($file->uri);
      $state['tpps_export_files'][$fid] = (array) $file;
    }
  }
  return $state;
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
