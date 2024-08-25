<?php

/**
 * @file
 * TPPS Google Maps Settings Form.
 */

/**
 * TPPS Google Maps Settings Form.
 */
function tpps_google_maps_settings_form(array $form, array &$form_state) {
  $id_list = [
    '34cd3326f3006a1f (treegenesdb@gmail.com, live-site)',
    'b3bae3716891ee58 (vlad.savitsky@gmail.com, debug only)',
  ];
  $form['tpps_google_map_id'] = [
    '#type' => 'textfield',
    '#title' => t('Google Map Id'),
    '#default_value' => variable_get('tpps_google_map_id'),
    '#description' => l(t('Get Google Map Id'),
      'https://developers.google.com/maps/documentation/get-map-id'
    ) . theme('item_list', ['items' => $id_list]),
  ];
  $form['tpps_maps_api_key'] = [
    '#type' => 'textfield',
    '#title' => t('TPPS Google Maps API Key'),
    '#default_value' => variable_get('tpps_maps_api_key', NULL),
  ];
  return system_settings_form($form);
}
