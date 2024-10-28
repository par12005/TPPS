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
  global $tppsForm;
  $tppsForm->form['study_info']['#title'] = t('Growth Chamber Information:');
// @todo update!
  $group = 'growth_chamber';
  $list = [
    'co2' => 'CO2 level',
    'humidity' => 'Air Humidity level',
    'light' => 'Light Intensity level',
    'temp' => 'Temperature',

    // @TODO New fields. Check names.
    'growth_medium' => 'Growth Medium',
    'ph_growth_medium' => 'pH of the growth medium',
    'treatment' => 'Treatment',
  ];
  foreach ($list as $type => $label) {
    tppsc_page2_add_control_fields($group, $type, $label);
  }
}

/**
 * This function creates fields for the growth chamber study type.
 *
 * @param array $form_bus
 *   Data to build form.
 */
function tppsc_page2_greenhouse(array $form_bus) {
  global $tppsForm;
  $tppsForm->form['study_info']['#title'] = t('GreenHouse Information:');
  $group = 'greenhouse';
  $list = [
    // Note: no 'CO2'. 'growth_chamber' has 'CO2'.
    'humidity' => 'Air Humidity level',
    'light' => 'Light Intensity level',
    'temp' => 'Temperature',
    // @TODO New fields. Check names.
    'growth_medium' => 'Growth Medium',
    'ph_growth_medium' => 'pH of the growth medium',
    'treatment' => 'Treatment',
  ];
  foreach ($list as $type => $label) {
    tppsc_page2_add_control_fields($group, $type, $label);
  }
}

/**
 * This function creates fields for the growth chamber study type.
 *
 * @param array $form_bus
 *   Data to build form.
 */
function tppsc_page2_plantation(array $form_bus) {
  global $tppsForm;
  $tppsForm->form['study_info']['#title'] = t('Plantation Information:');
  $group = 'plantation';
  $list = [
    'treatment' => 'Treatment',
  ];
  foreach ($list as $type => $label) {
    tppsc_page2_add_control_fields($group, $type, $label);
  }
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
  global $tppsForm;
  $tppsForm->form['study_info']['#title'] = t('Common Garden Information:');
  $group = 'common_garden';

  $subform = &$tppsForm->form['study_info'];
  $num_arr = range(0, 30);
  $num_arr[0] = '- Select -';

  $subform['assessions'] = [
    '#type' => 'select',
    '#title' => t('Number of times the populations were assessed (on average):'),
    '#options' => $num_arr,
    '#required_when_visible' => TRUE,
  ];

  $subform['irrigation'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
  ];

  $submform['irrigation']['option'] = [
    '#type' => 'select',
    '#title' => t('Irrigation Type:'),
    '#required_when_visible' => TRUE,
    '#options' => [
      0 => t('- Select -'),
      'Irrigation from top' => t('Irrigation from top'),
      'Irrigation from bottom' => t('Irrigation from bottom'),
      'Drip Irrigation' => t('Drip Irrigation'),
      'Other' => t('Other'),
      'No Irrigation' => t('No Irrigation'),
    ],
  ];

  $subform['irrigation']['other'] = [
    '#type' => 'textfield',
    '#required_when_visible' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="study_info[irrigation][option]"]' => ['value' => 'Other'],
      ],
    ],
  ];


  $form['biotic_env'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
  ];

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

  $form['biotic_env']['other'] = [
    '#type' => 'textfield',
    '#title' => t('Please specify Biotic Environment Type:'),
    '#required_when_visible' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="study_info[biotic_env][option][Other]"]' => ['checked' => TRUE],
      ],
    ],
  ];

  $list = [
    'treatment' => 'Treatment',
  ];
  foreach ($list as $type => $label) {
    tppsc_page2_add_control_fields($group, $type, $label);
  }
}

/**
 * Creates fields for the items that have control options.
 *
 * Fields which could be added:
 * co2, humidity, light intensity, salinity, and pH.
 *
 * @param string $group
 *   Group.
 * @param string $type
 *   The machine-readable type of control options.
 * @param string $label
 *   The human-readable label for the control options.
 */
function tppsc_page2_add_control_fields($group, $type, $label) {
  global $tppsForm;

  // The form to be updated.
  $subform = &$tppsForm->form['study_info'];
  $suffix = ($type == 'growth_medium' ? t('used in') : t('within'));

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Yes/No field.
  $field_name = $type;
  if ($type == 'treatment') {
    $title = t('Do you have information about the treatments to these plants?');
  }
  else {
    $title = t('Do you have information about the <strong>@type</strong> '
      . '@suffix the <span class="group">group</span>?',
      [
        '@type' => $label,
        '@suffix' => $suffix,
      ]
    );
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $type_list = [
    'humidity', 'light', 'temp', 'growth_medium', 'ph_growth_medium',
  ];
  // States.
  if ($type == 'co2') {
    $states = [
      'visible' => [
        ':input[name="experimental_design"]'
        => ['value' => TPPS_EXP_DESIGN_GROWTH_CHAMBER],
      ],
    ];
  }
  elseif (in_array($type, $type_list)) {
    $states = [
      'visible' => [
        [
          ':input[name="experimental_design"]'
          => ['value' => TPPS_EXP_DESIGN_GROWTH_CHAMBER],
        ], 'or', [
          ':input[name="experimental_design"]'
          => ['value' => TPPS_EXP_DESIGN_GREENHOUSE],
        ],
      ],
    ];
  }
  elseif ($type == 'treatment') {
    $states = [
      'visible' => [
        [
          ':input[name="experimental_design"]'
          => ['value' => TPPS_EXP_DESIGN_GROWTH_CHAMBER],
        ], 'or', [
          ':input[name="experimental_design"]'
          => ['value' => TPPS_EXP_DESIGN_GREENHOUSE],
        ], 'or', [
          ':input[name="experimental_design"]'
          => ['value' => TPPS_EXP_DESIGN_EXPERIMENTAL],
        ], 'or', [
          ':input[name="experimental_design"]'
          => ['value' => TPPS_EXP_DESIGN_PLANTATION],
        ],
      ],
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  tpps_form_add_yesno_field([
    'stage' => TPPS_PAGE_2,
    'parents' => ['study_info'],
    'field_name' => $field_name,
    '#title' => $title,
    '#default_value' => 0,
    '#required' => FALSE,
    '#states' => $states ?? [],
  ]);

  $subform[$type] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
    //'#states' => $states ?? [],
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
