<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'includes/common');
//module_load_include('inc', 'tpps', 'includes/submissions');
//module_load_include('inc', 'tpps', 'includes/submission_file');

// @TODO Minor. Clear part table on change of part.
/**
 * TPPS Study View Form.
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
  // Show with line numbers.
  if (!empty($accession)) {
    module_load_include('inc', 'tpps', 'includes/submissions');
    if ($state = tpps_submission_get_state($accession)) {
      $part = $form_state['input']['tpps_submission_view_part'] ?? NULL;
      // Always show stings and allow to choose arrays.
      foreach (array_keys($state) as $key) {
        if (is_array($state[$key])) {
          $part_list[$key] = $key . ' (' . count($state[$key]) . ')';
        }
        else {
          $simple[] = $key;
        }
      }
      foreach ($simple as $line => $key) {
        $simple_rows[] = [
          $line,
          check_plain($key),
          '<pre>' . check_plain($state[$key]) . '</pre>',
        ];
      }
      $form['simple'] = [
        '#type' => 'fieldset',
        '#title' => t('Submission State'),
        '#collapsible' => TRUE,
        '#collapsed' => (bool) $part,
      ];
      $form['simple']['simple_table'] = [
        '#markup' => theme('table', [
          'header' => ['#', 'Key', 'Value'],
          'rows' => $simple_rows,
          'attributes' => ['class' => ['submission-view']],
        ]),
      ];

      ksort($part_list);
      // List of items which are arrays.
      $form['tpps_submission_view_part'] = [
        '#type' => 'select',
        '#title' => 'View submission state parts',
        '#default_value' => $part,
        '#options' => array_merge($part_list, ['all' => t('ALL')]),
      ];
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Parts
      $state_part = ($part == 'all') ? $state : ($state[$part] ?? []);
      $state_array = preg_split("/\r\n|\n|\r/", var_export($state_part, 1));
      foreach ($state_array as $line => $content) {
        // @TODO Minor. Highlight intends.
        $rows[] = [$line, '<pre>' . check_plain($content) . '</pre>'];
      }
      $form['part'] = [
        '#type' => 'fieldset',
        '#title' => $part,
        '#collapsible' => TRUE,
      ];
      $form['part']['table'] = [
        '#markup' => theme('table', [
          'header' => ['Line', 'Content'],
          'rows' => $rows,
        ]),
      ];
      $form['footer_view'] = [
        '#type' => 'submit',
        '#value' => t('View'),
        '#submit' => ['tpps_submission_view_form_submit'],
      ];
      $form['footer_reset'] = [
        '#type' => 'submit',
        '#value' => t('Reset'),
        '#submit' => ['tpps_submission_view_form_reset'],
      ];

    }
    else {
      drupal_set_message('No submission found.', 'error');
    }
  }
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
  //// Debug Report.
  //$part_list = ($form_state['input']['tpps_import_debug_state'] ?? []);
  //foreach ($part_list as $key => $value) {
  //  if (empty($value)) {
  //    continue;
  //  }
  //  elseif (!empty($state[$key])) {
  //    dpm(print_r($state[$key], 1), $value);
  //  }
  //  else {
  //    dpm('Submission State part "' . $value . '" is empty.');
  //    continue;
  //  }
  //}
  $form_state['rebuild'] = 1;
}
