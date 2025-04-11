<?php

/**
 * @file
 * Zenodo API actions for certain TPPS Submission.
 *
 * To get Zenodo API actions for certain Zenodo Depotions use:
 * /api/zenodo/:deposit_id/:action/
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. Shows Zenodo Action buttons.
 *
 * Path: /tpps/submission/:accession/zenodo.
 */
function tpps_submission_zenodo_form(array $form, array &$form_state = [], $accession = NULL) {
  // Check submission accession.
  $path = 'tpps/submission';
  if (empty($accession)) {
    drupal_set_message(
      t('Empty study accession. Please choose one from the list below.'),
      'error'
    );
    drupal_goto($path);
  }
  tpps_submission_list_validate_accession($accession);
  //$submission = new Submission($accession);

  //// Check if submission exists.
  //$deposit_id = $submission->getZenodoDepositId();

  // Show Zenodo Host on page.
  //if (empty($form_state['triggering_element'])) {
  //  ZenodoDeposit::printHostStatus();
  //}

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Zenodo Deposition Widget.
  $context = [
    'form' => &$form,
    'accession' => $accession,
    'zenodo_page_path' => $path . '/' . $accession . '/zenodo',
    'files_page_path' => $path . '/' . $accession . '/zenodo/files',
    'show_deposition_dump' => TRUE,
    'show_deposition_links' => FALSE,
  ];
  ZenodoDeposit::addWidget($context);

  // Add back button.
  $form['actions'] = $form['actions'] ?? ['#type' => 'container'];
  $action = 'back';
  $form['actions'][$action] = [
    '#type' => 'submit',
    '#name' => $action,
    '#value' => t(ucfirst($action)),
  ];
  $action = 'files';
  $form['actions'][$action] = [
    '#type' => 'submit',
    '#name' => $action,
    '#value' => t('Zenodo Files'),
  ];

  // WARNING: This CSS class adds theme and design.
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  return $form;
}

/**
 * Form validator for 'tpps_submission_list_form' form.
 */
function tpps_submission_zenodo_form_validate(array $form, array &$form_state) {
  // Check accession.
  $zenodo_widget = $form_state['values']['zenodo_deposition'] ?? NULL;
  if (empty($accession = $zenodo_widget['accession'] ?? '')) {
    form_set_error('accession',
      t('Empty accession. Please choose from the list below.')
    );
  }
  // Note: Deposit Id will be validated in form submitter because it requires
  // fully loaded Submission objec (which takes time).
  // @TOdO Minor. Check if clicked button is in the list of existing buttons.
}

/**
 * Form submitter for 'tpps_submission_list_form' form.
 */
function tpps_submission_zenodo_form_submit(array &$form, array &$form_state) {
  $action = $form_state['triggering_element']['#name'];
  $zenodo_widget = $form_state['values']['zenodo_deposition'] ?? NULL;
  if (empty($accession = $zenodo_widget['accession'] ?? '')) {
    drupal_get_messages(
      t('Empty accession. Please choose from the list below.'),
      'error'
    );
  }
  // 'Back' button.
  if ($action == 'back') {
    // For some reasons messages from export/import pages duplicates when 'Back'
    // button clicked so we simply clear all messages out.
    drupal_get_messages();
    drupal_goto('tpps/submission/' . $accession);
  }
  if ($action == 'files') {
    drupal_get_messages();
    drupal_goto('tpps/submission/' . $accession . '/zenodo/files');
  }
  $form_state['rebuild'] = TRUE;
}
