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
    '#title' => t('Relative path to the CMAP page'),
    '#type' => 'textfield',
    '#default_value' => variable_get('cmap_path'),
    '#description' => t('This path will be used for CMAP page.') . '<br />'
      . t('After change new menu item will be added automatically '
      . '(no need to clear cache).') . '<br />' . t('View current page: !link',
      ['!link' => l(variable_get('cmap_path'), variable_get('cmap_path'))]),
  ];

  $form['#submit'][] = 'cmap_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Form 'cmap_settings_form' submitter.
 */
function cmap_settings_form_submit(array $form, array &$form_state) {
  if (
    $form_state['values']['cmap_path']
    != $form_state['complete form']['cmap_path']['#default_value']
  ) {
    // Value was changed. Clear menu cache to add new path.
    variable_set('menu_rebuild_needed', TRUE);
  }
}
