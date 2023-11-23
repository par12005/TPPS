<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'includes/common');

/**
 * Menu callback. TPPS Study View Form.
 *
 * Shows dropdown list of existing studies to be selected for view.
 *
 * @TODO Convert to use '/tpps/submission' to show list of studies with
 * action-buttons (View, Compare and etc.).
 * Use /tpps/submission/%/[operation].
 * @TODO Move each page to separate file.
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
 * Menu callback. Shows a single submission data.
 */
function tpps_submission_view_form(array $form, array $form_state) {
  $accession = arg(5);
  if (empty($accession)) {
    drupal_set_message('Empty accession.', 'error');
    drupal_goto('admin/config/tpps/submission/view');
  }

  module_load_include('inc', 'tpps', 'includes/submissions');
  // @TODO Validate $accession.
  if (empty($state = tpps_submission_get_state($accession))) {
    drupal_set_message(t('Submission "<strong>@accession</strong>" not found. '
      . 'Please choose from the list below.',
      ['@accession' => $accession]), 'error'
    );
    drupal_goto('admin/config/tpps/submission/view');
  }
  $form['tpps_submission_current_accession'] = [
    '#markup' => '<h3>' . t('Current Accession: @accession',
    ['@accession' => $accession]) . '</h3>',
  ];
  $form['tpps_submission_accession_1'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Import data.
  $import_meta = $state['saved_values']['tpps_submission_import'] ?? NULL;
  if ($import_meta) {
    $items = [
      t(
        // @TODO Use link.
        'Original Accession: @accession',
        ['@accession' => ($import_meta['original_accession'] ?? NULL)]
      ),
      t(
        'Import Date: @date.',
        ['@date' => ($import_meta['imported_date'] ?? NULL)]
      ),
      t(
        'Import Timestamp: @timestamp.',
        ['@timestamp' => ($import_meta['imported'] ?? NULL)]
      ),
    ];
    $form['original_accession'] = [
      '#markup' => theme('item_list', [
        'title' => t('Import Metadata'),
        'items' => $items,
      ]),
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $form['header_back'] = [
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => ['tpps_submission_view_select_form_reset'],
  ];
  $form['action_purge'] = [
    '#type' => 'submit',
    '#value' => t('Purge'),
    '#submit' => ['tpps_submission_view_form_submit'],
  ];
  $form['tpps_submission_accession_2'] = [
    '#type' => 'select',
    '#title' => t('Accession to compare with'),
    '#options' => tpps_submission_get_accession_list(),
  ];
  $form['action_compare'] = [
    '#type' => 'submit',
    '#value' => t('Compare'),
    '#submit' => ['tpps_submission_view_form_submit'],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show the data!
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
  $button = $form_state['triggering_element']['#value'];
  $accession_1 = ($form_state['values']['tpps_submission_accession_1'] ?? '');
  if ($button == t('Compare')) {
    $accession_2 = ($form_state['values']['tpps_submission_accession_2'] ?? '');
    drupal_goto(
      'admin/config/tpps/submission/compare/'
      . $accession_1 . '/' . $accession_2
    );
  }
  elseif ($button == t('Purge')) {
    drupal_goto('admin/config/tpps/submission/purge/' . $accession_1);
  }
}
