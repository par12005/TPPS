<?php

/**
 * @file
 * JS Debug Mode Settings Form.
 */

/**
 * JS Debug Mode Settings Form.
 */
function tpps_js_debug_mode_settings_form(array $form, array &$form_state) {
  $notes = [
    t('Debug Mode in Javascript in TPPS module uses '
      . 'custom dog() (Debug lOG) function which allows enable/disabled '
      . 'colored by type and more sofisticated log messages.'),
    t("WARNING: if featureName not allowed on this page log messages won't "
      . "be shown in developer's console."),
    t('JS Debug Mode allows to avoid commented "console.log()" commands and '
      . 'enable/disable debug messages without changes in code.'),

    '<h3>' . t('Usage examples') . '</h3>',

    '<h4>' . t('To dump variable:') . '</h4>',
    '<code>var featureName = \'jQuery.fn.fileColumnsChange\';',
    'dog(\'Got fid: \' + fid, featureName);</code>',

    '<h4>' . t('To dump array/object:') . '</h4>',
    '<code>dog(variableName, featureName);</code>',

    '<h4>' . t('To temporary and quickly create log message without '
      . 'adding featureName:') . '</h4>',
    "<code>dog(variableName, 'quickDebugMode');</code>",
  ];

  $fieldset = 'js_debug_mode_notes';
  $form[$fieldset] = [
    '#type' => 'fieldset',
    // Extra linebreak required to fix layout issue.
    // @todo Fix CSS rules for fieldsets.
    '#description' => '<br />' . implode('<br />', $notes),
    '#title' => t('JavaScript Debug Mode Notes'),
    '#collapsible' => TRUE,
  ];
  tpps_form_add_fieldset_collapsed_state($form, $form_state, $fieldset);

  $default_value = variable_get('tpps_js_debug_mode');
  $features = tpps_js_debug_mode_get_features_list();
  // Add always checked and disabled option.
  $disabled_key = 'quickDebugMode';
  $default_value[$disabled_key] = $disabled_key;
  $form['#submit'][] = 'tpps_js_debug_mode_settings_form_submit';
  // See tpps_form_js_debug_mode_settings_form_alter() which adds processing
  // for element.
  $form['tpps_js_debug_mode'] = [
    '#type' => 'checkboxes',
    '#title' => t('JS functions/features which supports "JS Debug Mode"'),
    '#description' => t('Special feature "quickDebugMode" was designed for '
      . 'quick debugging. It\'s enabled by default and must NOT be used '
      . 'on regular basis.'),
    '#default_value' => $default_value,
    '#options' => array_combine($features, $features),
  ];

  return system_settings_form($form);
}

/**
 * Disables single option of 'checkboxes' element.
 *
 * @param array $element
 *   Drupal Form API array element.
 */
function tpps_js_debug_mode_settings_form_disable_element(array $element) {
  $disabled_key = 'quickDebugMode';
  foreach (element_children($element) as $key) {
    if ($key == $disabled_key) {
      $element[$key]['#attributes']['disabled'] = 'disabled';
      return $element;
    }
  }
  return $element;
}

/**
 * Form Submitter for 'tpps_js_debug_mode_settings_form' form.
 */
function tpps_js_debug_mode_settings_form_submit(array $form, array &$form_state) {
  $disabled_key = 'quickDebugMode';
  // Since disabled option do not give it's value we need to add it manually.
  // @TODO Leave only one.
  $form_state['input']['tpps_js_debug_mode'][$disabled_key] = $disabled_key;
  $form_state['values']['tpps_js_debug_mode'][$disabled_key] = $disabled_key;
}
