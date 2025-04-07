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
  ZenodoDeposit::printHostStatus();

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

  if (!ZenodoDeposit::isHostValid($data)) {
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
  $response = ZenodoFiles::list($data);
  if (!empty($response)) {
    foreach ($response as $item) {
      foreach ($item as $key => $value) {
        $links = '';
        if ($key == 'links') {
          foreach ($value as $name => $url) {
            $links[] = l($name, $url);
          }
          $value = implode(', ', $links);
        }
        $zenodo_file_list[$item['id']][$key] = $value;
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission files.
  foreach (array_unique($submission->getFileList()) as $fid) {
    $create_url = $path . '/' . $accession . '/zenodo/files/' . $fid . '/create';
    if (empty($file = tpps_file_load($fid))) {
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
        $rows[$fid]['zenodo'] =
          // 'Delete' button.
          l(t('Delete'), $delete_url, $action_link_params)
          // Zenodo File metadata.
          . tpps_array_print_list($zenodo_file);
        unset($zenodo_file_list[$zenodo_file['id']]);
      }
    }
    $update_url = $path . '/' . $accession . '/zenodo/files/' . $fid . '/update';
    unset($submission_file['md5']);
    $rows[$fid]['submission'] =
      // 'Create' or 'Update' button.
      (
        $rows[$fid]['zenodo']
        ? l(t('Update'), $update_url, $action_link_params)
        : l(t('Create'), $create_url, $action_link_params)
      )
      // Drupal File metadata.
      . tpps_array_print_list($submission_file);
  }

  // Add Zenodo files which not match Submission Files.
  if (!empty($zenodo_file_list)) {
    foreach ($zenodo_file_list as $zenodo_file) {
      $rows[$zenodo_file['id']]['submission'] = '';
      $delete_url = $path . '/' . $accession . '/zenodo/files/' . $zenodo_file['id'] . '/delete';
      $rows[$zenodo_file['id']]['zenodo'] =
        l(t('Delete'), $delete_url, $action_link_params)
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
  tpps_submission_list_add_buttons($form, FALSE);
  return $form;
}
