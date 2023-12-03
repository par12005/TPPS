<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'includes/common');
module_load_include('inc', 'tpps', 'includes/form');
module_load_include('inc', 'tpps', 'admin/submission_list');
module_load_include('inc', 'tpps', 'includes/submissions');

/**
 * Menu callback. Shows a single submission data.
 */
function tpps_submission_view_form(array $form, array $form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);

  if (empty($state = tpps_submission_get_state($accession))) {
    drupal_set_message(t('Submission "<strong>@accession</strong>" not found. '
      . 'Please choose from the list below.',
      ['@accession' => $accession]), 'error'
    );
    drupal_goto('tpps/submission');
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
    tpps_form_add_report($form, [
      'import_metadata' => [
        'title' => t('Import Metadata'),
        'data' => theme('item_list', ['items' => $items]),
      ],
    ]);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $form['tpps_submission_accession_2'] = [
    '#type' => 'select',
    '#title' => t('Accession to compare with'),
    '#options' => tpps_submission_get_accession_list(),
  ];
  $form['action'] = ['#type' => 'container'];
  $form['action']['compare'] = [
    '#type' => 'submit',
    '#value' => t('Compare'),
    '#submit' => ['tpps_submission_view_form_submit'],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show the data!
  $parsed = tpps_array_dump($state);
  $form['submission'] = [
    '#type' => 'fieldset',
    '#title' => t('Submission'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
  ];

  foreach ($parsed as $key => $item) {
    $form['submission'][$key . '_container'] = [
      '#type' => 'fieldset',
      '#title' => ($item['title'] ?? $key)
        . ($item['count'] ? ' (' . $item['count'] . ')' : ''),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      'plain_items' => ['#markup' => ($item['data'] ?? NULL)],
    ];
  }
  //tpps_submission_list_add_buttons($form);
  tpps_form_autofocus($form, 'tpps_submission_import_accession');
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
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
    $path = 'tpps/submission/' . $accession_1 . '/compare/' . $accession_2;
    drupal_goto($path);
  }
}
