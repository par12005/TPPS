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

  // Get Zenodo Deposition.
  if ($deposit_id) {
    $form[$deposit_id] = [
      '#type' => 'fieldset',
      '#title' => t('Zenodo Deposit Id: @id', ['@id' => $deposit_id]),
    ];
    $data['deposit_id'] = $deposit_id;
    try {
      $deposition = ZenodoDeposit::retrieve($data);
      // @todo Message about changed host.
      $message = t('Deposition was found on non-global Zenodo Host.');
      drupal_set_message($message, 'error');
      $output = $deposition;
      if (is_array($deposition)) {
        $output = ZenodoDeposit::printDeposition($deposition);
      }
      $form[$deposit_id]['dump'] = ['#markup' => $output];
    }
    catch (Exception $e) {
      if (in_array($e->getCode(), [403, 404])) {
        $deposition = NULL;
        $message = t('Submission has Deposit Id but it was not found. Check Zenodo host.');
        drupal_set_message($message, 'error');
      }
      // @TODO Idea: Switch to other host and retrieve deposition again.
      $form[$deposit_id]['dump'] = ['#markup' => t('No Depostion found.')];
    }
    ZenodoDeposit::printHostStatus(TRUE);
  }
  else {
    $form['message'] = [
      '#markup' => t("Submission doesn't have Zenodo Deposition"),
    ];
  }

  // Show buttons.
  $form['actions'] = $form['actions'] ?? ['#type' => 'container'];
  // Set of buttons when there is no Depostion.
  $action_list = ['create'];
  if (!empty($deposition)) {
    $action_list = [
      'update', 'delete',
      'publish', 'newversion', 'discard', 'edit',
      'files',
    ];
  }
  foreach ($action_list as $key) {
    // Show only 'Create' button if there is no Zenodo Deposition and
    // don't show 'Create' Button when Depositon exists.
    if (
      // Hide 'Edit' button for not yet submitted Depositions.
      $deposit_id && $key == 'edit' && empty($deposition['submitted'])
    ) {
      continue;
    }
    $form['actions'][$key] = [
      '#type' => 'submit',
      '#name' => $key,
      '#value' => t(ucfirst($key)),
    ];
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
  // Note: Deposit Id will be validated in form submitter because it requires
  // fully loaded Submission objec (which takes time).
  // @TOdO Minor. Check if clicked button is in the list of existing buttons.
}

/**
 * Form submitter for 'tpps_submission_list_form' form.
 */
function tpps_submission_zenodo_form_submit(array &$form, array &$form_state) {
  $data = [];
  $accession = $form_state['values']['accession'] ?? '';
  $data['accession'] = $accession;
  $action = $form_state['triggering_element']['#name'];
  $submission = new Submission($accession);
  $deposit_id = $submission->getZenodoDepositId();
  $path = 'tpps/submission';
  // Validate Deposit Id here intead of orm validator because it will be used
  // here and full load of the Submission object takes a lot of time.
  if (empty($deposit_id)) {
    if (!in_array($action, ['create', 'list', 'files'])) {
      $message = t('Submission has no Zenodo Deposit Id. Create Zenodo Deposition first.');
      drupal_set_message($message, 'error');
      drupal_goto($path . '/' . $accession . '/zenodo');
    }
  }
  else {
    $data['deposit_id'] = $deposit_id;
  }
  // Files has a subpage.
  if ($action == 'files') {
    drupal_goto($path . '/' . $accession . '/zenodo/files');
  }
  // Validate $action.
  $class = 'ZenodoDeposit';
  $method_name = $class . '::' . $action;
  if (!is_callable($method_name, TRUE)) {
    drupal_set_message(t('Attempt to call non-existing Zenodo API action: @name.',
      ['@name' => $method_name]), 'error'
    );
    drupal_goto($path . '/' . $accession . '/zenodo');
  }
  // Get response.
  $response = call_user_func($method_name, $data);

dpm($response);



  // Output response.
  $output = $response ? ZenodoDeposit::printDeposition($response) : t('No data');
  $form['response'] = ['#type' => 'fieldset', '#title' => t('Response')];
  $form['response']['data'] = ['#markup' => $output];
  $form_state['rebuild'] = TRUE;
}
