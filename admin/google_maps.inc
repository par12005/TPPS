<?php

/**
 * @file
 * TPPS Google Maps Settings Form.
 */

/**
 * TPPS Google Maps Settings Form.
 */
function tpps_google_maps_settings_form(array $form, array &$form_state) {
  $form['tpps_google_map_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('TPPS Google Maps Debug Mode'),
    '#default_value' => variable_get('tpps_google_map_debug_mode', FALSE),
    '#description' => t("When enabled then more messages in browser's Developer"
      . " Tool Console will be shown."),
  ];

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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Zoom settings.
  $form['zoom'] = [
    '#type' => 'fieldset',
    '#title' => t('Zoom Settings'),
  ];
  $form['zoom']['tpps_google_map_zoom_smooth_timeout'] = [
    '#type' => 'textfield',
    '#title' => t('Zoom Smooth Timeout, msec'),
    '#default_value' => variable_get('tpps_google_map_zoom_smooth_timeout'),
    '#description' => t('Timeout is used to have smooth zoom when map '
      . 'appears for the first time. Minimum suitable value is 80 msec.'
      . '<br />Default value is 100 msec. 200 msec is too slow.'
    ),
  ];
  $form['zoom']['tpps_google_map_zoom_default_level'] = [
    '#type' => 'select',
    '#options' => range(0, 22),
    '#title' => t('Zoom Default level'),
    '#description' => t('Default Zoom Level will be used to initialize map.'
     . '<br />Levels are from 0 (world) to 22 (street).'
     . '<br />Suggested default level is 3.'),
    '#default_value' => variable_get('tpps_google_map_zoom_default_level'),
  ];
  $form['zoom']['tpps_google_map_zoom_custom_level'] = [
    '#type' => 'select',
    '#options' => range(0, 22),
    '#title' => t('Zoom Custom Level'),
    '#description' => t('Custom Zoom Level will be used when all markers has '
     . 'the same coordinates and we need to show cluster with number of items '
     . 'instead of single marker with big zoom.'
     . '<br />Levels are from 0 (world) to 22 (street).'
     . '<br />Suggested custom level is 14 but number of items in cluster is shown up to 17 level.'),


    '#default_value' => variable_get('tpps_google_map_zoom_custom_level'),
  ];

  return system_settings_form($form);
}
