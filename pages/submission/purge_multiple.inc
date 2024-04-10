<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Gets list of all studies and first and last broken studies.
 */
function tpps_submission_purge_multiple_get_range() {
  $output = [
    'accession_list' => tpps_submission_get_accession_list(),
    // Note: without 'TGDR' - only number.
    'latest_accession' => tpps_init_project_get_next_accession() - 1,
  ];
  // Note: without 'TGDR' - only number.
  $output['last_real_accession'] = str_replace('TGDR', '',
    array_keys($output['accession_list'])[0]) + 1;
  return $output;
}

/**
 * Menu callback. TPPS Study View Form.
 */
function tpps_submission_purge_multiple_form(array $form, array &$form_state, $range = '') {
  $data = tpps_submission_purge_multiple_get_range();

  if ($data['latest_accession'] > $data['last_real_accession']) {
    $accession_list = array_merge(
      [
        'all_broken' => t(
          'All Broken Studies (from TGDR@start to TGDR@end)',
          [
            '@start' => $data['latest_accession'],
            '@end' => $data['last_real_accession'],
          ]
        ),
      ],
      $data['accession_list']
    );
  }
  else {
    $accession_list = $data['accession_list'];
  }

  $form['tpps_submission_accession'] = [
    '#type' => 'checkboxes',
    '#title' => t('Accessions'),
    '#options' => $accession_list,
    '#required' => TRUE,
  ];

  // Pre select items specified in URL.
  // Note: given values will be included.
  $range = str_replace('TGDR', '', check_plain(trim($range)));
  if ($range) {
    $max_accession_number = str_replace('TGDR', '',
      array_keys($accession_list)[0]
    );
    if (strpos($range, '-') === FALSE) {
      // Single value. This is min value.
      if ($range < $max_accession_number) {
        $range = [
          '0' => check_plain(trim($range)),
          '1' => $max_accession_number,
        ];
      }
      else {
        $range = [];
      }
    }
    else {
      $range = explode('-', $range);
      $range[0] = empty($range[0]) ? 0 : $range[0];
      $range[1] = empty($range[1]) ? $max_accession_number : $range[1];
      sort($range);
    }
    if (is_numeric($range[0] ?? NULL) && is_numeric($range[1] ?? NULL)) {
      for ($i = $range[0]; $i <= $range[1]; $i++) {
        $default_value[] = 'TGDR' . $i;
      }
      $form['tpps_submission_accession']['#default_value'] = $default_value ?? NULL;
    }
  }

  $form['actions'] = ['#type' => 'container'];
  $buttons = ['back', 'purge'];
  foreach ($buttons as $key) {
    $form['actions'][$key] = [
      '#type' => 'submit',
      '#name' => $key,
      '#value' => t(ucfirst($key)),
    ];
  }
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_purge_multiple_form_submit(array $form, array &$form_state) {
  $accession_list = array_filter(
    array_values($form_state['input']['tpps_submission_accession'] ?? [])
  );
  if (!empty($accession_list)) {
    foreach ($accession_list as $accession) {
      if ($accession == 'all_broken') {
        $data = tpps_submission_purge_multiple_get_range();
        $submission = new Submission();
        for ($i = $data['latest_accession']; $i >= $data['last_real_accession']; $i--) {
          $submission->accession = 'TGDR' . $i;
          $submission->purge();
          drupal_set_message(t("Submission @accession and it's files was "
            . "removed and cache was cleared.", ['@accession' => 'TGDR' . $i]
          ));
        }
      }
      else {
        $submission = new Submission($accession);
        $submission->purge();
        drupal_set_message(
          t("Submission @accession and it's files was removed and cache was cleared.",
            ['@accession' => $accession]
          )
        );
      }
    }
    cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
  }
  else {
    drupal_set_message(t('No accession was selected.'));
  }
  //drupal_goto('tpps/submission/');
}
