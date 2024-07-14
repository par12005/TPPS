<?php

/**
 * @file
 * Define the helper functions for the Study Design page.
 */

/**
 * This function creates fields for the growth chamber study type.
 *
 * @param array $form_bus
 *   Data to build form.
 */
function tppsc_page2_growth_chamber(array $form_bus) {
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
function tppsc_page2_greenhouse(array $form_bus) {
  $form_bus['form']['study_info']['#title'] = t('GreenHouse Information:');
  $form_bus['group'] = 'greenhouse';

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
 * This function creates fields for the growth chamber study type.
 *
 * @param array $form_bus
 *   Data to build form.
 */
function tppsc_page2_plantation(array $form_bus) {
  $form_bus['form']['study_info']['#title'] = t('Plantation Information:');
  $form_bus['group'] = 'plantation';

  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'treatment', 'label' => 'Treatment']));
}

/**
 * tppsc_page2_common_garden
 *
 * @param array $form_bus
 * @access public
 *
 * @return void
 */
function tppsc_page2_common_garden(array $form_bus) {
  $form_bus['form']['study_info']['#title'] = t('Common Garden Information:');
  $form_bus['group'] = 'plantation';

  $subform = &$form_bus['form']['study_info'];

  // range()?
  $num_arr = array();
  $num_arr[0] = '- Select -';
  for ($i = 1; $i <= 30; $i++) {
    $num_arr[$i] = $i;
  }

  $subform['assessions'] = array(
    '#type' => 'select',
    '#title' => t('Number of times the populations were assessed (on average):'),
    '#options' => $num_arr,
    '#required_when_visible' => TRUE,
  );

  $subform['irrigation'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
  ];

  $submform['irrigation']['option'] = [
    '#type' => 'select',
    '#title' => t('Irrigation Type:'),
    '#required_when_visible' => TRUE,
    '#options' => array(
      0 => t('- Select -'),
      'Irrigation from top' => t('Irrigation from top'),
      'Irrigation from bottom' => t('Irrigation from bottom'),
      'Drip Irrigation' => t('Drip Irrigation'),
      'Other' => t('Other'),
      'No Irrigation' => t('No Irrigation'),
    ),
  ];

  $subform['irrigation']['other'] = array(
    '#type' => 'textfield',
    '#required_when_visible' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="study_info[irrigation][option]"]' => array('value' => 'Other'),
      ),
    ),
  );


  $form['biotic_env'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );

  $form['biotic_env']['option'] = [
    '#type' => 'checkboxes',
    '#title' => t('Biotic environmental interactions:'),
    '#required_when_visible' => TRUE,
    // @TODO Update to use english keys.
    '#options' => drupal_map_assoc([
      t('Herbivores'),
      t('Mutulists'),
      t('Pathogens'),
      t('Endophytes'),
      t('Other'),
      t('None'),
    ]),
  ];

  $form['biotic_env']['other'] = array(
    '#type' => 'textfield',
    '#title' => t('Please specify Biotic Environment Type:'),
    '#required_when_visible' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="study_info[biotic_env][option][Other]"]' => array('checked' => TRUE),
      ),
    ),
  );

  tppsc_page2_add_control_fields(array_merge($form_bus,
    ['type' => 'treatment', 'label' => 'Treatment']));
}

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
