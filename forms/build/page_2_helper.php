<?php

/**
 * @file
 * Define the helper functions for the Study Design page.
 */

/**
 * This function creates fields describing the study dates.
 *
 * @param string $type
 *   The type of date, 'Starting' or 'Ending'.
 * @param array $form
 *   The form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 *
 * @return array
 *   The populated form.
 */
function study_date($type, array &$form, array $values, array &$form_state) {

  $form[$type . 'Date'] = array(
    '#prefix' => "<div class='container-inline'>",
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#suffix' => '</div>',
  );

  if ($type == "Ending") {
    $form['EndingDate']['#states'] = array(
      'invisible' => array(
      array(
        array(':input[name="StartingDate[month]"]' => array('value' => '0')),
        'or',
        array(':input[name="StartingDate[year]"]' => array('value' => '0')),
      ),
      ),
    );
  }
  else {
    $form[$type . 'Date']['#title'] = t('<div class="fieldset-title">Experiment Dates</div>');
  }

  $yearArr = array();
  $yearArr[0] = '- Select -';
  for ($i = 1970; $i <= 2018; $i++) {
    $yearArr[$i] = $i;
  }

  $monthArr = array(
    0 => '- Select -',
    'January' => 'January',
    'February' => 'February',
    'March' => 'March',
    'April' => 'April',
    'May' => 'May',
    'June' => 'June',
    'July' => 'July',
    'August' => 'August',
    'September' => 'September',
    'October' => 'October',
    'November' => 'November',
    'December' => 'December',
  );

  $form[$type . 'Date']['year'] = array(
    '#type' => 'select',
    '#title' => t("@type Year: *", array('@type' => $type)),
    '#options' => $yearArr,
  );

  $form[$type . 'Date']['month'] = array(
    '#type' => 'select',
    '#title' => t("@type Month: *", array('@type' => $type)),
    '#options' => $monthArr,
    '#states' => array(
      'invisible' => array(
        ':input[name="' . $type . 'Date[year]"]' => array('value' => '0'),
      ),
    ),
  );

  if ($type == "Starting") {
    $form['StartingDate']['year']['#ajax'] = array(
      'callback' => 'ajax_date_year_callback',
      'wrapper' => 'Endingyear',
    );
    $form['StartingDate']['month']['#ajax'] = array(
      'callback' => 'ajax_date_month_callback',
      'wrapper' => 'Endingmonth',
    );
  }
  else {
    $form['EndingDate']['year']['#ajax'] = array(
      'callback' => 'ajax_date_month_callback',
      'wrapper' => 'Endingmonth',
    );
    $form['EndingDate']['year']['#prefix'] = '<div id="Endingyear">';
    $form['EndingDate']['year']['#suffix'] = '</div>';
    $form['EndingDate']['month']['#prefix'] = '<div id="Endingmonth">';
    $form['EndingDate']['month']['#suffix'] = '</div>';

    if (isset($form_state['values']['StartingDate']['year']) and $form_state['values']['StartingDate']['year'] != '0') {
      $yearArr = array();
      $yearArr[0] = '- Select -';
      for ($i = $form_state['values']['StartingDate']['year']; $i <= 2018; $i++) {
        $yearArr[$i] = $i;
      }
      $form['EndingDate']['year']['#options'] = $yearArr;
    }
    if (isset($form_state['values']['EndingDate']['year']) and $form_state['values']['EndingDate']['year'] == $form_state['values']['StartingDate']['year'] and isset($form_state['values']['StartingDate']['month']) and $form_state['values']['StartingDate']['month'] != '0') {
      foreach ($monthArr as $key) {
        if ($key != '0' and $key != $form_state['values']['StartingDate']['month']) {
          unset($monthArr[$key]);
        }
        elseif ($key == $form_state['values']['StartingDate']['month']) {
          break;
        }
      }
      $form['EndingDate']['month']['#options'] = $monthArr;
    }
  }

  return $form;
}

/**
 * This function creates fields describing the study location.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 *
 * @return array
 *   The populated form.
 */
