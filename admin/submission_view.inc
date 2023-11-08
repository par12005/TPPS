<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'includes/common');
//module_load_include('inc', 'tpps', 'includes/submissions');
//module_load_include('inc', 'tpps', 'includes/submission_file');

/**
 * Menu callback. TPPS Study View Form.
 */
function tpps_submission_view_form(array $form, array &$form_state) {
  $accession = (($form_state['input']['op'] ?? NULL) == t('Reset')
    ? '' : ($form_state['input']['tpps_submission_view_accession'] ?? '')
  );
  $form['tpps_submission_view_accession'] = [
    '#type' => 'select',
    '#title' => t('Accession'),
    '#options' => tpps_submission_get_accession_list(),
    '#default_value' => $accession,
    '#required' => TRUE,
  ];
  $form['action_view'] = [
    '#type' => 'submit',
    '#value' => t('View'),
    '#submit' => ['tpps_submission_view_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_submission_view_form_reset'],
  ];
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_submission_view_form_reset() {
  $form_state['input']['tpps_submission_view_accession'] = '';
  $form_state['values']['tpps_submission_view_accession'] = '';
  drupal_goto('admin/config/tpps/submission/view');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_view_form_submit(array $form, array &$form_state) {
  $accession = ($form_state['values']['tpps_submission_view_accession'] ?? '');
  drupal_goto('admin/config/tpps/submission/view/' . $accession);
}

/**
 * Menu callback. Shows certain submission view page.
 *
 * @param string $accession
 *   Submission accession. Format: 'TGDRxxxx'.
 *
 * @return string
 *   Returns rendered form. We use forms to have collapsible fieldsets.
 */
function tpps_submission_view_page($accession) {
  if (empty($accession)) {
    drupal_set_message('Empty accession.', 'error');
    drupal_goto('admin/config/tpps/submission/view');
  }

  module_load_include('inc', 'tpps', 'includes/submissions');
  if (empty($state = tpps_submission_get_state($accession))) {
    drupal_set_message('No submission found.', 'error');
    drupal_goto('admin/config/tpps/submission/view');
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $form = drupal_get_form('tpps_show_dump_form', $state);
  return drupal_render($form);
}

/**
 * Creates a well-formatted array dump with fieldsets.
 *
 * Good for huge arrays. For small use dpm() or similar.
 *
 * If we just use drupal_render($form) then:
 * 1. Button 'Back' doesn't work.
 * 2. Fieldsets couldn't be collapsed.
 *
 * @param array $form
 *   Drupal Form API Array.
 * @param array $form_state
 *   Drupal Form State.
 * @param array $array
 *   Array to be shown.
 *
 * @return array
 *   Returns Drupal Form API Array.
 */
function tpps_show_dump_form(array $form, array $form_state, array $array) {
  $form['header_back'] = [
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => ['tpps_submission_view_form_reset'],
  ];
  if (empty($array)) {
    drupal_set_message('No data to show', 'error');
    return [];
  }
  // Get list of sub-arrays ($part list)
  foreach (array_keys($array) as $key) {
    if (is_array($array[$key])) {
      $part_list[$key] = count($array[$key]);
    }
    else {
      $plain[] = $key;
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show plain elements.
  foreach ($plain as $line => $key) {
    $simple_rows[] = [
      [
        'data' => $line,
        'style' => 'padding: 0 10px; font-weight: bold;'
      ],
      [
        'data' => check_plain($key),
        'style' => 'padding: 0 10px; font-weight: bold;'
      ],
      [
        'data' => check_plain(var_export($array[$key], 1)),
        'style' => 'padding: 0 10px;',
      ],
    ];
  }
  $form['plain'] = [
    '#type' => 'fieldset',
    '#title' => t('Plain Items'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    'plain_items' => [
      '#markup' => theme('table', [
        'header' => ['#', t('Key'), t('Value')],
        'rows' => $simple_rows,
      ]),
    ],
  ];

  ksort($part_list);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Sub-arrays.
  $header = [
    ['data' => '#', 'style' => 'font-weight: bold;'],
    ['data' => t('Data'), 'style' => 'width:100%;'],
  ];
  foreach ($part_list as $key => $count) {
    $rows = [];
    $state_part = $array[$key] ?? [];
    $state_array = preg_split("/\r\n|\n|\r/", var_export($state_part, 1));
    if ($state_part) {
      foreach ($state_array as $line => $content) {
        $rows[] = [
          [
            'data' => '<strong>' . $line . '</strong>',
            'style' => 'padding: 0 10px;',
          ],
          [
            'data' => tpps_string_format_indent($content),
            'style' => 'padding: 0 10px;',
          ],
        ];
      }
      $form[$key . '_container'] = [
        '#type' => 'fieldset',
        '#title' => $key . ' (' . $count . ')',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        $key => [
          '#markup' => theme('table', ['header' => $header, 'rows' => $rows]),
        ],
      ];
    }
  }
  $form['footer_back'] = [
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => ['tpps_submission_view_form_reset'],
  ];
  return $form;
}
