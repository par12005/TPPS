<?php

/**
 * @file
 * TPPS Misc Settings Form.
 */

/**
 * TPPS Misc Settings Form.
 */
function tpps_misc_settings_form(array $form, array &$form_state) {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // JS Debug Mode.
  $form['js_debug_mode'] = [
    '#type' => 'fieldset',
    '#description' => t('Debug Mode in Javascript used custom dog() function '
      . 'which allows enable/disabled colored and more useful log messages.'
      . '<br />Usage example: '
      . '<code>'
        . '<br />  var featureName = \'jQuery.fn.fileColumnsChange\';'
        . '<br />  dog(\'Got fid: \' + fid, featureName);'
      . '</code>'
    ),
    '#title' => t('JavaScript Debug Mode'),
    '#collapsible' => TRUE,
  ];
  $features = tpps_js_debug_mode_get_features_list();
  $form['js_debug_mode']['tpps_js_debug_mode'] = [
    '#type' => 'checkboxes',
    '#title' => t('JS functions/features which supports "JS Debug Mode"'),
    '#description' => t('Special feature "quickDebugMode" was designed for quick '
    . 'debugging. It\'s enabled by default and must NOT be used on regular basis.'),
    '#default_value' => variable_get('tpps_js_debug_mode'),
    '#options' => array_combine($features, $features),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Theme Settings.
  $form['theme_settings'] = [
    '#type' => 'fieldset',
    '#title' => t('Theme Settings'),
    '#collapsible' => TRUE,
  ];
  // Pager fix.
  $form['theme_settings']['tpps_theme_fix_pager'] = [
    '#type' => 'checkbox',
    '#title' => t('Restore original position of "Next" button of pager'),
    '#default_value' => variable_get('tpps_theme_fix_pager'),
    '#description' => t("Theme 'Dawn' moves 'Next' button of pager to the "
      . "very right position (after 'Last' button. <br />When checked "
      . "original position (before 'Last' button) will be restored."),
  ];
  return system_settings_form($form);
}
