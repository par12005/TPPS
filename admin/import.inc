<?php

/**
 * @file
 * TPPS Study Import Form and related functions.
 */

/**
 * TPPS Study Import Form.
 */
function tpps_study_import_form(array $form, array &$form_state) {
  $domain = variable_get('tpps_import_remote_domain', 'https://treegenesdb.org');
  $form['tpps_import_remote_domain'] = [
    '#type' => 'textfield',
    '#title' => t('Remote Domain'),
    '#default_value' => $domain ?? '',
    '#required' => TRUE,
    '#description' => t(
      'Default: https://treegenesdb.org<br />'
      . 'Domain name with schema but without trailing slash.'
    ),
  ];
  $form['tpps_import_remote_path'] = [
    '#type' => 'textfield',
    '#title' => t('Remote Path'),
    '#default_value' => variable_get('tpps_import_remote_path',
      'files/'
    ),
    '#required' => TRUE,
    '#description' => t(
      'Default: files/<br />'
      . "Path to folder where study's files are stored."
    ),
  ];
  // @TODO Minor. Use file field instead.
  $form['tpps_import_submission_state'] = [
    '#type' => 'textarea',
    '#title' => t('Submission State'),
    '#default_value' => variable_get('tpps_import_submission_state', ''),
    '#required' => TRUE,
    '#description' => t(
      'Open special page on remote site or use DB dump of certain study.'
      . '<br />Steps:'
      . '<br /> 1. Open: ' . l($domain . '/phppgadmin/', $domain . '/phppgadmin/')
      . '<br /> 2. Choose table: "public"."tpps_submission"'
      . '<br /> 3. Choose "SQL-query".'
      . '<br /> 4. Run: SELECT submission_state FROM "public"."tpps_submission" WHERE accession = "###";'
      . '<br /> 5. Click "Download".'
      . '<br /> 6. Choose "CSV".'
      . "<br /> 7. Open file and paste it's content."
    ),
  ];
  $form['#submit'][] = 'tpps_study_import_form_submit';
  return system_settings_form($form);
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_study_import_form_submit(array $form, array &$form_state) {
  // Remove useless data from the submission state.
  $state = $form_state['values']['tpps_import_submission_state'];
  if (substr($state, 0, 18) == '"submission_state"') {
    $state = mb_substr($state, 21);
    $state = rtrim($state, '"');
    $state = html_entity_decode($state);
    $state = str_replace('""', '"', $state);
  }
  // @see http://mozg.work/index.php/TPL:_Submission
  dpm(print_r(check_plain(substr($state, 2398, 10)), 1));
  dpm(print_r(unserialize($state), 1));
}
