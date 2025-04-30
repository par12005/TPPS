<?php

/**
 * @file
 * Shows list of all Zenodo Depositon Files.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. Shows list of all Zenodo Depositon Files.
 */
function tpps_submission_zenodo_files_list_form(array $form, array &$form_state = [], $accession = NULL) {
  // Show Zenodo Host on page.
  if (empty($form_state['triggering_element'])) {
    ZenodoHost::printStatus();
  }

  // Check submission accession.
  $path = 'tpps/submission';
  if (empty($accession)) {
    drupal_set_message(t('Empty acession. Please choose one from the list below.'), 'error');
    drupal_goto($path);
  }
  tpps_submission_list_validate_accession($accession);
  $submission = new Submission($accession);
  $form['accession'] = ['#type' => 'hidden', '#value' => $accession];
  $data = ['deposit_id' => $submission->getZenodoDepositId()];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate.
  if (empty($data['deposit_id'])) {
    $message = t('Submission has no Zenodo Deposit Id. Create Zenodo Deposition first.');
    drupal_set_message($message, 'error');
    drupal_goto($path . '/' . $accession . '/zenodo');
  }

  if (!ZenodoHost::isValid($data)) {
    $message = t('Deposition was created using different Zenodo Host.');
    drupal_set_message($message, 'error');
    drupal_goto('tpps/submission/' . $accession . '/zenodo');
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Build output.
  $rows = $zenodo_file_list = $submission_files = [];
  $header = [
    t('Submission'),
    t('Zenodo'),
  ];
  $action_link_params = [
    'attributes' => [
      'class' => ['btn', 'form-submit', 'btn-primary'],
      'style' => 'display: inline-block;',
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Zenodo Files.
  //
  // Format:
  //  id (String, 36 characters ) 83fea13b-8a51-4b67-aa75-089570415401
  //  filename (String, 28 characters ) TGDR400_Phenotype_Data_1.csv
  //  filesize (Float) 12362
  //  checksum (String, 32 characters ) 4bd90201fbb420ae5c149a4af50a224b
  //  links (String, 218 characters ) <a href="https://zenodo.org/api/deposit/deposit...
  $response = ZenodoFile::list($data);
  if (!empty($response)) {
    foreach ($response as $item) {
      foreach ($item as $key => $value) {
        $links = '';
        if ($key == 'links') {
          foreach ($value as $name => $url) {
            $links[] = l($name, Zenodo::addAccessToken($url));
          }
          $value = implode(', ', $links);
        }
        $zenodo_file_list[$item['id']][$key] = $value;
      }
    }
  }

  // Deposition.
  $deposition = ZenodoDeposit::retrieve($data);
  if ($deposition['submitted']) {
    $message = t("Files can't be added or deleted because Zenodo Deposition "
      . "<strong>was published</strong>. Create new version of this Deposition "
      . "to add new or delete existing files."
    );
    drupal_set_message($message, 'warning');
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission files.
  $missing_files = [];
  foreach (array_unique($submission->getFileIdList()) as $fid) {
    $create_url = $path . '/' . $accession . '/zenodo/files/' . $fid . '/create';
    if (empty($file = tpps_file_load($fid))) {
      $missing_files[] = $fid;
      continue;
    }
    // Make it first.
    $rows[$fid]['submission'] = '';
    $rows[$fid]['zenodo'] = '';
    $submission_file = [
      'fid' => $file->fid,
      'uid' => $file->uid,
      'filename' => $file->filename,
      'uri' => $file->uri,
      'filemime' => $file->filemime,
      'filesize' => $file->filesize,
      'status' => $file->status,
      'timestamp' => $file->timestamp,
      'md5' => md5(file_get_contents(tpps_realpath($file->uri))) ?? '',
    ];
    // Find relations between Submission and Zenodo files.
    if (!empty($zenodo_file_list)) {
      foreach ($zenodo_file_list as $zenodo_file) {
        if ($zenodo_file['checksum'] != $submission_file['md5']) {
          continue;
        }
        // Note: Filesize will be the same but filenames could differ.
        unset($zenodo_file['checksum']);
        $zenodo_file_path = $path . '/' . $accession . '/zenodo/files/';
        $delete_url = $zenodo_file_path . $zenodo_file['id'] . '/delete';
        $update_url = $zenodo_file_path . $zenodo_file['id'] . '/update';
        // 'Delete' button.
        if (empty($deposition['submitted'])) {
          $delete_button = l(t('Delete'), $delete_url, $action_link_params);
        }
        $rows[$fid]['zenodo'] = ($delete_button ?? '')
          // Zenodo File metadata.
          . tpps_array_print_list($zenodo_file);
        unset($zenodo_file_list[$zenodo_file['id']]);
      }
    }
    $update_url = $path . '/' . $accession . '/zenodo/files/' . $fid . '/update';
    unset($submission_file['md5']);

    // Buttons for the Submission side (left).
    $submission_button = '';
    if (!empty($rows[$fid]['zenodo'])) {
      // Files could be updated in published and not yet published Depositions.
      $submission_button = l(t('Update'), $update_url, $action_link_params);
    }
    else {
      // New files could be added to not yet published Depositions.
      if (!$deposition['submitted']) {
        $submission_button = l(t('Create'), $create_url, $action_link_params);
      }
    }
    $rows[$fid]['submission'] = $submission_button
      . tpps_array_print_list($submission_file);
  }

  // Report about files missing in file system.
  if ($missing_files) {
    $message = t('Files listed below was NOT found in file system: <br />@list'
      . '<br />Probably they was removed by cron because of temporary status.'
      . '<br />Those files must be re-attached to study again.',
      ['@list' => implode(', ', $missing_files)]
    );
    drupal_set_message($message, 'error');
  }

  // Add Zenodo files which not match Submission Files.
  if (!empty($zenodo_file_list)) {
    foreach ($zenodo_file_list as $zenodo_file) {
      $rows[$zenodo_file['id']]['submission'] = '';
      $delete_url = $path . '/' . $accession . '/zenodo/files/' . $zenodo_file['id'] . '/delete';
      if (empty($deposition['submitted'])) {
        $delete_button = l(t('Delete'), $delete_url, $action_link_params);
      }
      $rows[$zenodo_file['id']]['zenodo'] = ($delete_button ?? '')
        . tpps_array_print_list($zenodo_file);
      unset($zenodo_file_list[$zenodo_file['id']]);
    }
  }

  // Output.
  $form['table'] = [
    '#markup' => theme('table', [
      'header' => $header,
      'rows' => $rows,
      'attributes' => ['class' => ['tpps_table']],
    ]),
  ];
  // WARNING: This CSS-class adds theme and design.
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('main', $form);
  tpps_add_css_js('theme', $form);

  // Add back button.
  $form['actions'] = $form['actions'] ?? ['#type' => 'container'];
  $action = 'back';
  $form['actions'][$action] = [
    '#type' => 'submit',
    '#name' => $action,
    '#value' => t($action),
  ];
  if (empty($deposition['submitted'])) {
    $action = 'upload_all';
    $form['actions'][$action] = [
      '#type' => 'submit',
      '#name' => $action,
      '#value' => t('Upload All'),
    ];
  }

  return $form;
}

/**
 * Form submitter for 'tpps_submission_list_form' form.
 */
function tpps_submission_zenodo_files_list_form_submit(array &$form, array &$form_state) {
  $accession = $form_state['values']['accession'] ?? '';
  $action = $form_state['triggering_element']['#name'];

  // 'Back' button.
  if ($action == 'back') {
    // For some reasons messages from export/import pages duplicates when 'Back'
    // button clicked so we simply clear all messages out.
    drupal_get_messages();
    drupal_goto('tpps/submission/' . $accession . '/zenodo');
  }
  elseif ($action == 'upload_all') {
    // Existing files will be overwritten and will got new ids.
    drupal_get_messages();
    $submission = new Submission($accession);
    if ($submission->doesNotExist()) {
      drupal_set_message(t('Submission @accession not found.',
        ['@accession' => $accession]
      ));
      drupal_goto('tpps/submission');
    }
    if (empty($submission->getZenodoDepositId())) {
      drupal_set_message(
        t('Submission "@accession" has empty Zenodo Deposit Id.',
        ['@accession' => $accession]
      ), 'error');
      drupal_goto('tpps/submission/' . $accession . '/zenodo');
    }
    $submission->uploadAllFilesZenodo();
  }
  $form_state['#rebuild'] = TRUE;
}
