<?php

/**
 * @file
 * List of TPPS Submission with action buttons.
 *
 * @TODO Minor. Show submission list as table?
 */

module_load_include('inc', 'tpps', 'includes/common');

/**
 * TPPS Study Export Form.
 */
function tpps_submission_list_form(array $form, array &$form_state = [], $accession = NULL) {
  // Accession.
  $accession_list = SubmissionCache::getAccessionList();
  $form['tpps_submission_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => array_merge([
      'new' => t(
        'New (@next_accession)',
        ['@next_accession' => 'TGDR' . tpps_init_project_get_next_accession()]
      ),
    ], $accession_list),
    '#default_value' => $accession ?? array_key_first($accession_list) ?? '',
    '#required' => TRUE,
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $action_list = [
    'create', 'view', 'export', 'import',
    'compare', 'remove', 'purge', 'zenodo', 'edit', 'clone'
  ];
  foreach ($action_list as $action) {
    $form[$action] = [
      '#type' => 'submit',
      '#value' => $action,
    ];
    // Hide buttons when 'new' accession selected.
    if ($action != 'import') {
      $state = '!value';
      if ($action == 'create') {
        $state = 'value';
      }
      $form[$action]['#states'] = [
        'visible' => [
          ':input[name="tpps_submission_accession"]' => [$state => 'new'],
        ],
      ];
    }
  }
  TppsForm::autofocus($form, ['tpps_submission_accession']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  return $form;
}

/**
 * Form validator for 'tpps_submission_list_form' form.
 */
function tpps_submission_list_form_validate(array $form, array &$form_state) {
  // Check accession.
  $accession = $form_state['values']['tpps_submission_accession'] ?? '';
  if (empty($accession)) {
    form_set_error('tpps_submission_accession',
      t('Empty accession. Please choose from the list below.')
    );
  }
  // Check action and group.
  $action = $form_state['triggering_element']['#value'] ?? NULL;
  if (empty($action)) {
    form_set_error('non-existing_form_element', t('Empty action.'));
  }
}

/**
 * Form submitter for 'tpps_submission_list_form' form.
 */
function tpps_submission_list_form_submit(array $form, array &$form_state) {
  $accession = $form_state['values']['tpps_submission_accession'] ?? '';
  $action = $form_state['triggering_element']['#value'] ?? NULL;
  $path = 'tpps/submission';
  if ($accession == 'new' && $action == 'create') {
    drupal_goto('tppsc');
  }
  elseif ($action == 'edit' && !empty($accession)) {
    drupal_goto('tppsc/' . $accession);
  }
  else {
    $path .= ($accession && $action) ? '/' . $accession . '/' . $action : '';
    drupal_goto($path);
  }
}

/**
 * TPPS Submission form submitter for 'Back' button.
 */
function tpps_submission_list_back() {
  // For some reasons messages from export/import pages duplicates when 'Back'
  // button clicked so we simply clear all messages out.
  drupal_get_messages();
  drupal_goto('tpps/submission');
}
