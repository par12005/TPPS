<?php

/**
 * @file
 * Zenodo API actions for TPPS Submission.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. Shows Zenodo API actions for Submission.
 */
function tpps_submission_zenodo_form(array $form, array $form_state, $accession = NULL, $group_name = NULL, $action = NULL) {
  // Check submission accession.
  $path = 'tpps/submission';
  if (empty($accession)) {
    drupal_set_message(t('Empty acession. Please choose one from the list below.'), 'error');
    drupal_goto($path);
  }
  tpps_submission_list_validate_accession($accession);
  // Check action and group.
  if (empty($action) || empty($group_name)) {
    drupal_set_message(t('Empty action. Please click button below.'), 'error');
    drupal_goto($path . '/' . $accession);
  }

  $submission = new Submission($accession);
  $deposit_id = $submission->get(['zenodo_id']);
  if (empty($deposit_id)) {
    if (
      ($group_name == 'deposition' && !in_array($action, ['create', 'list']))
      || ($group_name == 'deposition_files')
    ) {
      drupal_set_message(t('Submission has no Zenodo Deposit Id.'), 'error');
      drupal_goto($path . '/' . $accession);
    }
  }
  $data['deposit_id'] = $deposit_id;

  switch ($group_name) {
    case 'deposition':
    case 'deposition_actions':
      $class = 'ZenodoDeposit';
      break;

    case 'deposition_files':
      $class = 'ZenodoFiles';
      break;
  }

  // Validate $action.
  $method_name = $class . '::' . $action;
  if (!is_callable($method_name, TRUE)) {
    drupal_set_message(t('Attempt to call non-existing Zenodo API action: @name.',
      ['@name' => $method_name]), 'error'
    );
    drupal_goto($path . '/' . $accession);
  }

  // Get response.
  $response = call_user_func($method_name, $data);
  $form['zenodo_api_response'] = [
    '#markup' => ZenodoDeposit::printDeposition($response),
  ];

  tpps_submission_list_add_buttons($form, FALSE);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  return $form;
}
