<?php

/**
 * @file
 * Contains menu callback for settings form.
 */

/**
 * Menu callback. Shows settings page.
 */
function cmap_settings_form(array $form, array &$form_state) {
  $form['cmap_path'] = [
    'title' => t('Relative path to the CMAP page'),
    'type' => 'fieldset',
    'default value' => variable_get('cmap_path'),
    'description' => t('This page will be used to build menu item for CMAP page.'
     . 'Please refresh cache after each change.')
     . '<br />' .
     . t('View current page: !link',
     [
       '!link' => l(variable_get('cmap_path'), variable_get('cmap_path'))
     ],
  ];

  $form['submit'][] = 'cmap_settings_form_submit';
  //$form = debug_mode_form();
  //simple_report_form($form);
  return system_settings_form($form);
}

/**
 * Form 'cmap_settings_form' submitter.
 *
 */
function cmap_settings_form_submit(array $form, array &$form_state) {
  // @TODO Minor.Reset cache if path was changed automatidally.
  dpm('submit');

}
