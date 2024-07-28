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
  $form['google_map'] = [
    '#type' => 'fieldset',
    '#title' => t('Google Map'),
    '#collapsible' => TRUE,
  ];
  // Pager fix.
  $url = 'https://github.com/googlemaps/js-markerclusterer';
  $docs_url = 'https://developers.google.com/maps/documentation/javascript/marker-clustering?hl=en';
  $form['google_map']['tpps_markercluster_url'] = [
    '#type' => 'textfield',
    '#title' => t('Google Maps JavaScript MarkerClusterer URL'),
    '#default_value' => variable_get('tpps_markercluster_url',
      'https://unpkg.com/@googlemaps/markerclusterer/dist/index.min.js'
    ),
    '#description' => t('Google Maps JavaScript MarkerClusterer<br />'
      . 'The library creates and manages per-zoom-level clusters for large '
      . 'amounts of markers.<br />'
      . 'Documentation: !docs_page<br />'
      . 'Project page: !project_page<br />'
      // @TODO Remove after switch to new URL.
      . '<br />Suggestions:'
      . '<br />Depricated URL: https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/markerclusterer.js'
      . '<br />Current URL: https://unpkg.com/@googlemaps/markerclusterer/dist/index.min.js',
      [
        '!project_page' => l($url, $url),
        '!docs_page' => l($docs_url, $docs_url),
      ]
    ),
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
