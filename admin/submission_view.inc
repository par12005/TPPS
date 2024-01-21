<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'includes/common');

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
  // @TODO Validate $accession.
  if (empty($state = tpps_submission_get_state($accession))) {
    drupal_set_message(t('Submission "<strong>@accession</strong>" not found. '
      . 'Please choose from the list below.',
      ['@accession' => $accession]), 'error'
    );
    drupal_goto('admin/config/tpps/submission/view');
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $form['tpps_submission_accession_1'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];
  $form['tpps_submission_current_accession'] = [
    '#markup' => '<h3>' . t('Current Accession: @accession',
    ['@accession' => $accession]) . '</h3>',
  ];
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
  // Show data.
  $data = [
    'form_state' => tpps_array_dump($state),
    'interface' => tpps_array_dump(tpps_submission_interface_load($accession)),
  ];
  foreach ($data as $data_type => $array) {
    $form[$data_type . '_container'] = [
      '#type' => 'fieldset',
      '#title' => $data_type,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    ];
    foreach ($array as $key => $item) {
      $form[$data_type . '_container'][$key . '_container'] = [
        '#type' => 'fieldset',
        '#title' => ($item['title'] ?? $key)
          . ($item['count'] ? ' (' . $item['count'] . ')' : ''),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        'plain_items' => ['#markup' => ($item['data'] ?? NULL)],
      ];
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
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
