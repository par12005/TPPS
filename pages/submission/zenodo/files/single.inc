<?php

/**
 * @file
 * Zenodo Single File  page.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. Shows Zenodo Action buttons.
 */
function tpps_submission_zenodo_files_single_form(array $form, array &$form_state = [], $accession = NULL, $file_id = NULL) {
  // Check submission accession.
  $path = 'tpps/submission';
  if (empty($accession)) {
    drupal_set_message(t('Empty acession. Please choose one from the list below.'), 'error');
    drupal_goto($path);
  }
  tpps_submission_list_validate_accession($accession);
  $submission = new Submission($accession);
  $form['accession'] = ['#type' => 'hidden', '#value' => $accession];
  $deposit_id = $submission->getZenodoDepositId();
  // Validate Zenodo Deposit Id.
  if (empty($deposit_id)) {
    $message = t('Submission has no Zenodo Deposit Id. Create Zenodo Deposition first.');
    drupal_set_message($message, 'error');
    drupal_goto($path . '/' . $accession . '/zenodo');
  }

  if (empty($file_id)) {
    $message = t('Empty Zenodo File Id. Please add file to Zenodo Deposition first.');
    drupal_set_message($message, 'error');
    drupal_goto($path . '/' . $accession . '/zenodo/files');
  }

  // Get action (if any).
  $action = arg(6) ?? NULL;

  if ($action) {
    if (is_numeric($file_id)) {
      // $file_id is the Drupal File Id.
      $file = tpps_file_load($file_id);

      // Create.
      if ($file && $action == 'create') {
        $data['drupal_file'] = $file;
        $data['deposit_id'] = $deposit_id;
        ZenodoBucket::create($data);
        //ZenodoFiles::create($data);

        // Validate response and show status message.
        if (empty($data['response']['id'])) {
          // Fail.
          $message = t('Drupal File "@file_id" was NOT added to Deposition '
            . '"@deposit_id". <br /><strong>Response</strong>: @status - @message.',
            [
              '@status' => $data['response']['status'] ?? '',
              '@message' => $data['response']['messaage'] ?? '',
              '@file_id' => $file_id,
              '@deposit_id' => $deposit_id,
            ]
          );
          drupal_set_message($message, 'error');
          drupal_goto($path . '/' . $accession . '/zenodo/files');
          // Do not redirect to stay at single Zenodo File page.
        }
        else {
          // Success.
          // Code: 201 Created
          // Body: a deposition file resource.
          $message = t('Drupal file #@file_id was successfully added to '
            . 'Zenodo Deposition #@deposit_id. '
            . '<br /><strong>Zenodo File Id</stong>: @zenodo_file_id.',
            [
              '@deposit_id' => $deposit_id,
              '@file_id' => $file_id,
              '@zenodo_file_id' => $data['response']['id'],
            ]
          );
          drupal_set_message($message, 'status');
          drupal_goto($path . '/' . $accession . '/zenodo/files/' . $file_id);
        }
      }
    }
    else {
      // $file_id is the Zenodo File Id.
      if ($action == 'delete') {
        $data = ['deposit_id' => $deposit_id, 'file_id' => $file_id];
        ZenodoFiles::delete($data);
        // Validate response and show status message.
        if (empty($data['response'])) {
          // Code: 204 No Content.
          $message = t('Zenodo File "@file_id" was successfully removed '
            . 'from Zenodo Deposition #@deposit_id.',
            ['@deposit_id' => $deposit_id, '@file_id' => $file_id]
          );
          drupal_set_message($message, 'status');
        }
        else {
          $message = t('Zenodo File "@file_id" was NOT removed. Response: @response.',
            ['@response' => $data['response'], '@file_id' => $file_id]
          );
          drupal_set_message($message, 'error');
        }
        // There is nothing to show when Zenodo File was removed and error
        // message will be shown at Zenodo Files List page.
        drupal_goto($path . '/' . $accession . '/zenodo/files');
      }
    }
  }




// @TODO Show table of Submission's files and Zenodo files and action buttons.

    // Get list of Zenodo Files.
    $form[$deposit_id] = [
      '#type' => 'fieldset',
      '#title' => t('Zenodo Deposit #@id Files', ['@id' => $deposit_id]),
    ];
    $data['deposit_id'] = $deposit_id;
    $response = ZenodoFiles::list($data);
    if (!empty($response)) {
      $output = $response;
      if (is_array($response)) {
        $output = ZenodoDeposit::printDeposition($response);
      }
    }
    $output = $output ?? t("Zenodo Deposition doesn't have files.");
    $form[$deposit_id]['dump'] = ['#markup' => $output];

  // Show buttons.
  $form['actions'] = $form['actions'] ?? ['#type' => 'container'];
  $action_list = [
    'create', 'update', 'retrieve', 'delete',
    //'list',
  ];
  foreach ($action_list as $key) {
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
function tpps_submission_zenodo_files_single_form_validate(array $form, array &$form_state) {
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
function tpps_submission_zenodo_files_single_form_submit(array &$form, array &$form_state) {
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

  // Validate $action.
  $class = 'ZenodoFiles';
  $method_name = $class . '::' . $action;
  if (!is_callable($method_name, TRUE)) {
    drupal_set_message(t('Attempt to call non-existing Zenodo API action: @name.',
      ['@name' => $method_name]), 'error'
    );
    drupal_goto($path . '/' . $accession . '/zenodo/files');
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
