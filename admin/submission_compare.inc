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
function tpps_submission_compare_form(array $form, array &$form_state, $accession_1 = '', $accession_2 = '') {
  if (($form_state['input']['op'] ?? NULL) == t('Reset')) {
    $accession_1 = $accession_2 = '';
  }
  else {
    $accession_1 = $accession_1 ?? $form_state['input']['tpps_submission_compare_accession_1'] ?? '';
    $accession_2 = $accession_2 ?? $form_state['input']['tpps_submission_compare_accession_2'] ?? '';
  }
  $options = tpps_submission_get_accession_list([], TRUE, $reset = TRUE);
  $form['tpps_submission_compare_accession_1'] = [
    '#type' => 'select',
    '#title' => t('Accession 1'),
    // Reset cached list of studies to have latest changes.
    '#options' => $options,
    '#default_value' => $accession_1,
    '#required' => TRUE,
  ];
  $form['tpps_submission_compare_accession_2'] = [
    '#type' => 'select',
    '#title' => t('Accession 2'),
    // No need to reset cached list of studies because we did this above.
    '#options' => $options,
    '#default_value' => $accession_2,
    '#required' => TRUE,
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
 * Resets TPPS Submission Compare Form.
 */
function tpps_submission_compare_form_reset() {
  $form_state['input']['tpps_submission_compare_accession_1'] = '';
  $form_state['input']['tpps_submission_compare_accession_2'] = '';
  drupal_goto('admin/config/tpps/submission/compare');
}

/**
 * Submitter for TPPS Submission Compare form.
 */
function tpps_submission_compare_form_submit(array $form, array &$form_state) {
  $accession_1 = $form_state['input']['tpps_submission_compare_accession_1'];
  $accession_2 = $form_state['input']['tpps_submission_compare_accession_2'];
  if ($accession_1 == $accession_2) {
    drupal_set_message(t('Submission @accession_1 and @accession_2 are equal.',
      ['@accession_1' => $accession_1, '@accession_2' => $accession_2]
    ));
    $form_state['rebuild'] = 1;
    return;
  }
  drupal_goto('admin/config/tpps/submission/compare/' . $accession_1 . '/' . $accession_2);
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

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/**
 * Menu callback. TPPS Submission Compare Page.
 */
function tpps_submission_compare(array $form, array &$form_state, $accession_1 = '', $accession_2 = '') {
  if (empty($accession_1) || empty($accession_2)) {
    drupal_set_message(t('Two accessions are required. '
      . 'Provided: "@accession_1" and "@accession_2".',
      ['@accession_1' => $accession_1, '@accession_2' => $accession_2]
    ));
    drupal_goto('admin/config/tpps/submission/compare/' .
      $accession_1 . '/' . $accession_2
    );
  }

  $form = [];
  $form['tpps_submission_accession_1'] = [
    '#type' => 'hidden',
    '#value' => $accession_1,
  ];
  $form['tpps_submission_accession_2'] = [
    '#type' => 'hidden',
    '#value' => $accession_2,
  ];
  module_load_include('inc', 'tpps', 'includes/submission');
  $result_1 = array_diff_assoc_recursive(
    tpps_submission_get_state($accession_1),
    tpps_submission_get_state($accession_2)
  );
  $result_2 = array_diff_assoc_recursive(
    tpps_submission_get_state($accession_2),
    tpps_submission_get_state($accession_1)
  );

  $form['header_back'] = [
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => ['tpps_submission_compare_form_reset'],
  ];
  if (empty($result_1) && empty($result_2)) {
    drupal_set_message(t('Submissions "@accession_1" and "@accession_2" are equal.',
      ['@accession_1' => $accession_1, '@accession_2' => $accession_2]
    ));
    return $form;
  }
  $form['warning'] = [
    '#markup' => t('<h3>WARNING: Shown only items which are different.</h3>'),
  ];
  $form['links']['#markup'] = theme('item_list', [
    'items' => [
      $accession_1 . ': ' . implode(', ', [
        l(t('View Submission'), 'admin/config/tpps/submission/view/' . $accession_1),
        l(t('Study at Admin Panel'), 'tpps-admin-panel/' . $accession_1),
      ]),
      $accession_2 . ': ' . implode(', ', [
        l(t('View Submission'), 'admin/config/tpps/submission/view/' . $accession_2),
        l(t('Study at Admin Panel'), 'tpps-admin-panel/' . $accession_2),
      ]),
    ],
  ]);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show comparison report.
  $parsed_1 = tpps_array_dump($result_1, $sticky = FALSE);
  $parsed_2 = tpps_array_dump($result_2, $sticky = FALSE);
  // Process items only in $result_2.
  tpps_submission_compare_show_diff($form, $parsed_1, $parsed_2);
  $form['footer_back'] = [
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => ['tpps_submission_compare_form_reset'],
  ];
  return $form;
}

/**
 * Shows comparison report in 2 columns.
 *
 * @param array $form
 *   Drupal Form API Array.
 * @param array $parsed_1
 *   Prepared array. See tpps_array_dump().
 *   Keys are: 'title', 'data' and 'count'.
 * @param array $parsed_2
 *   Prepared array. See tpps_array_dump().
 *   Keys are: 'title', 'data' and 'count'.
 */
function tpps_submission_compare_show_diff(array &$form, array &$parsed_1, array &$parsed_2) {
  if (
    (empty($parsed_1) && empty($parsed_2))
    || empty($accession_1 = ($form['tpps_submission_accession_1']['#value'] ?? ''))
    || empty($accession_2 = ($form['tpps_submission_accession_2']['#value'] ?? ''))
  ) {
    return;
  }
  $key_list = !empty($parsed_1) ? array_keys($parsed_1) : array_keys($parsed_2);
  foreach ($key_list as $key) {
    $title_1 = $accession_1 . (($parsed_1[$key]['count'] ?? NULL)
      ? ' (' . $parsed_1[$key]['count'] . ')' : '');
    $title_2 = $accession_2 . (($parsed_2[$key]['count'] ?? NULL)
      ? ' (' . $parsed_2[$key]['count'] . ')' : '');
    $fieldset_title = ($parsed_1[$key]['title'] ?? $key)
         . ' [' . ($parsed_1[$key]['count'] ?? 0)
         . ' / ' . ($parsed_2[$key]['count'] ?? 0) . ']';
    $form[$key . '_container'] = [
      '#type' => 'fieldset',
      '#title' => $fieldset_title,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      // Show 2 columns.
      $key . '_data' => [
        '#markup' => theme('table', [
          'header' => [$title_1, $title_2],
          'rows' => [
            [
              [
                'data' => ($parsed_1[$key]['data'] ?? NULL),
                'style' => 'width: 50%; vertical-align: top;',
              ],
              [
                'data' => ($parsed_2[$key]['data'] ?? NULL),
                'style' => 'width: 50%; vertical-align: top;',
              ],
            ],
          ],
        ]),
      ],
    ];
    unset($parsed_2[$key]);
  }
  // Do not change $parsed_1 array to not break the loop but for now
  // it was completely processed.
  $parsed_1 = [];
  tpps_submission_compare_show_diff($form, $parsed_1, $parsed_2);
}
