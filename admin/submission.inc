<?php

/**
 * @file
 * TPPS Submission Tools Settings Form.
 */

/**
 * TPPS Submission Tools Settings Form.
 */
function tpps_submission_settings_form(array $form, array &$form_state) {
  $form['tpps_submission_secret_key'] = [
    '#type' => 'textfield',
    '#title' => t('TPPS Submission Secret Key'),
    '#default_value' => variable_get('tpps_submission_secret_key'),
    '#description' => t('This key must be sent in POST requests to export '
      . 'Submission in JSON format.'),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Common Settings.
  $form['common'] = [
    '#type' => 'fieldset',
    '#title' => t('Common Settings'),
  ];
  $name_list = [
    [
      'name' => 'tpps_submission_log_save_event',
      'title' => 'Log each submission save event',
      'description' => '',
    ],
  ];
  foreach ($name_list as $item) {
    $form['common'][$item['name']] = [
      '#type' => 'checkbox',
      '#title' => t($item['title']),
      // Note: There is no default value to avoid misunderstanding when value
      // shown but not actually set.
      '#default_value' => variable_get($item['name']),
      '#description' => t($item['description'] ?? NULL),
    ];
  }
  // For searching:
  // 'tpps_dump_max_len_state',
  // 'tpps_dump_max_len_sharedState'.
  foreach (['sharedState', 'state'] as $data_type) {
    $form['common']['tpps_dump_max_len_' . $data_type] = [
      '#type' => 'textfield',
      '#title' => t('"@type" dump max length, chars', ['@type' => $data_type]),
      '#default_value' => variable_get('tpps_dump_max_len_' . $data_type, 300000),
      '#description' => t('Default is "300 000". '
        . "<br />Huge dump lenght: 5M."
        . "<br />Used on Submission View page. When dump's length exceeds this "
        . "limit then it will be shown as a flat dump instead of HTML tables in "
        . "fieldsets"),
    ];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission Export/Import.
  $form['export'] = [
    '#type' => 'fieldset',
    '#title' => t('Submission Export'),
  ];
  $form['export']['tpps_submission_export_exclude_data'] = [
    '#type' => 'checkbox',
    '#title' => t('Exclude "data" element from Submission Export array.'),
    '#default_value' => variable_get('tpps_submission_export_exclude_data'),
    '#description' => t('Element "data" of the submission State array '
      . 'contains huge amount of data added during Tripal pipeline processing. '
      . 'Those data is NOT required to import study but slow down processing. '
      . 'Those data could be easily re-generated again but sometimes it '
      . 'could be useful for debugging.'),
  ];
  $form['export']['tpps_submission_export_exclude_tree_info'] = [
    '#type' => 'checkbox',
    '#title' => t('Exclude "tree_info" element from Submission Export array.'),
    '#default_value' => variable_get('tpps_submission_export_exclude_tree_info'),
    '#description' => t('Element "tree_info" of the submission State array '
      . 'contains huge amount of data added during Tripal pipeline processing. '
      . 'Those data is NOT required to import study but slow down processing. '
      . 'Those data could be easily re-generated again but sometimes it '
      . 'could be useful for debugging.'),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission Cache.
  $form['cache'] = [
    '#type' => 'fieldset',
    '#title' => t('Submission Cache'),
  ];
  $form['cache']['tpps_submission_cache_log_single_update'] = [
    '#type' => 'checkbox',
    '#title' => t('Log each single Submission update.'),
    '#default_value' => variable_get('tpps_submission_cache_log_single_update'),
    '#description' => t('Log to watchdog each time single Submission cache '
      . 'was updated. Those updates does not take a lot of time but logging '
      . 'allows to find multiple calls of the same code.'),
  ];
  $form['cache']['tpps_submission_cache_log_full_update'] = [
    '#type' => 'checkbox',
    '#title' => t('Log when whole Submission Cache updated.'),
    '#default_value' => variable_get('tpps_submission_cache_log_full_update'),
    '#description' => t('Recreation of the SubmissionCache takes about 1 '
      . 'minute. Log each time cache was cleared and rebuilded from scratch.'),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission Version.
  $form['version'] = [
    '#type' => 'fieldset',
    '#title' => t('Submission Version'),
  ];
  $name_list = [
    [
      'name' => 'tpps_submission_version_log_downgrade',
      'title' => 'Log downgrade of the Submission version',
      'description' => '',
    ],
    [
      'name' => 'tpps_submission_version_log_upgrade',
      'title' => 'Log upgrade of the Submission version',
      'description' => '',
    ],
    [
      'name' => 'tpps_submission_version_switch_values',
      'title' => 'Switch values when target field not empty',
      'description' => 'When checked and "move" array modification have to '
        . 'update field which has value it will be swiched with source field '
        . 'value to avoid data loss.',
    ],
  ];
  foreach ($name_list as $item) {
    $form['version'][$item['name']] = [
      '#type' => 'checkbox',
      '#title' => t($item['title']),
      // Note: There is no default value to avoid misunderstanding when value
      // shown but not actually set.
      '#default_value' => variable_get($item['name']),
      '#description' => t($item['description'] ?? NULL),
    ];
  }
  return system_settings_form($form);
}
