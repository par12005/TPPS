<?php

/**
 * @file
 * Shows code to manually export submission when automatic export can't be used.
 */

/**
 * TPPS Submission Manual Export Form.
 */
function tpps_submission_manual_export_form(array $form, array &$form_state) {
  $input = &$form_state['input'];
  $accession = $input['tpps_manual_export_accession'] ?? '';
  $show_report = (int) ($input['tpps_manual_export_show_file_search_report'] ?? 0);

  $form['tpps_manual_export_accession'] = [
    '#type' => 'textfield',
    '#title' => t('Accession'),
    '#default_value' => $accession,
    '#required' => TRUE,
  ];
  $form['tpps_manual_export_show_file_search_report'] = [
    '#type' => 'checkbox',
    '#title' => 'Show File Search Report?',
    '#default_value' => $show_report,
  ];
  if (!empty($accession)) {
    module_load_include('inc', 'tpps', 'admin/submission_export');
    module_load_include('inc', 'tpps', 'includes/submissions');
    module_load_include('inc', 'tpps', 'includes/submission_file');
    module_load_include('inc', 'tpps', 'includes/form');
    $function_code = '';
    $function_list = [
      'tpps_submission_get_state',
      'tpps_form_search_field',
      'tpps_form_get_organism_number',
      'tpps_submission_file_get_parents',
      'tpps_submission_export_get_files_info',
      'tpps_submission_export_file_report',
      'tpps_submission_file_get_id_list',
      'tpps_submission_export_fixes',
    ];
    foreach ($function_list as $function_name) {
      if (function_exists($function_name)) {
        $function_code .= "\n" . tpps_get_function_code($function_name);
      }
      else {
        drupal_set_message(
          t('Function "<strong>@function_name()</strong>" not found.',
          ['@function_name' => $function_name]), 'error'
        );
      }
    }
    // Build the code.
    $code = <<<EOT
\$accession = '$accession';
\$show_report = $show_report;

$function_code

\$cache_reset = REQUEST_TIME;
\$state = tpps_submission_get_state(\$accession);
tpps_submission_export_fixes(\$state);
// Files.
\$fid_list = tpps_submission_file_get_id_list(\$state, \$show_report);
\$files = tpps_submission_export_get_files_info(\$fid_list);
tpps_submission_export_file_report(\$fid_list, \$files);
\$state['tpps_export_files'] = \$files;

drupal_set_message(
  '<div class="form-textarea-wrapper resizable
    resizable-textarea"><textarea id="edit-tpps-export-state" cols="200"
    rows="20" class="form-textarea" style="width: 100%; background: lightgray;">'
  . var_export(\$state, 1)
  . '</textarea></div>'
  . '<script type="text/javascript" src="https://tgwebdev.cam.uchc.edu/sites/all/modules/TGDR/js/tpps_submission_export.js?\$cache_reset"></script>'
);
// dpm(var_export(tpps_submission_get_state(\$accession), 1));
EOT;
    $live_url = 'https://treegenesdb.org/devel/php';
    $dev_url = 'https://tgwebdev.cam.uchc.edu/devel/php';
    $form['tpps_manual_export_code'] = [
      '#type' => 'textarea',
      '#title' => t('PHP Code to run'),
      '#value' => $code,
      '#rows' => 10,
      '#resizable' => TRUE,
      '#description' => theme('item_list', [
        'items' => [l($live_url, $live_url), l($dev_url, $dev_url)]
      ]),
    ];
  }
  $form['action_export'] = [
    '#type' => 'submit',
    '#value' => t('Get Code'),
    '#submit' => ['tpps_submission_manual_export_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_submission_manual_export_form_reset'],
  ];
  tpps_add_css_js($form, 'submission_manual_export');
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_submission_manual_export_form_reset() {
  drupal_goto('admin/config/tpps/migration/manual_export');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_manual_export_form_submit(array $form, array &$form_state) {
  $form_state['rebuild'] = 1;
}

/**
 * Get function's definition.
 *
 * @param string $function
 *   Name of the function.
 *
 * @return string
 *   Returns full function's definition or NULL if function not exists.
 */
function tpps_get_function_code($function) {
  if (!function_exists($function)) {
    return NULL;
  }
  $func = new ReflectionFunction($function);
  $filename = $func->getFileName();
  // It's actually - 1, otherwise you wont get the function() block.
  $start_line = $func->getStartLine() - 1;
  $end_line = $func->getEndLine();
  $length = $end_line - $start_line;

  $source = file($filename);
  $body = implode("", array_slice($source, $start_line, $length));
  return $body;
}
