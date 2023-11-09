<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'includes/common');
//module_load_include('inc', 'tpps', 'includes/submissions');
//module_load_include('inc', 'tpps', 'includes/submission_file');

/**
 * Menu callback. TPPS Study View Form.
 */
function tpps_submission_view_select_form(array $form, array &$form_state) {
  $accession = (($form_state['input']['op'] ?? NULL) == t('Reset')
    ? '' : ($form_state['input']['tpps_submission_accession'] ?? '')
  );
  $form['tpps_submission_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => tpps_submission_get_accession_list(),
    '#default_value' => $accession,
    '#required' => TRUE,
  ];
  $form['action_view'] = [
    '#type' => 'submit',
    '#value' => t('View'),
    '#submit' => ['tpps_submission_view_select_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_submission_view_select_form_reset'],
  ];
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_submission_view_select_form_reset() {
  $form_state['input']['tpps_submission_view_accession'] = '';
  $form_state['values']['tpps_submission_view_accession'] = '';
  drupal_goto('admin/config/tpps/submission/view');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_view_select_form_submit(array $form, array &$form_state) {
  $accession = ($form_state['input']['tpps_submission_accession'] ?? '');
  drupal_goto('admin/config/tpps/submission/view/' . $accession);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/**
 * Menu callback. Single submission view page.
 */
function tpps_submission_view_form(array $form, array $form_state) {
  $accession = arg(5);
  if (empty($accession)) {
    drupal_set_message('Empty accession.', 'error');
    drupal_goto('admin/config/tpps/submission/view');
  }

  module_load_include('inc', 'tpps', 'includes/submissions');
  if (empty($state = tpps_submission_get_state($accession))) {
    drupal_set_message('No submission found.', 'error');
    drupal_goto('admin/config/tpps/submission/view');
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $form['tpps_submission_accession_1'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];
  $form['tpps_submission_accession_2'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => tpps_submission_get_accession_list(),
  ];
  $form['header_back'] = [
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => ['tpps_submission_view_select_form_reset'],
  ];
  $form['action_compare'] = [
    '#type' => 'submit',
    '#value' => t('Compare'),
    '#submit' => ['tpps_submission_view_form_submit'],
  ];
  $parsed = tpps_array_dump($state);
  foreach ($parsed as $key => $item) {
    $form[$key . '_container'] = [
      '#type' => 'fieldset',
      '#title' => ($item['title'] ?? $key)
        . ($item['count'] ? ' (' . $item['count'] . ')' : ''),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      'plain_items' => ['#markup' => ($item['data'] ?? NULL)],
    ];
  }
  $form['footer_back'] = [
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => ['tpps_submission_view_select_form_reset'],
  ];
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_view_form_submit(array $form, array &$form_state) {
  $accession_1 = ($form_state['values']['tpps_submission_accession_1'] ?? '');
  $accession_2 = ($form_state['values']['tpps_submission_accession_2'] ?? '');
  drupal_goto(
    'admin/config/tpps/submission/compare/'
    . $accession_1 . '/' . $accession_2
  );
}