function study_location(array &$form, array $values, array &$form_state) {

  $form['study_location'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Study Location:</div>'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
  );

  $form['study_location']['type'] = array(
    '#type' => 'select',
    '#title' => t('Coordinate Projection: *'),
    '#options' => array(
      0 => '- Select -',
      1 => 'WGS 84',
      3 => 'NAD 83',
      4 => 'ETRS 89',
      2 => 'Custom Location (street address)',
    ),
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('left'),
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
    '#prefix' => '<div id="page_2_map">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'callback' => 'page_2_map_ajax',
      'wrapper' => 'page_2_map',
    ),
  );

  if (isset($form_state['values']['study_location'])) {
    $location = $form_state['values']['study_location'];
  }
  elseif (isset($form_state['saved_values'][TPPS_PAGE_2]['study_location'])) {
    $location = $form_state['saved_values'][TPPS_PAGE_2]['study_location'];
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

    if (!empty($query)) {
      $form['study_location']['map-button']['#suffix'] = "
      <br><iframe
        width=\"100%\"
        height=\"450\"
        frameborder=\"0\" style=\"border:0\"
        src=\"https://www.google.com/maps?q=$query&output=embed&key=AIzaSyDkeQ6KN6HEBxrIoiSCrCHFhIbipycqouY&z=5\" allowfullscreen>
      </iframe></div>";
    }
  }

  return $form;
}

