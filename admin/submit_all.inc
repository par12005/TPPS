<?php

/**
 * @file
 * Settings for 'submit_all.php' script.
 */

/**
 * Menu callback. Shows settings form.
 */
function tpps_submit_all_settings_form(array $form, array &$form_state) {
  $form['tpps_submitall_log_cli_show_time'] = [
    '#type' => 'checkbox',
    '#title' => t('Add time to command line log messages'),
    '#default_value' => variable_get('tpps_submit_all_log_cli_show_time', FALSE),
    // @todo Minor. Implement ability to change date format.
    '#description' => t('Time will be added to command line messages only.'
      . '<br />Format: "Y/m/d H:i:s O".'
      . '<br />Example: 2023/11/28 14:04:32 +0200.'
    ),
  ];
  $form['tpps_submitall_skip_gps_request'] = [
    '#type' => 'checkbox',
    '#title' => t('Skip GPS coordinates requests'),
    '#default_value' => variable_get('tpps_submitall_skip_gps_request', FALSE),
    '#description' => t('This requiest do not block processing but take some '
    . 'time. Checking allows to skip this requests and speed-up processing.'
  ];
  $form['tpps_submitall_phenotype_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('Skip GPS coordinates requests'),
    '#default_value' => variable_get(
      'tpps_submitall_phenotype_debug_mode', FALSE
    ),
    '#description' => t('Debug mode allows to call phpenotype processing '
      . 'function from browser (not CLI) and avoid extra processing '
      . 'of the phenotype files which already was added to project.')
  ];

  variable_set('tpps_submitall_phenotype_debug_mode', FALSE);
  return system_settings_form($form);
}
