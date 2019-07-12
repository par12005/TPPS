<?php

/**
 * @file
 * Define the helper functions for the Tree Accession page.
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
function study_location(array &$form, array &$form_state) {

  $form['study_location'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Study Location:</div>'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#description' => t('This should be the location of your common garden, plantation, etc.'),
  );

  $form['study_location']['type'] = array(
    '#type' => 'select',
    '#title' => t('Coordinate Projection: *'),
    '#options' => array(
      0 => '- Select -',
      1 => 'WGS 84',
      3 => 'NAD 83',
      4 => 'ETRS 89',
      2 => 'Custom Location (brief description)',
    ),
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('right'),
      'title' => array('Please select a Coordinate Projection, or select "Custom Location", to enter a custom study location'),
    ),
  );

  $form['study_location']['coordinates'] = array(
    '#type' => 'textfield',
    '#title' => t('Coordinates: *'),
    '#states' => array(
      'visible' => array(
      array(
      array(':input[name="study_location[type]"]' => array('value' => '1')),
        'or',
      array(':input[name="study_location[type]"]' => array('value' => '3')),
        'or',
      array(':input[name="study_location[type]"]' => array('value' => '4')),
      ),
      ),
    ),
    '#description' => 'Accepted formats: <br>'
    . 'Degrees Minutes Seconds: 41° 48\' 27.7" N, 72° 15\' 14.4" W<br>'
    . 'Degrees Decimal Minutes: 41° 48.462\' N, 72° 15.24\' W<br>'
    . 'Decimal Degrees: 41.8077° N, 72.2540° W<br>',
  );

  $form['study_location']['custom'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Location: *'),
    '#states' => array(
      'visible' => array(
        ':input[name="study_location[type]"]' => array('value' => '2'),
      ),
    ),
  );

  $form['study_location']['map-button'] = array(
    '#type' => 'button',
    '#title' => 'Click here to update map',
    '#value' => 'Click here to update map',
    '#button_type' => 'button',
    '#executes_submit_callback' => FALSE,
    '#prefix' => '<div id="study_location_map">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'callback' => 'study_location_map_ajax',
      'wrapper' => 'study_location_map',
    ),
  );

  if (isset($form_state['values']['study_location'])) {
    $location = $form_state['values']['study_location'];
  }
  elseif (isset($form_state['saved_values'][TPPS_PAGE_3]['study_location'])) {
    $location = $form_state['saved_values'][TPPS_PAGE_3]['study_location'];
  }

  if (isset($location)) {
    if (isset($location['coordinates'])) {
      $raw_coordinate = $location['coordinates'];
      $standard_coordinate = tpps_standard_coord($raw_coordinate);
    }

    if (isset($location['type']) and $location['type'] == '2' and isset($location['custom'])) {
      $query = $location['custom'];
    }
    elseif (isset($location['type']) and $location['type'] != '0') {
      if ($standard_coordinate) {
        $query = $standard_coordinate;
      }
      else {
        drupal_set_message(t('Invalid coordinates'), 'error');
      }
    }

    $map_api_key = variable_get('tpps_maps_api_key', NULL);
    if (!empty($query) and !empty($map_api_key)) {
      $form['study_location']['map-button']['#suffix'] = "
      <br><iframe
        width=\"100%\"
        height=\"450\"
        frameborder=\"0\" style=\"border:0\"
        src=\"https://www.google.com/maps?q=$query&output=embed&key=$map_api_key&z=5\" allowfullscreen>
      </iframe></div>";
    }
  }

  return $form;
}