/**
 * This function creates fields for the natural population study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function natural_population(array &$form) {

  $form['#title'] = t('<div class="fieldset-title">Natural Population/Landscape Information:</div>');

  $form['season'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Seasons (select all that apply): *'),
    '#options' => drupal_map_assoc(array(
      t('Spring'),
      t('Summer'),
      t('Fall'),
      t('Winter'),
    )),
  );

  $num_arr = array();
  $num_arr[0] = '- Select -';
  for ($i = 1; $i <= 30; $i++) {
    $num_arr[$i] = $i;
  }

  $form['assessions'] = array(
    '#type' => 'select',
    '#title' => t('Number of times the populations were assessed (on average): *'),
    '#options' => $num_arr,
  );
}

/**
 * This function creates fields for the growth chamber study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function growth_chamber(array &$form) {

  $form['#title'] = t('<div class="fieldset-title">Growth Chamber Information:</div>');

  control($form, 'co2', 'CO2');
  control($form, 'humidity', 'Air humidity');
  control($form, 'light', 'Light Intensity');

  $form['temp'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Temperature Information:</div>'),
    '#description' => t('Please provide temperatures in Degrees Celsius'),
    '#tree' => TRUE,
  );

  $form['temp']['high'] = array(
    '#type' => 'textfield',
    '#title' => t('Average High Temperature: *'),
  );

  $form['temp']['low'] = array(
    '#type' => 'textfield',
    '#title' => t('Average Low Temperature: *'),
  );

  rooting($form);
}

/**
 * This function creates fields for the greenhouse study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function greenhouse(array &$form) {

  $form['#title'] = t('<div class="fieldset-title">Greenhouse Information:</div>');

  control($form, 'humidity', 'Air humidity');
  control($form, 'light', 'Light Intensity');

  $form['temp'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Temperature Information:</div>'),
    '#description' => t('Please provide temperatures in Degrees Celsius'),
    '#tree' => TRUE,
  );

  $form['temp']['high'] = array(
    '#type' => 'textfield',
    '#title' => t('Average High Temperature: *'),
  );

  $form['temp']['low'] = array(
    '#type' => 'textfield',
    '#title' => t('Average Low Temperature: *'),
  );

  rooting($form);
}

/**
 * This function creates fields for the common garden study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function common_garden(array &$form) {

  $form['#title'] = t('<div class="fieldset-title">Common Garden Information:</div>');

  $form['irrigation'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );

  $form['irrigation']['option'] = array(
    '#type' => 'select',
    '#title' => t('Irrigation Type: *'),
    '#options' => array(
      0 => '- Select -',
      'Irrigation from top' => 'Irrigation from top',
      'Irrigation from bottom' => 'Irrigation from bottom',
      'Drip Irrigation' => 'Drip Irrigation',
      'Other' => 'Other',
      'No Irrigation' => 'No Irrigation',
    ),
  );

  $form['irrigation']['other'] = array(
    '#type' => 'textfield',
    '#states' => array(
      'visible' => array(
        ':input[name="study_info[irrigation][option]"]' => array('value' => 'Other'),
      ),
    ),
  );

  control($form, 'salinity', 'Salinity');

  $form['biotic_env'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );

  $form['biotic_env']['option'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Biotic Environment: *'),
    '#options' => drupal_map_assoc(array(
      t('Herbivores'),
      t('Mutulists'),
      t('Pathogens'),
      t('Endophytes'),
      t('Other'),
      t('None'),
    )),
  );

  $form['biotic_env']['other'] = array(
    '#type' => 'textfield',
    '#title' => t('Please specify Biotic Environment Type: *'),
    '#states' => array(
      'visible' => array(
        ':input[name="study_info[biotic_env][option][Other]"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['season'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Seasons: *'),
    '#options' => drupal_map_assoc(array(
      t('Spring'),
      t('Summer'),
      t('Fall'),
      t('Winter'),
    )),
  );

  $treatment_options = drupal_map_assoc(array(
    t('Seasonal environment'),
    t('Antibiotic regime'),
    t('Chemical administration'),
    t('Disease status'),
    t('Fertilizer regime'),
    t('Fungicide regime'),
    t('Gaseous regime'),
    t('Gravity Growth hormone regime'),
    t('Herbicide regime'),
    t('Mechanical treatment'),
    t('Mineral nutrient regime'),
    t('Non-mineral nutrient regime'),
    t('Salt regime'),
    t('Watering regime'),
    t('Pesticide regime'),
    t('pH regime'),
    t('Other perturbation'),
  ));

  $form['treatment'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Treatments:</div>'),
  );

  $form['treatment']['check'] = array(
    '#type' => 'checkbox',
    '#title' => t('My Common Garden experiment used treatments/regimes/perturbations.'),
  );

  foreach ($treatment_options as $key => $option) {
    $form['treatment']["$option"] = array(
      '#type' => 'checkbox',
      '#title' => t("@opt", array('@opt' => $option)),
      '#states' => array(
        'visible' => array(
          ':input[name="study_info[treatment][check]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['treatment']["$option-description"] = array(
      '#type' => 'textfield',
      '#description' => t("@opt Description *", array('@opt' => $option)),
      '#states' => array(
        'visible' => array(
          ':input[name="study_info[treatment][' . $option . ']"]' => array('checked' => TRUE),
          ':input[name="study_info[treatment][check]"]' => array('checked' => TRUE),
        ),
      ),
    );
  }
}

/**
 * This function creates fields for the plantation study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function plantation(array &$form) {

  $form['#title'] = t('<div class="fieldset-title">Plantation Information:</div>');

  $form['season'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Seasons (select all that apply): *'),
    '#options' => drupal_map_assoc(array(
      t('Spring'),
      t('Summer'),
      t('Fall'),
      t('Winter'),
    )),
  );

  $num_arr = array();
  $num_arr[0] = '- Select -';
  for ($i = 1; $i <= 30; $i++) {
    $num_arr[$i] = $i;
  }

  $form['assessions'] = array(
    '#type' => 'select',
    '#title' => t('Number of times the populations were assessed (on average): *'),
    '#options' => $num_arr,
  );

  $treatment_options = drupal_map_assoc(array(
    t('Seasonal environment'),
    t('Antibiotic regime'),
    t('Chemical administration'),
    t('Disease status'),
    t('Fertilizer regime'),
    t('Fungicide regime'),
    t('Gaseous regime'),
    t('Gravity Growth hormone regime'),
    t('Herbicide regime'),
    t('Mechanical treatment'),
    t('Mineral nutrient regime'),
    t('Non-mineral nutrient regime'),
    t('Salt regime'),
    t('Watering regime'),
    t('Pesticide regime'),
    t('pH regime'),
    t('Other perturbation'),
  ));

  $form['treatment'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Treatments:</div>'),
  );

  $form['treatment']['check'] = array(
    '#type' => 'checkbox',
    '#title' => t('My Plantation experiment used treatments/regimes/perturbations.'),
  );

  foreach ($treatment_options as $key => $option) {
    $form['treatment']["$option"] = array(
      '#type' => 'checkbox',
      '#title' => t("@opt", array('@opt' => $option)),
      '#states' => array(
        'visible' => array(
          ':input[name="study_info[treatment][check]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['treatment']["$option-description"] = array(
      '#type' => 'textfield',
      '#description' => t("@opt Description *", array('@opt' => $option)),
      '#states' => array(
        'visible' => array(
          ':input[name="study_info[treatment][' . $option . ']"]' => array('checked' => TRUE),
          ':input[name="study_info[treatment][check]"]' => array('checked' => TRUE),
        ),
      ),
    );
  }
}

/**
 * This function creates fields for rooting information.
 *
 * @param array $form
 *   The form to be populated.
 */
