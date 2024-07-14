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
 *
 * @return array
 *   The populated form.
 */
function tppsc_study_date($type, array &$form, array &$form_state) {

  $form[$type . 'Date'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
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
    $form[$type . 'Date']['#title'] = t('<div class="fieldset-title">Experiment/Analysis Dates</div>');
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
      'callback' => 'tpps_date_year_callback',
      'wrapper' => 'Endingyear',
    );
    $form['StartingDate']['month']['#ajax'] = array(
      'callback' => 'tpps_date_month_callback',
      'wrapper' => 'Endingmonth',
    );
  }
  else {
    $form['EndingDate']['year']['#ajax'] = array(
      'callback' => 'tpps_date_month_callback',
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
 * This function creates fields for the natural population study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function tppsc_natural_population(array &$form) {

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
 * @param array $form_bus
 *   Data to build form.
 */
function tppsc_growth_chamber(array $form_bus) {
  $form_bus['form']['study_info']['#title'] = t('Growth Chamber Information:');
  $form_bus['group'] = 'growth_chamber';

  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'co2', 'label' => 'CO2 level']));
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'humidity', 'label' => 'Air Humidity level']));
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'light', 'label' => 'Light Intensity level']));
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'temp', 'label' => 'Temperature']));

  // @TODO New fields. Check names.
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'growth_medium', 'label' => 'Growth Medium']));
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'ph_growth_medium', 'label' => 'pH of the growth medium']));
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'treatment', 'label' => 'Treatment']));
}

/**
 * This function creates fields for the growth chamber study type.
 *
 * @param array $form_bus
 *   Data to build form.
 */
function tppsc_greenhouse(array $form_bus) {
  $form_bus['form']['study_info']['#title'] = t('Growth Chamber Information:');
  $form_bus['group'] = 'growth_chamber';

  // Note: no 'CO2'. 'growth_chamber' has 'CO2'.
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'humidity', 'label' => 'Air Humidity level']));
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'light', 'label' => 'Light Intensity level']));
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'temp', 'label' => 'Temperature']));

  // @TODO New fields. Check names.
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'growth_medium', 'label' => 'Growth Medium']));
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'ph_growth_medium', 'label' => 'pH of the growth medium']));
  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'treatment', 'label' => 'Treatment']));
}


/**
 * This function creates fields for the common garden study type.
 *
 * @param array $form
 *   The form to be populated.
 */
