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
  global $user;
  // Check submission accession.
  $path = 'tpps/submission';
  if (empty($accession)) {
    $message = t('Empty acession. Please choose one from the list below.');
    drupal_set_message($message, 'error');
    drupal_goto($path);
  }
  tpps_submission_list_validate_accession($accession);
  $submission = new Submission($accession);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Check permsisions.
  $actions_allowed = ZenodoFilesWidget::isActionAllowed($user);
  if (!$actions_allowed) {
    // Probably user opened URL directly.
    $message = t('Actions with files not allowed.');
    drupal_set_message($message, 'error');
    drupal_goto($path);
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
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
    tpps_submission_zenodo_files_action($action, $deposit_id, $file_id, $accession);
  }

  if (!empty($_GET['destination'])) {
    drupal_goto($_GET['destination']);
  }
  // @TODO Temporary redirect to list-page because single file page doesn't work.
  drupal_goto($path . '/' . $accession . '/zenodo/files');



// @TODO Show table of Submission's files and Zenodo files and action buttons.

  // Get list of Zenodo Files.
  $form[$deposit_id] = [
    '#type' => 'fieldset',
    '#title' => t('Zenodo Deposit #@id Files', ['@id' => $deposit_id]),
  ];
  $data['deposit_id'] = $deposit_id;
  $response = ZenodoFile::list($data);
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
  $class = 'ZenodoFile';
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

/**
 * Form submitter for 'tpps_submission_list_form' form.
 *
 * Prepaires data for ZenodoFile methods and shows messages.
 */
function tpps_submission_zenodo_files_action($action, $deposit_id, $file_id, $accession) {
  $path = 'tpps/submission';
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if ($action == 'create') {
    if (
        // $file_id is the Drupal File Id.
      is_numeric($file_id)
      // File exists.
      && !empty($file = tpps_file_load($file_id))
    ) {
      $data['drupal_file'] = $file;
      $data['deposit_id'] = $deposit_id;
      ZenodoFile::create($data);
      // @TODO Validate response and show status message.
      // Success.
      $message = t('Drupal file #@file_id was successfully added to '
        . 'Zenodo Deposition #@deposit_id.',
        [
          '@deposit_id' => $deposit_id,
          '@file_id' => $file_id,
        ]
      );
      drupal_set_message($message, 'status');
      // @TODO Minor. Redirect to single file page in case of success.
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Delete.
  if ($action == 'delete') {
    if (is_numeric($file_id)) {
      // Drupal File Id.
      // @TODO Minor. Get Zenodo File Id by Drupal File Id and remove exception.
      $message = t('Removement by Drupal File Id not yet implemented');
      throw new Exception($message);
    }
    $data = ['deposit_id' => $deposit_id, 'file_id' => $file_id];
    try {
      ZenodoFile::delete($data);
      // Validate response and show status message.
      if (empty($data['response'])) {
        // Code: 204 No Content.
        $message = t('Zenodo File "@file_id" (Deposition #@deposit_id) was '
          . 'successfully deleted.',
          ['@deposit_id' => $deposit_id, '@file_id' => $file_id]
        );
        drupal_set_message($message, 'status');
      }
      else {
        $message = t('Removement of the Zenodo File "@file_id" failed. Response: @response.',
          ['@response' => $data['response'], '@file_id' => $file_id]
        );
        drupal_set_message($message, 'error');
      }
      // There is nothing to show when Zenodo File was removed and error
      // message will be shown at Zenodo Files List page.
      drupal_goto($path . '/' . $accession . '/zenodo/files');
    }
    catch (Exception $e) {
      $message = '@code@message';
      $params = [
        '@code' => $e->getCode() ? $e->getCode() . ' - ' : '',
        '@message' => $e->getMessage(),
        '!settings_page_link' => l(t('Zenodo Settings Page'), 'admin/config/tpps/zenodo'),
      ];
      drupal_set_message(t($message, $params), 'error');
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Update.
  if ($action == 'update') {
    $data = ['deposit_id' => $deposit_id];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get required Drupal File.
    // We could remove by Zenodo File Id but not update. To update we need
    // appropriate Drupal File.
    if (is_numeric($file_id)) {
      // Drupal File Id.
      if (empty($file = tpps_file_load($file_id))) {
        $message = t('Drupal File #@file_id was not found.',
          ['@file_id' => $file_id]
        );
        throw new Exception($message);
      }
      $data['drupal_file'] = $file;
    }
    else {
      // Zenodo File Id.
      $data['file_id'] = $file_id;
      // @TODO Minor. Get Zenodo File Id by Drupal File Id and remove exception.
      // Find submission by Zenodo Deposit Id. $data['deposit_id']
      $submission_cache = SubmissionCache::getAll($reset);
      $rows = [];
      $i = 0;
      foreach ($submission_cache as $accession => $item) {
        if (
          empty($item['zenodo'])
          || empty($item['zenodo']['deposit_id'])
          || $data['deposit_id'] != $item['zenodo']['deposit_id']
        ) {
          continue;
        }
        foreach ($submission_cache['file']['id_list'] as $fid => $filesize) {
          if (empty($fid) || is_null($filesize) || empty($file = tpps_file_load($fid))) {
            continue;
          }
          if (
            ZenodoFile::getChecksum(['zenodo_fid' => $file_id])
            == ZenodoFile::getChecksum(['drupal_file' => $file])
          ) {
            $data['drupal_file'] = $file;
          }
        }
      }
      if (empty($data['drupal_file'])) {
        $message = t("Dupal file for Zenodo File #@zenodo_fid wasn't found."
          . "It cound't be updated. Remove it and then create new one.",
          ['@zenodo_fid' => $file_id]
        );
        throw new Exception($message);
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Do the update.
    try {
      ZenodoFile::update($data);
      // Validate response and show status message.
      if (empty($data['response'])) {
        // Code: 204 No Content.
        $message = t('Zenodo File "@file_id" (Deposition #@deposit_id) was '
          . 'successfully updated.',
          ['@deposit_id' => $deposit_id, '@file_id' => $file_id]
        );
        drupal_set_message($message, 'status');
      }
      else {
        $message = t('Updating of the Zenodo File "@file_id" failed. Response: @response.',
          ['@response' => $data['response'], '@file_id' => $file_id]
        );
        drupal_set_message($message, 'error');
      }
      // There is nothing to show when Zenodo File was removed and error
      // message will be shown at Zenodo Files List page.
      drupal_goto($path . '/' . $accession . '/zenodo/files');
    }
    catch (Exception $e) {
      $message = '@code@message';
      $params = [
        '@code' => $e->getCode() ? $e->getCode() . ' - ' : '',
        '@message' => $e->getMessage(),
        '!settings_page_link' => l(t('Zenodo Settings Page'), 'admin/config/tpps/zenodo'),
      ];
      drupal_set_message(t($message, $params), 'error');
    }
  }
  // Return to the initial page to allow to use widget at any page.
  if (!empty($_GET['destination'])) {
    drupal_goto($_GET['destination']);
  }
}
