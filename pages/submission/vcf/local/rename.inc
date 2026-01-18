<?php

/**
 * @file
 * TPPS Submission Rename local VCF file.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Builds form "TPPS Submission Rename local VCF file".
 */
function tpps_submission_local_vcf_rename_form(array $form, array &$form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);
  if (empty($accession)) {
    $message = t("Empty accession.");
    drupal_set_message($message, 'error');
    drupal_goto('tpps/submission');
  }
  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    $message = t("Submission @accession doesn't exists.",
      ['@accession' => $accession]
    );
    drupal_set_message($message, 'error');

    drupal_goto('tpps/submission');
  }
  $form['tpps_submission_accession'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];
  $exists = FALSE;
  $organism_number = $submission->getOrganismNumber();
  for ($index = 1; $index <= $organism_number; $index++) {
    $organism_key = 'organism-' . $index;
    $parents = tpps_submission_local_vcf_get_parents(
      $submission,
      $index,
      'local_vcf'
    );
    $form[$organism_key] = [
      '#type' => 'fieldset',
      '#title' => t('Organism @index', ['@index' => $index]),
      '#tree' => TRUE,
    ];
    $file_path = $submission->get($parents);
    if (!empty($file_path)) {
      $exists = TRUE;
    }
    $repeat_check = $submission->get([
      'saved_values',
      TPPS_PAGE_4,
      $organism_key,
      'genotype-repeat-check',
    ]);
    if ($repeat_check) {
      $form[$organism_key]['notice'] = [
        '#markup' => '<p>' . t('Note: file the same as for organism 1.') . '</p>',
      ];
    }
    else {
      $form[$organism_key]['path'] = [
        '#type' => 'textfield',
        '#title' => t('Local VCF File Path'),
        '#default_value' => $file_path,
      ];
    }
  }
  if (!$exists) {
    $message = t("Submission @accession doesn't have local VCF files.",
      ['@accession' => $accession]
    );
    drupal_set_message($message, 'error');
    drupal_goto('tpps/submission');
  }
  // Buttons.
  $form['actions'] = ['#type' => 'container'];
  $buttons = ['update', 'back'];
  foreach ($buttons as $key) {
    $form['actions'][$key] = [
      '#type' => 'submit',
      '#name' => $key,
      '#value' => t(ucfirst($key)),
    ];
  }
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  return $form;
}

/**
 * Form submitter for 'submission_local_vcf_rename_form' form.
 */
function tpps_submission_local_vcf_rename_form_submit(array $form, array &$form_state) {
  $action = $form_state['triggering_element']['#name'] ?? NULL;
  $path = 'tpps/submission';
  $accession = $form_state['values']['tpps_submission_accession'] ?? '';
  if (empty($accession) || !in_array($action, ['back', 'update'])) {
    drupal_goto($path);
  }
  if ($action == 'back') {
    drupal_goto($path . '/' . $accession);
  }
  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    return;
  }
  // Get fieldset according to Form version.
  $organism_number = $submission->getOrganismNumber();
  $updated = FALSE;
  for ($index = 1; $index <= $organism_number; $index++) {
    $organism_key = 'organism-' . $i;
    $file_path = $form_state['values'][$organism_key]['path'] ?? NULL;
    $file_path = trim($file_path);
    if (is_null($file_path)) {
      continue;
    }
    $parents = tpps_submission_local_vcf_get_parents(
      $submission,
      $index,
      'local_vcf'
    );
    $existing_file_path = $submission->get($parents);
    if ($existing_file_path != $file_path) {
      // Update state.
      $submission->set($parents, $file_path);
      // Update Shared State.
      $parents = tpps_submission_local_vcf_get_parents(
        $submission,
        $index,
        'local_vcf',
        $form_version = 1
      );
      TppsArray::set($submission->sharedState, $parents, $file_path);
      $updated = TRUE;
    }
  }
  if ($updated) {
    $message = t("Submission @accession was updated.",
      ['@accession' => $accession]
    );
    $submission->save();
  }
  else {
    $message = t("No changes detected and submission @accession was not updated.",
      ['@accession' => $accession]
    );
  }
  drupal_set_message($message);
}

/**
 * Gets parents for given field.
 *
 * WARNING: Currently works for 'local_vcf' field and supports 1 and 2 versions.
 *
 * @param object $submission
 *   TPPS Submission instance.
 * @param int $organism_index
 *   Ordinal number of organism on form.
 * @param string $field_name
 *   Field machine name.
 * @param int $version
 *   Optional. Default is current Submission Form Version.
 *   Desired version.
 *
 * @return array
 *   Returns list of parents accorging to Form Version.
 */
function tpps_submission_local_vcf_get_parents($submission, $organism_index, $field_name, $version = NULL) {
  $version = $version ?? $submission->getFormVersion();
  $organism_key = 'organism-' . $organism_index;
  if ($field_name == 'local_vcf') {
    switch ($version) {
      case 1:
        // live.
        $parent_fieldset = 'files';
        break;

      default:
      case 2:
        // @TODO Use latest version instead of hardcoded '2'.
        // dev.
        $parent_fieldset = 'SNPs';
        break;
    }
    $parents = [
      'saved_values',
      TPPS_PAGE_4,
      $organism_key,
      'genotype',
      $parent_fieldset,
      $field_name,
    ];
    return $parents;
  }
}
