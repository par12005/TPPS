<?php

/**
 * @file
 * Zenodo API actions for certain TPPS Submission.
 *
 * To get Zenodo API actions for certain Zenodo Depotions use:
 * /api/zenodo/:deposit_id/:action/
 */

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
  module_load_include('inc', 'tpps', 'pages/submission/list');
  tpps_submission_list_validate_accession($accession);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Zenodo Deposition Widget.
  $context = [
    'debug_mode' => FALSE,
    'form' => &$form,
    'accession' => $accession,
    'zenodo_deposit_page_path' => $path . '/' . $accession . '/zenodo',
    'zenodo_files_page_path' => $path . '/' . $accession . '/zenodo/files',
  ];
  $deposition = ZenodoDepositWidget::add($context);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add back button.
  $form['actions'] = $form['actions'] ?? ['#type' => 'container'];
  $action = 'back';
  $form['actions'][$action] = [
    '#type' => 'submit',
    '#name' => $action,
    '#value' => t(ucfirst($action)),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
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
  $widget_name = ZenodoDepositWidget::NAME;
  $widget = $form_state['values'][$widget_name] ?? NULL;
  if (empty($accession = $widget['accession'] ?? '')) {
    form_set_error('accession',
      t('Empty accession. Please choose from the list below.')
    );
  }
  // @TOdO Minor. Check if clicked button is in the list of existing buttons.
}

/**
 * Form submitter for 'tpps_submission_list_form' form.
 */
function tpps_submission_zenodo_form_submit(array &$form, array &$form_state) {
  $action = $form_state['triggering_element']['#name'];
  // 'Back' button.
  if ($action == 'back') {
    $widget_name = ZenodoDepositWidget::NAME;
    $widget = $form_state['values'][$widget_name] ?? NULL;
    if (!empty($accession = $widget['accession'])) {
      // For some reasons messages from export/import pages duplicates when 'Back'
      // button clicked so we simply clear all messages out.
      drupal_get_messages();
      drupal_goto('tpps/submission/' . $accession);
    }
  }
  $form_state['rebuild'] = TRUE;
}
