<?php

/**
 * @file
 * Settings for 'submit_all.php' script.
 */

/**
 * Menu callback. Shows settings form.
 */
function tpps_submit_all_settings_form(array $form, array &$form_state) {
  $form['tpps_submit_all_log_cli_show_time'] = [
    '#type' => 'checkbox',
    '#title' => t('Add time to command line log messages'),
    '#default_value' => variable_get('tpps_submit_all_log_cli_show_time', FALSE),
    // @todo Minor. Implement ability to change date format.
    '#description' => t('Time will be added to command line messages only.'
      . '<br />Format: "Y/m/d H:i:s O".'
      . '<br />Example: 2023/11/28 14:04:32 +0200.'
    ),
  ];
  return system_settings_form($form);
}
