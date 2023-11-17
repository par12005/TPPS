<?php

/**
 * @file
 * Settings for 'submit_all.php' script.
 */

/**
 * Menu callback. Shows settings form.
 */
function tpps_submit_all_settings_form(array $form, array &$form_state) {
  $form['tpps_submit_all_dump_report_to_file'] = [
    '#type' => 'checkbox',
    '#title' => t('Dump Report to File'),
    '#default_value' => variable_get('tpps_submit_all_dump_report_to_file', FALSE),
  ];
  return system_settings_form($form);
}
