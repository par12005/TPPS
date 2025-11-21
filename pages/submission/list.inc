<?php

/**
 * @file
 * List of TPPS Submission with action buttons.
 *
 * @TODO Minor. Show submission list as table?
 */

module_load_include('inc', 'tpps', 'includes/common');
module_load_include('inc', 'tpps', 'includes/form');

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
    'create', 'view', 'export', 'import', 'compare', 'remove', 'purge', 'zenodo', 'edit',
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
  tpps_form_autofocus($form, ['tpps_submission_accession']);
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
 * Adds 'Back' button to TPPS Submission subforms.
 *
 * NOTE:
 * Used on subpages for navigation.
 *
 * @param array $form
 *   Drupal Form API array.
 */
function tpps_submission_list_add_buttons(array &$form, $add_action = TRUE) {
  $form['actions'] = $form['actions'] ?? ['#type' => 'container'];
  // Back-button.
  $action = 'back';
  $form['actions'][$action] = [
    '#type' => 'submit',
    '#name' => $action,
    '#value' => t(ucfirst($action)),
    '#submit' => ['tpps_submission_list_back'],
  ];
  if ($add_action) {
    // Main action button.
    $main_action = check_plain(arg(3));
    // Get caller function name.
    $function = debug_backtrace(
      !DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 2
    )[1]['function'];
    $form['actions'][$main_action] = [
      '#type' => 'submit',
      '#name' => $main_action,
      '#value' => t(ucfirst($main_action)),
      '#submit' => [$function . '_submit'],
    ];
  }
}

/**
 * Validates study accession for subforms.
 *
 * @param string $accession
 *   Study accession. Example: TGDR23423.
 */
function tpps_submission_list_validate_accession($accession) {
  if (empty($accession)) {
    drupal_set_message(t('Empty accession in @function().',
      [
        '@function' => debug_backtrace(
          !DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 2
        )[1]['function'],
      ]
    ), 'warning');
    drupal_goto('tpps/submission');
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
