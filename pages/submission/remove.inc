<?php

/**
 * @file
 * Removes parts of the TPPS Submission.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. TPPS Submission Remove Form.
 */
function tpps_submission_remove_form(array $form, array &$form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);
  $message = t(
    'Are you sure you want to remove Submission @accession?',
    ['@accession' => $accession]
  );
  $cancel_redirect = 'tpps/submission';
  $description = t("Submission @accession and it's files will be removed.",
      ['@accession' => $accession]
    ) . '<br />' . l(
        t('Submistion @accession view page', ['@accession' => $accession]),
        'tpps/submission/' . $accession . '/view'
      );
  $confirm_button_text = t('Remove');
  $cancel_button_text = t('Cancel');
  // @TODO Get accession from URL (if it's safe).
  $form['tpps_submission_accession'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];

  $form = confirm_form(
    $form,
    $message,
    $cancel_redirect,
    $description,
    $confirm_button_text,
    $cancel_button_text
  );
  $form['#attributes']['class'][] = 'tpps-submission';
  // Removement options.
  // @TODO Check each option and show status.
  $form['options'] = [
    '#type' => 'fieldset',
    '#title' => t('Removement options'),
    '#collapsible' => TRUE,
  ];
  $remove_options = tpps_submission_get_remove_options();
  foreach ($remove_options as $name => $item) {
    $form['options'][$name] = [
      '#type' => 'checkbox',
      '#title' => $item['title'],
      '#default_value' => $item['default_value'],
      '#description' => $item['description'] ?? NULL,
    ];
  }
  tpps_add_css_js('theme', $form);
  $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_remove_form_submit(array $form, array &$form_state) {
  $accession = $form_state['values']['tpps_submission_accession'] ?? NULL;
  tpps_submission_list_validate_accession($accession);
  // Get removement options.
  $options = [];
  $remove_options = tpps_submission_get_remove_options();
  foreach (array_keys($remove_options) as $name) {
    $options[$name] = (bool) $form_state['values'][$name];
  }
  // Remove study.
  tpps_submission_load($accession)->remove($options);
  drupal_goto('tpps/submission/' . $accession . '/remove');
}

/**
 * Gets list of removement options.
 *
 * @return array
 *   Returns array to build form and set default values.
 */
function tpps_submission_get_remove_options() {
  // Options.
  $options = [
    'clearTppsSubmissionTable' => [
      'default_value' => TRUE,
      'title' => t('Remove record in "tpps_submission" table.'),
      'description' => t('Removes records from "public.tpps_submission" and "chado.dbxref" tables.')
    ],
    'clearDB' => [
      'default_value' => TRUE,
      // @TODO Convert into 'checkboxes' and show list of tables which checked
      // by default.
      'title' => t('Clear DB tables.'),
    ],
    'removeFiles' => [
      'default_value' => FALSE,
      'title' => t('Remove files.'),
      'description' => t('Removes only files not used by other studies.'),
    ],
  ];
  return $options;
}
