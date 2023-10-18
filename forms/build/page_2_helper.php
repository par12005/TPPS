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
 * @param array $form_state
 *   The state of the form to be populated.
 */
function tpps_study_date($type, array &$form, array &$form_state) {
  $form[$type . 'Date'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
    // Must be above 'Data Type' and 'Study Type' fields.
    '#weight' => -10,
  ];
  if ($type == "Ending") {
    $form['EndingDate']['#states'] = [
      'invisible' => [
        [
          [':input[name="StartingDate[month]"]' => ['value' => '0']],
          'or',
          [':input[name="StartingDate[year]"]' => ['value' => '0']],
        ],
      ],
    ];
  }
  else {
    $form[$type . 'Date']['#title'] =
      t('<div class="fieldset-title">Experiment/Analysis Dates</div>');
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Year
  $form[$type . 'Date']['year'] = [
    '#type' => 'select',
    '#title' => t("@type Year: *", ['@type' => $type]),
    '#options' => tpps_get_year_options(),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Month.

  $form[$type . 'Date']['month'] = [
    '#type' => 'select',
    '#title' => t("@type Month: *", ['@type' => $type]),
    '#options' => tpps_get_month_options(),
    '#states' => [
      'invisible' => [
        ':input[name="' . $type . 'Date[year]"]' => ['value' => '0'],
      ],
    ],
  ];

  if ($type == "Starting") {
    $form['StartingDate']['year']['#ajax'] = [
      'callback' => 'tpps_date_year_callback',
      'wrapper' => 'Endingyear',
    ];
    $form['StartingDate']['month']['#ajax'] = [
      'callback' => 'tpps_date_month_callback',
      'wrapper' => 'Endingmonth',
    ];
  }
  else {
    $form['EndingDate']['year']['#ajax'] = [
      'callback' => 'tpps_date_month_callback',
      'wrapper' => 'Endingmonth',
    ];
    $form['EndingDate']['year']['#prefix'] = '<div id="Endingyear">';
    $form['EndingDate']['year']['#suffix'] = '</div>';
    $form['EndingDate']['month']['#prefix'] = '<div id="Endingmonth">';
    $form['EndingDate']['month']['#suffix'] = '</div>';

    if (
      isset($form_state['values']['StartingDate']['year'])
      && $form_state['values']['StartingDate']['year'] != '0'
    ) {
      $form['EndingDate']['year']['#options'] = tpps_get_year_options(
        ($form_state['values']['StartingDate']['year'] ?? 1900)
      );
    }
    // @TODO Minor. Move to JS to avoid AJAX requests.
    if (
      !empty($form_state['values']['StartingDate']['month'])
      && ($form_state['values']['EndingDate']['year'] ?? 0)
        == $form_state['values']['StartingDate']['year']
    ) {
      $form['EndingDate']['month']['#options']
        = tpps_get_month_options($form_state['values']['StartingDate']['month']);
    }
  }
}

/**
 * This function creates fields for the natural population study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function tpps_natural_population(array &$form) {

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
    '#description' => t('If you do not know which season your samples were collected, please select all.'),
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
function tpps_growth_chamber(array &$form) {

  $form['#title'] = t('<div class="fieldset-title">Growth Chamber Information:</div>');

  tpps_control($form, 'co2', 'CO2');
  tpps_control($form, 'humidity', 'Air humidity');
  tpps_control($form, 'light', 'Light Intensity');

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

  tpps_rooting($form);
}

/**
 * This function creates fields for the greenhouse study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function tpps_greenhouse(array &$form) {

  $form['#title'] = t('<div class="fieldset-title">Greenhouse Information:</div>');

  tpps_control($form, 'humidity', 'Air humidity');
  tpps_control($form, 'light', 'Light Intensity');

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

  tpps_rooting($form);
}

/**
 * This function creates fields for the common garden study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function tpps_common_garden(array &$form) {

  $form['#title'] = t('<div class="fieldset-title">Common Garden Information:</div>');

  $form['irrigation'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );

  $form['irrigation']['option'] = array(
    '#type' => 'select',
    '#title' => t('Irrigation Type: *'),
    '#options' => array(
      0 => t('- Select -'),
      'Irrigation from top' => t('Irrigation from top'),
      'Irrigation from bottom' => t('Irrigation from bottom'),
      'Drip Irrigation' => t('Drip Irrigation'),
      'Other' => t('Other'),
      'No Irrigation' => t('No Irrigation'),
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

  tpps_control($form, 'salinity', 'Salinity');

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
    '#description' => t('If you do not know which season your samples were collected, please select all.'),
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

  foreach ($treatment_options as $option) {
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
function tpps_plantation(array &$form) {

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
    '#description' => t('If you do not know which season your samples were collected, please select all.'),
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

  foreach ($treatment_options as $option) {
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
function tpps_rooting(array &$form) {

  $form['rooting'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Rooting Information:</div>'),
    '#tree' => TRUE,
  );

  $form['rooting']['option'] = array(
    '#type' => 'select',
    '#title' => t('Rooting Type: *'),
    '#options' => array(
      0 => t('- Select -'),
      'Aeroponics' => t('Aeroponics'),
      'Hydroponics' => t('Hydroponics'),
      'Soil' => t('Soil'),
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
      0 => t('- Select -'),
      'Sand' => t('Sand'),
      'Peat' => t('Peat'),
      'Clay' => t('Clay'),
      'Mixed' => t('Mixed'),
      'Other' => t('Other'),
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

  tpps_control($form['rooting'], 'ph', 'pH');

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

  foreach ($treatment_options as $option) {
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
function tpps_control(array &$form, $type, $label) {
  $form[$type] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );

  $form[$type]['option'] = array(
    '#type' => 'select',
    '#title' => t('@label controlled or uncontrolled: *', array('@label' => $label)),
    '#options' => array(
      0 => t('- Select -'),
      1 => t('Controlled'),
      2 => t('Uncontrolled'),
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
