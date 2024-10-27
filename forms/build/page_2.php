<?php

/**
 * @file
 * Creates the Study Design page and includes helper files.
 */

require_once 'page_2_ajax.php';
require_once 'page_2_helper.php';
require_once 'page_2_tppsc_helper.php';

use Drupal\tpps\Submission;

/**
 * Builds TPPS Page 2 Form.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 */
function tpps_page_2_create_form_tpps(array &$form, array $form_state) {
  tpps_study_date('Starting', $form, $form_state);
  tpps_study_date('Ending', $form, $form_state);
  $form['data_type'] = [
    '#type' => 'select',
    '#title' => t('Data Type: *'),
    '#options' => tpps_form_get_data_type_options(),
    '#prefix' => '<legend><span class="fieldset-legend">'
      . '<div class="fieldset-title">Study Design</div></span></legend>',
  ];
  $form['study_type'] = [
    '#type' => 'select',
    '#title' => t('Study Type: *'),
    '#options' => tpps_form_get_experimental_design_options(),
    '#ajax' => [
      'wrapper' => 'study_info',
      'callback' => 'tpps_study_type_callback',
    ],
  ];
  $form['study_info'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#prefix' => '<div id="study_info">',
    '#suffix' => '</div>',
  ];
  $type = tpps_get_ajax_value($form_state, ['study_type'], 0);
  switch ($type) {
    case TPPS_EXP_DESIGN_NATURAL_POPULATION:
      tpps_natural_population($form['study_info']);
      break;

    case TPPS_EXP_DESIGN_GROWTH_CHAMBER:
      tpps_growth_chamber($form['study_info']);
      break;

    case TPPS_EXP_DESIGN_GREENHOUSE:
      tpps_greenhouse($form['study_info']);
      unset($form['study_info']['humidity']['uncontrolled']);
      unset($form['study_info']['light']['uncontrolled']);
      unset($form['study_info']['rooting']['ph']['uncontrolled']);
      break;

    case TPPS_EXP_DESIGN_EXPERIMENTAL:
      tpps_common_garden($form['study_info']);
      break;

    case TPPS_EXP_DESIGN_PLANTATION:
      tpps_plantation($form['study_info']);
      break;

    default:
      $form['study_info']['#prefix'] = '<div id="study_info" style="display:none;">';
      break;
  }
  tpps_form_autofocus($form, ['data_type']);
}

/**
 * Builds TPPSc Page 2 Form.
 *
 * This function mainly calls the helper functions for the start date, end date,
 * and various study types.
 */
function tpps_page_2_create_form_tppsc() {
  global $tppsForm;

  //$tppsForm->debugMode(1);

  $tppsForm->form['study_design'] = [
    '#type' => 'fieldset',
    // WARNING: No tree!
    '#tree' => FALSE,
    '#collapsible' => TRUE,
    '#title' => t('Study Design'),
  ];
  //// Relocated v.2:
  //// $form['data_type'] -> $form['study_design']['data_type'].
  $tppsForm->form['study_design']['data_type'] = [
    '#type' => 'select',
    '#title' => t('Data Type'),
    '#options' => tpps_form_get_data_type_options(),
    '#required' => TRUE,
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Relocated v.2.
  // $form['study_type'] -> $form['study_design']['experimental_design'].
  $tppsForm->form['study_design']['experimental_design'] = [
    '#type' => 'select',
    '#title' => t('Experimental Design:'),
    '#options' => tpps_form_get_experimental_design_options(),
    '#required_when_visible' => TRUE,
    '#states' => [
      'visible' => [
        [
          ':input[name="data_type"]'
          => ['value' => TPPS_DATA_TYPE_GENOTYPE],
        ], 'or', [
          ':input[name="data_type"]'
          => ['value' => TPPS_DATA_TYPE_PHENOTYPE],
        ], 'or', [
          ':input[name="data_type"]'
          => ['value' => TPPS_DATA_TYPE_G_P],
        ],
      ],
    ],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $tppsForm->form['study_info'] = [
    '#type' => 'fieldset',
    '#title' => t('Study Info'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#required_when_visible' => TRUE,
    '#states' => [
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
    ],
  ];

  tppsc_page2_add_control_fields(
    ['type' => 'co2', 'label' => 'CO2 level']);
  tppsc_page2_add_control_fields(
    ['type' => 'humidity', 'label' => 'Air Humidity level']);
  tppsc_page2_add_control_fields(
    ['type' => 'light', 'label' => 'Light Intensity level']);
  tppsc_page2_add_control_fields(
    ['type' => 'temp', 'label' => 'Temperature']);

  // @TODO New fields. Check names.
  tppsc_page2_add_control_fields(
    ['type' => 'growth_medium', 'label' => 'Growth Medium']);
  tppsc_page2_add_control_fields(
    ['type' => 'ph_growth_medium', 'label' => 'pH of the growth medium']);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  $form_bus['group'] = 'common_garden';

  $subform = &$tppsForm->form['study_info'];

  //$num_arr = array_combine(range(1, 30), range(1, 30));
  // range()?
  $num_arr = [];
  $num_arr[0] = '- Select -';
  for ($i = 1; $i <= 30; $i++) {
    $num_arr[$i] = $i;
  }

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
    '#options' => array(
      0 => t('- Select -'),
      'Irrigation from top' => t('Irrigation from top'),
      'Irrigation from bottom' => t('Irrigation from bottom'),
      'Drip Irrigation' => t('Drip Irrigation'),
      'Other' => t('Other'),
      'No Irrigation' => t('No Irrigation'),
    ),
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


  $subform['biotic_env'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
  ];

  $subform['biotic_env']['option'] = [
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

  $subform['biotic_env']['other'] = [
    '#type' => 'textfield',
    '#title' => t('Please specify Biotic Environment Type:'),
    '#required_when_visible' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="study_info[biotic_env][option][Other]"]' => ['checked' => TRUE],
      ],
    ],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  tppsc_page2_add_control_fields(
    ['type' => 'treatment', 'label' => 'Treatment']);

  // WARNING: no '#tree' element!
  // @TODO Fix it.
  $tppsForm->autoFocus(['study_design', 'data_type']);
}
