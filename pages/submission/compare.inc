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

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * TPPS Study Import Form.
 */
function tpps_submission_compare_form(array $form, array &$form_state, $accession_1 = '', $accession_2 = '') {
  tpps_submission_list_validate_accession($accession_1);
  $accession_list = tpps_submission_get_accession_list();
  $form['tpps_submission_accession_1'] = [
    '#type' => 'select',
    '#title' => t('Accession 1'),
    '#options' => $accession_list,
    '#default_value' => $accession_1 ?? array_key_first($accession_list) ?? '',
    '#required' => TRUE,
  ];
  $form['tpps_submission_accession_2'] = [
    '#type' => 'select',
    '#title' => t('Accession 2'),
    '#options' => $accession_list,
    '#default_value' => $accession_2 ?? array_key_first($accession_list) ?? '',
    '#required' => TRUE,
  ];
  tpps_form_autofocus($form, ['tpps_submission_accession_2']);
  tpps_submission_list_add_buttons($form);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  return $form;
}

/**
 * Submitter for TPPS Submission Compare form.
 */
function tpps_submission_compare_form_submit(array $form, array &$form_state) {
  $accession_1 = $form_state['input']['tpps_submission_accession_1'] ?? NULL;
  $accession_2 = $form_state['input']['tpps_submission_accession_2'] ?? NULL;
  tpps_submission_list_validate_accession($accession_1);
  if ($accession_1 == $accession_2) {
    drupal_set_message(t('Submission @accession_1 and @accession_2 are equal.',
      ['@accession_1' => $accession_1, '@accession_2' => $accession_2]
    ));
    $form_state['rebuild'] = 1;
    return;
  }
  if ($accession_1 && !$accession_2) {
    drupal_goto('tpps/submission/' . $accession_1 . '/compare');
  }
  else {
    drupal_goto('tpps/submission/' . $accession_1 . '/compare/' . $accession_2);
  }
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
  if (empty($accession_1)) {
    drupal_set_message(t('Accession 1 is required.'), 'warning');
    drupal_goto('tpps/submission');
  }
  elseif (empty($accession_2)) {
    drupal_set_message(t('Accession 2 is required.'), 'waring');
    drupal_goto('tpps/submission/' . $accession_1 . '/compare');
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
  $form_state_diff_1 = array_diff_assoc_recursive(
    tpps_load_submission_state($accession_1),
    tpps_load_submission_state($accession_2)
  );
  $form_state_diff_2 = array_diff_assoc_recursive(
    tpps_load_submission_state($accession_2),
    tpps_load_submission_state($accession_1)
  );

  if (empty($form_state_diff_1) && empty($form_state_diff_2)) {
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
        l(t('View Submission'), 'tpps/submission/' . $accession_1 . '/view'),
        l(t('Study at Admin Panel'), 'tpps-admin-panel/' . $accession_1),
      ]),
      $accession_2 . ': ' . implode(', ', [
        l(t('View Submission'), 'tpps/submission/' . $accession_2 . '/view'),
        l(t('Study at Admin Panel'), 'tpps-admin-panel/' . $accession_2),
      ]),
    ],
  ]);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show comparison report.
  $interface_1 = tpps_submission_interface_load($accession_1) ?? [];
  $interface_2 = tpps_submission_interface_load($accession_2) ?? [];

  $interface_diff_1 = array_diff_assoc_recursive($interface_1, $interface_2);
  $interface_diff_2 = array_diff_assoc_recursive($interface_2, $interface_1);

  $dumps = [
    'Submission Interface' => [
      'parsed_1' => tpps_array_dump($interface_diff_1, $sticky = FALSE),
      'parsed_2' => tpps_array_dump($interface_diff_2, $sticky = FALSE),
    ],
    'Form State' => [
      'parsed_1' => tpps_array_dump($form_state_diff_1, $sticky = FALSE),
      'parsed_2' => tpps_array_dump($form_state_diff_2, $sticky = FALSE),
    ],
  ];
  foreach ($dumps as $type => $data) {
    $form[$type] = [
      '#type' => 'fieldset',
      '#title' => t(ucfirst($type)),
      '#collapsed' => FALSE,
      '#collapsible' => FALSE,
    ];
    tpps_submission_compare_show_diff(
      $form[$type],
      $accession_1,
      $accession_2,
      $data['parsed_1'],
      $data['parsed_2']
    );
  }
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  return $form;
}

/**
 * Shows comparison report in 2 columns.
 *
 * @param array $form
 *   Drupal Form API Array.
 * @param string $accession_1
 *   Submission 1 Accession.
 * @param string $accession_2
 *   Submission 2 Accession.
 * @param array $parsed_1
 *   Prepared array. See tpps_array_dump().
 *   Keys are: 'title', 'data' and 'count'.
 * @param array $parsed_2
 *   Prepared array. See tpps_array_dump().
 *   Keys are: 'title', 'data' and 'count'.
 */
function tpps_submission_compare_show_diff(array &$form, $accession_1, $accession_2, array &$parsed_1, array &$parsed_2) {
  if (empty($parsed_1) && empty($parsed_2)) {
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
  tpps_submission_compare_show_diff($form, $accession_1, $accession_2, $parsed_1, $parsed_2);
}
