<?php

/**
 * @file
 * Define the helper functions for the Plant Accession page.
 */

/**
 * This function creates fields describing the study location.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_study_location(array &$form, array &$form_state) {

  $form['study_location'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">STUDY LOCATION:</div>'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#description' => t('This should be the location(s) of your common garden, plantation, etc.'),
    '#prefix' => '<div id="common-garden-loc">',
    '#suffix' => '</div>',
  );

  $form['study_location']['type'] = array(
    '#type' => 'select',
    '#title' => t('Coordinate Projection: *'),
    '#options' => array(
      0 => t('- Select -'),
      1 => t('WGS 84'),
      3 => t('NAD 83'),
      4 => t('ETRS 89'),
      2 => t('Custom Location (brief description)'),
    ),
    '#ajax' => array(
      'callback' => 'tpps_update_locations',
      'wrapper' => 'common-garden-loc',
    ),
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('right'),
      'title' => array('Please select a Coordinate Projection, or select "Custom Location", to enter a custom study location'),
    ),
  );

  $type = tpps_get_ajax_value($form_state, array(
    'study_location',
    'type',
  ), NULL);

  $field = array(
    '#type' => 'textfield',
    '#title' => 'Location !num: *',
  );

  if ($type != 2) {
    $field['#description'] = t('Accepted formats: <br>Degrees Minutes Seconds: 41° 48\' 27.7" N, 72° 15\' 14.4" W<br>Degrees Decimal Minutes: 41° 48.462\' N, 72° 15.24\' W<br>Decimal Degrees: 41.8077° N, 72.2540° W<br>');
  }

  tpps_dynamic_list($form, $form_state, 'locations', $field, array(
    'label' => 'Location',
    'title' => 'Study Location(s):',
    'parents' => array('study_location'),
    'callback' => 'tpps_update_locations',
    'default' => 1,
    'wrapper' => 'common-garden-loc',
    'substitute_fields' => array(
      '#title',
    ),
  ));

  $form['study_location']['locations']['#states'] = array(
    'invisible' => array(
      ':input[name="study_location[type]"]' => array('value' => '0'),
    ),
  );

  if ($type != 2 and $type != 0) {
    $form['study_location']['map-button'] = array(
      '#type' => 'button',
      '#title' => 'Click here to update map',
      '#value' => 'Click here to update map',
      '#button_type' => 'button',
      '#name' => 'study_locations_map_button',
      '#executes_submit_callback' => FALSE,
      '#prefix' => '<div id="study_location_map">',
      '#suffix' => '</div>',
      '#ajax' => array(
        'callback' => 'tpps_study_location_map_ajax',
        'wrapper' => 'study_location_map',
      ),
    );
  }

  $locs = tpps_get_ajax_value($form_state, array('study_location', 'locations'), NULL);

  if ($form_state['triggering_element']['#name'] == 'study_locations_map_button' and $type != 2 and !empty($locs) and !empty($locs['number'])) {
    $coords = array();
    $valid_coords = TRUE;
    for ($i = 1; $i <= $locs['number']; $i++) {
      if (empty($locs[$i])) {
        $valid_coords = FALSE;
        drupal_set_message(t('Location %num is required', array('%num' => $i)), 'error');
        continue;
      }
      $raw_coord = $locs[$i];
      $standard_coordinate = tpps_standard_coord($raw_coord);
      if ($standard_coordinate) {
        $parts = explode(',', $standard_coordinate);
        $coords[] = array(
          "Location $i",
          $parts[0],
          $parts[1],
        );
        continue;
      }
      $valid_coords = FALSE;
      drupal_set_message(t('Location %num: Invalid coordinates', array('%num' => $i)), 'error');
    }

    if (!empty($coords) and $valid_coords) {
      $map_api_key = variable_get('tpps_maps_api_key', NULL);
      // @TODO Minor. Replace with '#attached' and 'type' => 'external'
      // Be sure to set 'async' and 'defer' HTML attributes.
      $map_api_tools = "<script src=\"https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/markerclusterer.js\"></script><script src=\"https://maps.googleapis.com/maps/api/js?key=$map_api_key&callback=initMap\"
      async defer></script>"
      . "<div id=\"_map_wrapper\"></div>";
      // WARNING: Using $form['#attached']['js'][] causes missing Google Map
      // widget at page. Probably it's caused by using AJAX-requests to get
      // this form elements...
      drupal_add_js(
        ['tpps' => ['tree_info' => $coords, 'study_locations' => TRUE]],
        'setting'
      );
      $form['study_location']['map-button']['#suffix'] = $map_api_tools;
    }
  }

  return $form;
}

/**
 * This function processes a single row of an accession file.
 *
 * This function populates the pop_groups attribute of the options array with
 * the names of all the selected population groups from an accession file. This
 * function is meant to be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_accession_pop_groups($row, array &$options) {
  $groups = &$options['pop_groups'];
  $col = current($options['columns']);
  if (array_search($row[$col], $groups) === FALSE) {
    $groups[] = $row[$col];
  }
}
