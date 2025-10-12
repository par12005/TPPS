<?php

/**
 * @file
 * TPPS Misc Settings Form.
 */

/**
 * TPPS Watchdog Cleanup Settings Form.
 */
function tpps_watchdog_cleanup_settings_form(array $form, array &$form_state) {

  $variable_name = 'tpps_watchdog_cleanup_filter';
  $filter_example = [
    [
      'uid' => '0',
      'type' => 'php',
      'message' => 'Cleaned up @count released locks',
      'variables' => [
        'Invalid argument supplied for foreach()',
        '248',
        '/var/www/Drupal/sites/all/modules/file_entity/file_entity.file.inc',
      ],
      'severity' => WATCHDOG_WARNING,
    ],
  ];
  $form['tpps_watchdog_cleanup'] = [
    '#type' => 'checkbox',
    '#title' => t('Remove spam watchdog messages'),
    '#default_value' => variable_get('tpps_watchdog_cleanup'),
    '#description' => t('When enabled predefined rules to remove watchdog-'
      . 'messages will be used. Default: unchecked. '
      . '<br />New rules could be added by code changes or using variable '
      . '"@variable_name" which value is array of arrays.'
      . 'When "variables" key is an array of strings then each must be found '
      . ' to remove watchdog record.'
      . '<br />Example: <pre>@example</pre>',
      [
        '@example' => var_export($filter_example, 1),
        '@variable_name' => $variable_name,
      ]
    ),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Custom Rules.
  $form['custom'] = [
    '#type' => 'fieldset',
    '#title' => t('Custom Rules'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#description' => '<br />' . t('Value of the variable "@variable_name":',
        ['@variable_name' => $variable_name]
    ),
  ];
  $form['custom']['rules'] = [
    '#markup' => '<pre>' . print_r(variable_get($variable_name, []), 1) . '</pre>',
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Hardcoded Rules.
  $form['hardcoded'] = [
    '#type' => 'fieldset',
    '#title' => t('Hardcoded Rules'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#description' => '<br />',
  ];
  $form['hardcoded']['rules'] = [
    '#markup' => '<pre>' . print_r(tpps_watchdog_cleanup_rules(), 1) . '</pre>',
  ];

  return system_settings_form($form);
}
