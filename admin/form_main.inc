<?php

/**
 * @file
 * TPPS Page 1 Settings Form.
 */

/**
 * TPPS Page 1 Settings Form.
 */
function tpps_form_main_settings_form(array $form, array &$form_state) {
  // @TODO Build list of themes dynamically.
  $form['tpps_theme'] = [
    '#type' => 'select',
    '#title' => t('TPPS Theme'),
    '#default_value' => variable_get('tpps_theme', 'default'),
    '#options' => [
      'default' => t('Default Theme'),
      'rachel' => t('Rachel Theme'),
    ],
  ];
  $form['tpps_top_bar'] = [
    '#type' => 'select',
    '#title' => t('Top Bar Type'),
    '#default_value' => variable_get('tpps_top_bar', 'status'),
    '#options' => [
      'status' => t('Status Bar'),
      'navigation' => t('Navigation Bar'),
    ],
    '#description' => t('Status bar has icons which show completed steps.<br />'
      . 'Navigation bar has no icons but allows to go back to completed steps.'
    ),
  ];
  $form['tpps_dark_theme'] = [
    '#type' => 'checkbox',
    '#title' => t('Use Dark Theme'),
    '#default_value' => variable_get('tpps_dark_theme', FALSE),
    '#description' => t('Dark theme only changes backgraund color of the TPPS '
      . 'forms. Enabled by default at dev-server.'),
  ];

  // Submission Export/Import.

  // @TODO Move this settings to 'Submission Settings' page.

  $form['tpps_submission_export_exclude_data'] = [
    '#type' => 'checkbox',
    '#title' => t('Exclude "data" element from Submission Export array.'),
    '#default_value' => variable_get('tpps_submission_export_exclude_data', TRUE),
    '#description' => t('Element "data" of the submission State array '
      . 'contains huge amount of data added during Tripal pipeline processing. '
      . 'Those data is NOT required to import study but slow down processing. '
      . 'Those data could be easily re-generated again but sometimes it '
      . 'could be useful for debugging.'),
  ];
  $form['tpps_submission_export_exclude_tree_info'] = [
    '#type' => 'checkbox',
    '#title' => t('Exclude "data" element from Submission Export array.'),
    '#default_value' => variable_get('tpps_submission_export_exclude_tree_info', TRUE),
    '#description' => t('Element "tree_info" of the submission State array '
      . 'contains huge amount of data added during Tripal pipeline processing. '
      . 'Those data is NOT required to import study but slow down processing. '
      . 'Those data could be easily re-generated again but sometimes it '
      . 'could be useful for debugging.'),
  ];
  $form['#submit'][] = 'tpps_form_main_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Custom form submitter for 'tpps_page_1_settings_form' form.
 */
function tpps_form_main_settings_form_submit(array $form, array &$form_state) {
  // Clear cache on change only but not on each form submission.
  if (
    // CSS theme was changed.
    ($form_state['values']['tpps_theme'] ?? NULL)
    != variable_get('tpps_theme', 'default')
    // Enabled dark theme.
    || ($form_state['values']['tpps_dark_theme'] ?? NULL)
    != variable_get('tpps_dark_theme', FALSE)
  ) {
    cache_clear_all();
  }
}
