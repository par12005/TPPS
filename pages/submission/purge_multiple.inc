<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'includes/common');
module_load_include('inc', 'tpps', 'pages/submission/list');
module_load_include('inc', 'tpps', 'includes/form');

/**
 * Menu callback. TPPS Study View Form.
 */
function tpps_submission_purge_multiple_form(array $form, array &$form_state) {
  $accession_list = tpps_submission_get_accession_list();
  $form['tpps_submission_accession'] = [
    '#type' => 'checkboxes',
    '#title' => t('Accessions'),
    '#options' => $accession_list,
    '#required' => TRUE,
  ];

  $form['actions'] = ['#type' => 'container'];
  $buttons = ['back', 'purge'];
  foreach ($buttons as $key) {
    $form['actions'][$key] = [
      '#type' => 'submit',
      '#name' => $key,
      '#value' => t(ucfirst($key)),
    ];
  }
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_purge_multiple_form_submit(array $form, array &$form_state) {
  $accession_list = array_filter(
    array_values($form_state['input']['tpps_submission_accession'] ?? [])
  );
  if (!empty($accession_list)) {
    foreach ($accession_list as $accession) {
      tpps_submission_purge($accession, $remove_files = TRUE);
      drupal_set_message(
        t("Submission @accession and it's files was removed and cache was cleared.",
          ['@accession' => $accession]
        )
      );
    }
    cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
  }
  else {
    drupal_set_message(t('No accession was selected.'));
  }
  //drupal_goto('tpps/submission/');
}