function tppsc_common_garden(array &$form) {

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

  tpps_page2_add_control_fields($form, 'salinity', 'Salinity');

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
    '#title' => t('<div class="fieldset-title">TREATMENTS:</div>'),
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
function tppsc_plantation(array &$form) {

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

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Code below tested!
//
/**
 * Creates fields for the items that have control options.
 *
 * Fields which could be added:
 * co2, humidity, light intensity, salinity, and pH.
 *
 * @param array $form_bus
 *   Form Bus.
 */
function tppsc_page2_add_control_fields(array $form_bus) {
  // The form to be updated.
  $subform = &$form_bus['form']['study_info'];
  // The machine-readable type of control options.
  $type = $form_bus['type'];
  $suffix = ($type == 'growth_medium' ? t('used in') : t('within'));
  // The human-readable label for the control options.
  $label = $form_bus['label'];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Yes/No field.
  $field_name = $type . '_' . $form_bus['group'];
  if ($type == 'treatment') {
    $title = t('Do you have information about the treatments to these plants?');
  }
  else {
    $title = t('Do you have information about the <strong>@type</strong> '
      . '@suffix the @group?',
      [
        '@type' => $label,
        '@suffix' => $suffix,
        '@group' => str_replace('_', ' ', $form_bus['group']),
      ]
    );
  }
  tpps_form_add_yesno_field(array_merge($form_bus, [
    'stage' => TPPS_PAGE_2,
    'parents' => ['study_info'],
    'field_name' => $field_name,
    '#title' => $title,
    '#default_value' => 0,
    '#required' => FALSE,
  ]));

  $subform[$type] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
      ],
    ],
  ];

  if ($type == 'treatment') {
    $subform[$type]['header'] = [
      '#markup' => t('Which of the following treatments were applied to the plants?'),
    ];
    $treatment_options = tppsc_page2_get_treatment_options();
    foreach ($treatment_options as $option) {
      $subform[$type][$option] = [
        '#type' => 'checkbox',
        '#title' => t($option),
        '#required_when_visible' => TRUE,
        '#states' => [
          'visible' => [
            ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
          ],
        ],
      ];
      $subform[$type]["$option-description"] = [
        '#type' => 'textfield',
        '#title' => t("<strong>$option</strong> Description"),
        '#required_when_visible' => TRUE,
        '#states' => [
          'visible' => [
            ':input[name="study_info[' . $type . '][' . $option . ']"]' =>
              ['checked' => TRUE],
          ],
        ],
      ];
    }
  }
  elseif ($type == 'growth_medium') {
    $subform[$type]['rooting_type'] = [
      '#type' => 'select',
      '#title' => t('Rooting Type:'),
      '#options' => [
        0 => t('- Select -'),
        'Aeroponics' => t('Aeroponics'),
        'Hydroponics' => t('Hydroponics'),
        'Soil' => t('Soil'),
      ],
      '#required_when_visible' => TRUE,
      '#states' => [
        'visible' => [
          ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
        ],
      ],
    ];
  }
  elseif ($type == 'temp') {
    $subform[$type]['high'] = [
      '#type' => 'textfield',
      '#title' => t('Average <strong>High</strong> Temperature (in degrees Celsius):'),
      '#required_when_visible' => TRUE,
      '#states' => [
        'visible' => [
          ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
        ],
      ],
    ];
    $subform[$type]['low'] = [
      '#type' => 'textfield',
      '#required_when_visible' => TRUE,
      '#title' => t('Average <strong>Low</strong> Temperature (in degrees Celsius):'),
      '#states' => [
        'visible' => [
          ':input[name="study_info[' . $field_name . ']"]' => ['value' => 'yes'],
        ],
      ],
    ];

  }
  else {
    $title = t('Was the <strong>@label</strong> controlled or uncontrolled?',
      ['@label' => $label]
    );
    $subform[$type]['option'] = [
      '#type' => 'select',
      '#title' => $title,
      '#options' => [
        0 => t('- Select -'),
        TPPS_CONTROLLED => t('Controlled'),
        TPPS_UNCONTROLLED => t('Uncontrolled'),
      ],
    ];
    $subform[$type]['controlled'] = [
      '#type' => 'textfield',
      '#title' => t('Controlled <strong>@label</strong> Value:', ['@label' => $label]),
      '#required_when_visible' => TRUE,
      '#states' => [
        'visible' => [
          ":input[name=\"study_info[$type][option]\"]" => [
            'value' => TPPS_CONTROLLED,
          ],
        ],
      ],
    ];
    $subform[$type]['uncontrolled'] = [
      '#type' => 'textfield',
      '#title' => t('Uncontrolled <strong>@label</strong> Value:', ['@label' => $label]),
      '#required_when_visible' => TRUE,
      '#states' => [
        'visible' => [
          ":input[name=\"study_info[$type][option]\"]" => [
            'value' => TPPS_UNCONTROLLED,
          ],
        ],
      ],
    ];
  }
}

/**
 * Gets list of treatment options.
 *
 * @return array
 *   Returns list of treatment options.
 */
function tppsc_page2_get_treatment_options() {
  return [
    'Seasonal Environment',
    'Air temperature regime',
    'Soil Temperature regime',
    'Antibiotic regime',
    'Chemical administration',
    'Disease status',
    'Fertilizer regime',
    'Fungicide regime',
    'Gaseous regime',
    'Gravity Growth hormone regime',
    'Mechanical treatment',
    'Mineral nutrient regime',
    'Humidity regime',
    'Non-mineral nutrient regime',
    'Radiation (light, UV-B, X-ray) regime',
    'Rainfall regime',
    'Salt regime',
    'Watering regime',
    'Water temperature regime',
    'Pesticide regime',
    'pH regime',
    'other perturbation',
  ];
}
