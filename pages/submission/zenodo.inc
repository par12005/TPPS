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
 */
function tpps_submission_zenodo_form(array $form, array &$form_state = [], $accession = NULL) {
  // Check submission accession.
  $path = 'tpps/submission';
  if (empty($accession)) {
    drupal_set_message(t('Empty acession. Please choose one from the list below.'), 'error');
    drupal_goto($path);
  }
  tpps_submission_list_validate_accession($accession);
  $submission = new Submission($accession);

  // Check if submission exists.
  $form['accession'] = ['#type' => 'hidden', '#value' => $accession];
  $deposit_id = $submission->getZenodoDepositId();

  // Get Zenodo Depositon.
  if ($deposit_id) {
    $form[$deposit_id] = [
      '#type' => 'fieldset',
      '#title' => t('Zenodo Deposit Id: @id', ['@id' => $deposit_id]),
    ];
    $data['deposit_id'] = $deposit_id;
    $response = ZenodoDeposit::retrieve($data);
    if (!empty($response)) {
      $output = $response;
      if (is_array($response)) {
        $output = ZenodoDeposit::printDeposition($response);
      }
    }
    $form[$deposit_id]['dump'] = ['#markup' => $output ?? t('Empty')];
  }
  else {
    $form['message'] = [
      '#markup' => t("Submission doesn't have Zenodo Deposition"),
    ];
  }

  // Show buttons.
  $action_list = [
    [
      'title' => 'Zenodo Deposition',
      'button_list' => [
        'create', 'update', 'delete',
      ],
    ],
    [
      'title' => 'Zenodo Deposition Actions',
      'button_list' => [
        'publish', 'newversion', 'discard', 'edit',
      ],
    ],
    // @todo Requires file id which requires extra request to get them.
    [
      'title' => 'Zenodo Deposition Files',
      'button_list' => [
        'create', 'update', 'retrieve', 'delete', 'list',
      ],
    ],
  ];
  foreach ($action_list as $action) {
    $group_name = str_replace(' ', '_', strtolower($action['title']));
    if (!$deposit_id && $group_name != 'zenodo_deposition') {
      continue;
    }
    $form[$group_name] = [
      '#type' => 'fieldset',
      '#title' => t($action['title']),
      '#tree' => TRUE,
    ];
    foreach ($action['button_list'] as $key) {
      // Show only 'Create' button if there is no Zenodo Deposition and
      // don't show 'Create' Button when Depositon exists.
      if (
        (!$deposit_id && !($key == 'create' && $group_name == 'zenodo_deposition'))
        || ($deposit_id && ($key == 'create' && $group_name == 'zenodo_deposition'))
      ) {
        continue;
      }
      $form[$group_name][$key] = [
        '#type' => 'submit',
        '#name' => $group_name . '_' . $key,
        '#value' => t(ucfirst($key)),
      ];
    }
  }

  // WARNING: This CSS class adds theme and design.
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  tpps_submission_list_add_buttons($form, FALSE);
  return $form;
}

/**
 * Form validator for 'tpps_submission_list_form' form.
 */
function tpps_submission_zenodo_form_validate(array $form, array &$form_state) {
  // Check accession.
  if (empty($accession = $form_state['values']['accession'] ?? '')) {
    form_set_error('accession',
      t('Empty accession. Please choose from the list below.')
    );
  }
  // @TOdO Validate clicked button.
}

/**
 * Form submitter for 'tpps_submission_list_form' form.
 */
function tpps_submission_zenodo_form_submit(array &$form, array &$form_state) {
  $data = [];
  $accession = $form_state['values']['accession'] ?? '';
  $data['accession'] = $accession;
  list($group_name, $action) = $form_state['triggering_element']['#array_parents'];
  $submission = new Submission($accession);
  $deposit_id = $submission->getZenodoDepositId();

  if (empty($deposit_id)) {
    if (
      ($group_name == 'deposition' && !in_array($action, ['create', 'list']))
      || ($group_name == 'deposition_files')
    ) {
      drupal_set_message(t('Submission has no Zenodo Deposit Id.'), 'error');
      drupal_goto($path . '/' . $accession);
    }
  }
  else {
    $data['deposit_id'] = $deposit_id;
  }

  switch ($group_name) {
    case 'zenodo_deposition':
    case 'zenodo_deposition_actions':
      $class = 'ZenodoDeposit';
      break;

    case 'zenodo_deposition_files':
      $class = 'ZenodoFiles';
      break;
  }

  // Validate $action.
  $method_name = $class . '::' . $action;
  if (!is_callable($method_name, TRUE)) {
    drupal_set_message(t('Attempt to call non-existing Zenodo API action: @name.',
      ['@name' => $method_name]), 'error'
    );
    drupal_goto($path . '/' . $accession . '/zenodo');
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get response.
  $response = call_user_func($method_name, $data);

  // Output response.
  $output = $response ? ZenodoDeposit::printDeposition($response) : t('No data');
  $form['response'] = ['#type' => 'fieldset', '#title' => t('Response')];
  $form['response']['data'] = ['#markup' => $output];
  $form_state['rebuild'] = TRUE;
}
