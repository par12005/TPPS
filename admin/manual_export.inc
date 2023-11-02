<?php

/**
 * @file
 * Shows code to manually export study when automatic export can't be used.
 */

/**
 * TPPS Study Manual Export Form.
 */
function tpps_study_manual_export_form(array $form, array &$form_state) {
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
    module_load_include('inc', 'tpps', 'admin/export');
    module_load_include('inc', 'tpps', 'includes/form');
    $function_code = '';
    $function_list = [
      'tpps_study_export_get_state',
      'tpps_form_search_field',
      'tpps_form_get_organism_number',
      'tpps_study_export_get_file_parents',
      'tpps_export_show_file_search_report',
      'tpps_study_export_show_method_report',
      'tpps_study_export_get_files_info',
    ];
    foreach ($function_list as $function_name) {
      $function_code .= "\n" . tpps_get_function_code($function_name);
    }
    // Build the code.
    $code = <<<EOT
\$accession = '$accession';
variable_set('tpps_export_show_file_search_report', $show_report);

$function_code

drupal_set_message(
  '<div class="form-textarea-wrapper resizable
   resizable-textarea"><textarea cols="200" rows="20" class="form-textarea"
   style="width: 100%; background: lightgray;">'
  . var_export(tpps_study_export_get_state(\$accession), 1)
  . '</textarea></div>'
);
// dpm(var_export(tpps_study_export_get_state(\$accession), 1));
EOT;
    $form['tpps_manual_export_code'] = [
      '#type' => 'textarea',
      '#title' => t('PHP Code to run'),
      '#value' => $code,
      '#rows' => 10,
      '#resizable' => TRUE,
    ];
  }
  $form['action_export'] = [
    '#type' => 'submit',
    '#value' => t('Get Code'),
    '#submit' => ['tpps_study_manual_export_form_submit'],
  ];
  $form['action_reset'] = [
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => ['tpps_study_manual_export_form_reset'],
  ];
  return $form;
}

/**
 * Resets TPPS Export Form.
 */
function tpps_study_manual_export_form_reset() {
  drupal_goto('admin/config/tpps/migration/manual_export');
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_study_manual_export_form_submit(array $form, array &$form_state) {
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
