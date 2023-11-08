<?php

/**
 * @file
 * TPPS Submission Export Form and related functions.
 *
 * @TODO Minor. Get status of submission in
 * https://tgwebdev.cam.uchc.edu/admin/reports/db/public.tripal_jobs.
 * @TODO Check $form_state['saved_values'] for files but it's structure differs
 * from $form_state['value'] and splitted by pages. But some studies at
 * dev-server (maybe broken) has no 'values' but only 'saves_values' but in
 * reviewed studies files was detected correclty using 'comolete form'.
 */

module_load_include('inc', 'tpps', 'includes/common');

/**
 * TPPS Study Import Form.
 */
function tpps_submission_compare_form(array $form, array &$form_state) {
  $accession_1 = (($form_state['input']['op'] ?? NULL) == t('Reset')
    ? '' : ($form_state['input']['tpps_submission_compare_accession_1'] ?? '')
  );
  $form['tpps_submission_compare_accession_1'] = [
    '#type' => 'select',
    '#title' => t('Accession 1'),
    // Reset cached list of studies to have latest changes.
    '#options' => tpps_submission_get_accession_list([], TRUE, $reset = TRUE),
    '#default_value' => $accession_1,
    '#required' => TRUE,
  ];
  $accession_2 = (($form_state['input']['op'] ?? NULL) == t('Reset')
    ? '' : ($form_state['input']['tpps_submission_compare_accession_2'] ?? '')
  );
  $form['tpps_submission_compare_accession_2'] = [
    '#type' => 'select',
    '#title' => t('Accession 2'),
    // No need to reset cached list of studies because we did this above.
    '#options' => tpps_submission_get_accession_list(),
    '#default_value' => $accession_2,
    '#required' => TRUE,
  ];
  $form['tpps_submission_compare_dump_function'] = [
    '#type' => 'select',
    '#title' => t('Dump Function'),
    '#options' => [
      'var_export' => 'var_export()',
      'print_r' => 'print_r()',
    ],
    '#default_value' => $form_state['input']['tpps_submission_compare_dump_function'] ?? 'var_export',
    '#description' => t('Function var_export() gives smaller dump but will'
      . 'fail on recurcive elements. Function print_r() safer but created '
      . "huge dumps which can't be imported"),
  ];

  // Actions
  $form['action_compare'] = [
    '#type' => 'submit',
    '#value' => t('Compare'),
    '#submit' => ['tpps_submission_compare_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_submission_compare_form_reset'],
  ];
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_submission_compare_form_reset() {
  $form_state['input']['tpps_submission_compare_accession_1'] = '';
  $form_state['input']['tpps_submission_compare_accession_2'] = '';
  drupal_goto('admin/config/tpps/submission/compare');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_compare_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'tpps', 'includes/submission');
  $accession_1 = $form_state['input']['tpps_submission_compare_accession_1'];
  $accession_2 = $form_state['input']['tpps_submission_compare_accession_2'];

  $result_1 = array_diff_assoc_recursive(
    tpps_submission_get_state($accession_1),
    tpps_submission_get_state($accession_2)
  );
  $result_2 = array_diff_assoc_recursive(
    tpps_submission_get_state($accession_2),
    tpps_submission_get_state($accession_1)
  );
  // @TODO Show in 2 columns.
  //dpm(print_r($result_1, 1), "$accession_1 vs $accession_2");
  //dpm(print_r($result_2, 1), "$accession_2 vs $accession_1");

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $function = $form_state['values']['tpps_submission_compare_dump_function'] ?? 'var_export';
  // Show report.
  foreach (array_keys($result_1) as $key) {
    $rows[] = [
      [
        'data' => $key,
        'style' => 'vertical-align:top;font-weight:bold;'
      ],
      [
        'data' => '<pre>' . call_user_func($function, $result_1[$key], 1) . '</pre>',
        'style' => 'vertical-align:top;'
      ],
      [
        'data' => '<pre>' . call_user_func($function, ($result_2[$key] ?? NULL), 1) . '</pre>',
        'style' => 'vertical-align:top;'
      ],
    ];
    unset($result_2[$key]);
  }
  foreach ($result_2 as $key => $value) {
    $rows[] = [
      $key, '', '<pre>' . call_user_func($function, $value, 1) . '</pre>',
    ];
    unset($result_2[$key]);
  }
  $header = ['Key', $accession_1, $accession_2];
  drupal_set_message(theme('table', ['header' => $header, 'rows' => $rows]));
  $form_state['rebuild'] = 1;
}

/**
 * Array_diff_assoc_recursive.
 *
 * Idea: https://www.php.net/manual/en/function.array-diff-assoc.php.
 *
 * @param array $array1
 *   Array #1 to compare.
 * @param array $array2
 *   Array #1 to compare.
 *
 * @return array
 *   Returns difference.
 */
function array_diff_assoc_recursive(array $array1, array $array2) {
  $difference = array();
  foreach ($array1 as $key => $value) {
    if (is_array($value)) {
      if (!isset($array2[$key]) || !is_array($array2[$key])) {
        $difference[$key] = $value;
      }
      else {
        $new_diff = array_diff_assoc_recursive($value, $array2[$key]);
        if (!empty($new_diff)) {
          $difference[$key] = $new_diff;
        }
      }
    }
    elseif (!array_key_exists($key, $array2) || $array2[$key] !== $value) {
      $difference[$key] = $value;
    }
  }
  return $difference;
}

