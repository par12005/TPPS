<?php

/**
 * @file
 * Settings for 'submit_all.php' script.
 */

/**
 * Menu callback. Shows settings form.
 */
function tpps_submit_all_settings_form(array $form, array &$form_state) {
  $form['common'] = [
    '#type' => 'fieldset',
    '#title' => t('Common Settings'),
  ];
  $form['common']['tpps_submitall_skip_gps_request'] = [
    '#type' => 'checkbox',
    '#title' => t('Skip GPS coordinates requests'),
    '#default_value' => variable_get('tpps_submitall_skip_gps_request', FALSE),
    '#description' => t('This requiest do not block processing but take some '
    . 'time. Checking allows to skip this requests and speed-up processing.'),
  ];

  $form['common']['tpps_submitall_phenotype_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('Phenotype Debug Mode'),
    '#default_value' => variable_get(
      'tpps_submitall_phenotype_debug_mode', FALSE
    ),
    '#description' => t('Debug mode allows to call phpenotype processing '
      . 'function from browser (not CLI) and avoid extra processing '
      . 'of the phenotype files which already was added to project.')
  ];
  $form['common']['tpps_submitall_log_tripal_suppress_cli'] = [
    '#type' => 'checkbox',
    '#title' => t('Suppres output of Tripal Log messages to CLI'),
    '#default_value' => variable_get('tpps_submitall_log_tripal_suppress_cli', TRUE),
    '#description' => t('Tripal log messages usually duplicate CLI messages '
      . 'and could be disabled.')
  ];
  $form['common']['tpps_submitall_run_nextflow'] = [
    '#type' => 'checkbox',
    '#title' => t('Allow to run nextflow'),
    '#default_value' => variable_get('tpps_submitall_run_nextflow'),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Logging.
  $severity_options = [
    TRIPAL_CRITICAL => t('2. Critical - Critical conditions'),
    TRIPAL_ERROR => t('3. Error - Error conditions'),
    TRIPAL_WARNING => t('4. Warning - Warning conditions'),
    TRIPAL_NOTICE => t('5. Notice - Normal but significant conditions'),
    TRIPAL_INFO => t('6. Info - (default) Informational messages'),
    TRIPAL_DEBUG => t('7. Debug - Debug-level messages'),
  ];
  $list = [
    'log_cli' => 'CLI Log',
    'log_file' => 'Log to File',
    'log_tripal' => 'Tripal Log',
  ];

  // To allow search dynamically build variable names:
  // 'tpps_submitall_log_cli_severity_level'.
  // 'tpps_submitall_log_cli_show_time'.
  // 'tpps_submitall_log_cli_time_format'.
  // 'tpps_submitall_log_cli_colorful'.
  // 'tpps_submitall_log_file_severity_level'.
  // 'tpps_submitall_log_file_show_time'.
  // 'tpps_submitall_log_file_time_format'.
  // 'tpps_submitall_log_tripal_severity_level'.
  // 'tpps_submitall_log_tripal_show_time'.
  // 'tpps_submitall_log_tripal_time_format'.
  foreach ($list as $key => $value) {
    $params = ['@log_type' => $value];
    $form[$key] = [
      '#type' => 'fieldset',
      '#title' => t('@log_type Settings', $params),
      '#collapsible' => TRUE,
    ];
    tpps_form_add_fieldset_collapsed_status($form, $form_state, $key);
    if ($key == 'log_cli') {
      // Only CLI supports colorful output.
      $form[$key]['tpps_submitall_' . $key . '_colorful'] = [
        '#type' => 'checkbox',
        '#title' => t('Color depending on severity level', $params),
        '#default_value' => variable_get('tpps_submitall_' . $key . '_colorful', FALSE),
      ];
    }
    $form[$key]['tpps_submitall_' . $key . '_severity_level'] = [
      '#type' => 'select',
      '#title' => t('Severity Level', $params),
      '#options' => $severity_options,
      '#default_value' => variable_get(
        'tpps_submitall_' . $key . '_severity_level', 7
      ),
      '#required' => TRUE,
      '#description' => t('Default: debug (all messages will be logged).'
        . '<br />Will be shown messages with severity levels from the top '
        . 'to selected level. <br />Critical messages will always be shown.'
      ),
    ];
    $form[$key]['tpps_submitall_' . $key . '_show_time'] = [
      '#type' => 'checkbox',
      '#title' => t('Prepend Timestamp to Message Text', $params),
      '#default_value' => variable_get('tpps_submitall_' . $key . '_show_time', FALSE),
    ];
    $form[$key]['tpps_submitall_' . $key . '_time_format'] = [
      '#type' => 'textfield',
      // @TODO Must be required if checkbox was checked.
      '#title' => t('Timestamp format', $params),
      '#default_value' => variable_get('tpps_submitall_' . $key . '_time_format', FALSE),
      '#description' => t("See php's date() function formats. "
        . '<br />Default format: "Y/m/d H:i:s O".'
        . "<br />Example: 2023/11/28 14:04:32 +0200."
      ),
    ];
  }
  return system_settings_form($form);
}