function rooting(array &$form) {

  $form['rooting'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Rooting Information:</div>'),
    '#tree' => TRUE,
  );

  $form['rooting']['option'] = array(
    '#type' => 'select',
    '#title' => t('Rooting Type: *'),
    '#options' => array(
      0 => '- Select -',
      'Aeroponics' => 'Aeroponics',
      'Hydroponics' => 'Hydroponics',
      'Soil' => 'Soil',
    ),
  );

  $form['rooting']['soil'] = array(
    '#type' => 'fieldset',
    '#states' => array(
      'visible' => array(
        ':input[name="study_info[rooting][option]"]' => array('value' => 'Soil'),
      ),
    ),
  );

  $form['rooting']['soil']['type'] = array(
    '#type' => 'select',
    '#title' => t('Soil Type: *'),
    '#options' => array(
      0 => '- Select -',
      'Sand' => 'Sand',
      'Peat' => 'Peat',
      'Clay' => 'Clay',
      'Mixed' => 'Mixed',
      'Other' => 'Other',
    ),
  );

  $form['rooting']['soil']['other'] = array(
    '#type' => 'textfield',
    '#states' => array(
      'visible' => array(
        ':input[name="study_info[rooting][soil][type]"]' => array('value' => 'Other'),
      ),
    ),
  );

  $form['rooting']['soil']['container'] = array(
    '#type' => 'textfield',
    '#title' => t('Soil Container Type: *'),
  );

  control($form['rooting'], 'ph', 'pH');

  $treatment_options = drupal_map_assoc(array(
    t('Seasonal Environment'),
    t('Air temperature regime'),
    t('Soil Temperature regime'),
    t('Antibiotic regime'),
    t('Chemical administration'),
    t('Disease status'),
    t('Fertilizer regime'),
    t('Fungicide regime'),
    t('Gaseous regime'),
    t('Gravity Growth hormone regime'),
    t('Mechanical treatment'),
    t('Mineral nutrient regime'),
    t('Humidity regime'),
    t('Non-mineral nutrient regime'),
    t('Radiation (light, UV-B, X-ray) regime'),
    t('Rainfall regime'),
    t('Salt regime'),
    t('Watering regime'),
    t('Water temperature regime'),
    t('Pesticide regime'),
    t('pH regime'),
    t('other perturbation'),
  ));

  $form['rooting']['treatment'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Treatments: *</div>'),
  );

  foreach ($treatment_options as $key => $option) {
    $form['rooting']['treatment']["$option"] = array(
      '#type' => 'checkbox',
      '#title' => t("@opt", array('@opt' => $option)),
    );
    $form['rooting']['treatment']["$option-description"] = array(
      '#type' => 'textfield',
      '#description' => t("@opt Description *", array('@opt' => $option)),
      '#states' => array(
        'visible' => array(
          ':input[name="study_info[rooting][treatment][' . $option . ']"]' => array('checked' => TRUE),
        ),
      ),
    );
  }
}

/**
 * This function creates fields for the items that have control options.
 *
 * This includes co2, humidity, light intensity, salinity, and pH.
 *
 * @param array $form
 *   The form to be updated.
 * @param string $type
 *   The machine-readable type of control options.
 * @param string $label
 *   The human-readable label for the control options.
 */
function control(array &$form, $type, $label) {
  $form[$type] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );

  $form[$type]['option'] = array(
    '#type' => 'select',
    '#title' => t('@label controlled or uncontrolled: *', array('@label' => $label)),
    '#options' => array(
      0 => '- Select -',
      1 => 'Controlled',
      2 => 'Uncontrolled',
    ),
  );

  $form[$type]['controlled'] = array(
    '#type' => 'textfield',
    '#title' => t('Controlled @label Value: *', array('@label' => $label)),
    '#states' => array(
      'visible' => array(
        ":input[name=\"study_info[$type][option]\"]" => array('value' => '1'),
      ),
    ),
  );

  $form[$type]['uncontrolled'] = array(
    '#type' => 'textfield',
    '#title' => t('Average @label Value: *', array('@label' => $label)),
    '#states' => array(
      'visible' => array(
        ":input[name=\"study_info[$type][option]\"]" => array('value' => '2'),
      ),
    ),
  );

  if ($type == 'ph') {
    $form[$type]['controlled']['#states']['visible'] = array(
      ':input[name="study_info[rooting][ph][option]"]' => array('value' => '1'),
    );
    $form[$type]['uncontrolled']['#states']['visible'] = array(
      ':input[name="study_info[rooting][ph][option]"]' => array('value' => '2'),
    );
  }
}
