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

  // Useful links.
  $url = 'https://github.com/googlemaps/js-markerclusterer';
  $docs_url = 'https://developers.google.com/maps/documentation/javascript/marker-clustering?hl=en';
  $form['google_map']['tpps_markercluster_url_type'] = [
    '#type' => 'select',
    '#options' => tpps_markercluster_get_url(),
    '#title' => t('Google Maps JavaScript MarkerClusterer URL'),
    '#default_value' => variable_get('tpps_markercluster_url_type', 'new'),
    '#description' => t('Google Maps JavaScript MarkerClusterer<br />'
      . 'The library creates and manages per-zoom-level clusters for large '
      . 'amounts of markers.<br />New URL: @new_url'
      . '<br />Documentation: !docs_page<br />Project page: !project_page',
      [
        '!project_page' => l($url, $url),
        '!docs_page' => l($docs_url, $docs_url),
        '@new_url' => tpps_markercluster_get_url('new'),
      ]
    ),
  ];
  $id_list = [
    '34cd3326f3006a1f (treegenesdb@gmail.com, live-site)',
    'b3bae3716891ee58 (vlad.savitsky@gmail.com, debug only)',
  ];
  $form['google_map']['tpps_google_map_id'] = [
    '#type' => 'textfield',
    '#title' => t('Google Map Id'),
    '#default_value' => variable_get('tpps_google_map_id'),
    '#description' => l(t('Get Google Map Id'),
      'https://developers.google.com/maps/documentation/get-map-id'
    ) . theme('item_list', ['items' => $id_list]),
  ];
  $form['google_map']['tpps_maps_api_key'] = [
    '#type' => 'textfield',
    '#title' => t('TPPS Google Maps API Key'),
    '#default_value' => variable_get('tpps_maps_api_key', NULL),
  ];
  tpps_add_css_js('suggestion', $form);
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
