<?php

/**
 * @file
 * List of Zenodo Depositon Files.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. Shows Zenodo Action buttons.
 */
function tpps_submission_zenodo_files_list_form(array $form, array &$form_state = [], $accession = NULL) {
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
  // Validate.
  if (empty($deposit_id)) {
    $message = t('Submission has no Zenodo Deposit Id. Create Zenodo Deposition first.');
    drupal_set_message($message, 'error');
    drupal_goto($path . '/' . $accession . '/zenodo');
  }

  $rows = $zenodo_file_list = $submission_files = [];
  $header = [
    t('Submission'),
    t('Zenodo'),
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
  $response = ZenodoFiles::list(['deposit_id' => $deposit_id]);
  if (!empty($response)) {
    foreach ($response as $item) {
      foreach ($item as $key => $value) {
        $zenodo_file_list[$item['id']][$key] = $value;
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission files.
  foreach ($submission->getFileList() as $fid) {
    if ($file = tpps_file_load($fid)) {
      $submission_file = [
        'fid' => $file->fid,
        'uid' => $file->uid,
        'filename' => $file->filename,
        'uri' => $file->uri,
        'filemime' => $file->filemime,
        'filesize' => $file->filesize,
        'status' => $file->status,
        'timestamp' => $file->timestamp,
        'md5' => md5(file_get_contents(tpps_realpath($file->uri))),
      ];
      $rows[$fid] = [
        'submission' => tpps_array_print_list($submission_file),
        'zenodo' => '',
      ];
    }
    // Find relations between Submission and Zenodo files.
    if (!empty($zenodo_file_list)) {
      foreach ($zenodo_file_list as $zenodo_file) {
        if ($zenodo_file['checksum'] != $submission_file['md5']) {
          continue;
        }
        $rows[$fid]['zenodo'] = tpps_array_print_list($zenodo_file);
        unset($zenodo_file_list[$zenodo_file['id']]);
      }
    }
  }
  if (!empty($zenodo_file_list)) {
    foreach ($zenodo_file_list as $zenodo_file) {
      $rows[$zenodo_file['id']]['submission'] = '';
      $rows[$zenodo_file['id']]['zenodo'] = tpps_array_print_list($zenodo_file);
      unset($zenodo_file_list[$zenodo_file['id']]);
    }
  }

  // Output.
  $form['table'] = [
    '#markup' => theme('table', [
      'header' => $header,
      'rows' => $rows,
    ]),
  ];


// @TODO Add action- buttons under each cell in table.


  // WARNING: This CSS class adds theme and design.
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  tpps_submission_list_add_buttons($form, FALSE);
  return $form;
}


// @TODO REview below.


/**
 * Form validator for 'tpps_submission_list_form' form.
 */
function tpps_submission_zenodo_files_list_form_validate(array $form, array &$form_state) {
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
function tpps_submission_zenodo_files_list_form_submit(array &$form, array &$form_state) {
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
