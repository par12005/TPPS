<?php

/**
 * @file
 * Creates the Study Design page and includes helper files.
 */

require_once 'page_2_ajax.php';
require_once 'page_2_helper.php';
require_once 'page_2_tppsc_helper.php';

/**
 * Builds TPPS/TPPSc Page 2 Form.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 *
 * @return array
 *   The completed Study Design form.
 */
function tpps_page_2_create_form(array &$form, array $form_state) {
  if (tpps_form_is_tppsc($form_state)) {
    tpps_page_2_create_form_tppsc($form, $form_state);
  }
  else {
    tpps_page_2_create_form_tpps($form, $form_state);
  }
  tpps_form_add_buttons(['form' => &$form, 'page' => 'page_2']);
  return $form;
}

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
    // TPPSC Note: Relocated.
    '#title' => t('Data Type: *'),
    '#options' => tpps_form_get_data_type_options(),
    '#prefix' => '<legend><span class="fieldset-legend">'
      . '<div class="fieldset-title">Study Design</div></span></legend>',
  ];
  $form['study_type'] = [
    '#type' => 'select',
    // TPPSC Note: Relocated and renamed.
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
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 */
function tpps_page_2_create_form_tppsc(array &$form, array $form_state) {
  $form['study_design'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#title' => t('Study Design'),
  ];
  // Relocated v.2:
  // $form['data_type'] -> $form['study_design']['data_type'].
  $form['study_design']['data_type'] = [
    '#type' => 'select',
    '#title' => t('Data Type'),
    '#options' => tpps_form_get_data_type_options(),
    '#required' => TRUE,
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Relocated v.2.
  // $form['study_type'] -> $form['study_design']['experimental_design'].
  $form['study_design']['experimental_design'] = [
    '#type' => 'select',
    '#title' => t('Experimental Design:'),
    '#options' => tpps_form_get_experimental_design_options(),
    '#required_when_visible' => TRUE,
    '#states' => [
      'visible' => [
        [
          ':input[name="study_design[data_type]"]'
          => ['value' => TPPS_DATA_TYPE_GENOTYPE],
        ],
        'or',
        [
          ':input[name="study_design[data_type]"]'
          => ['value' => TPPS_DATA_TYPE_PHENOTYPE],
        ],
        'or',
        [
          ':input[name="study_design[data_type]"]'
          => ['value' => TPPS_DATA_TYPE_G_P],
        ],
      ],
    ],
  ];
  $form['study_info'] = [
    '#type' => 'fieldset',
    '#title' => t('Study Info'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#states' => [
      'invisible' => [
        [
          ':input[name="study_design[experimental_design]"]' => [
            'value' => TPPS_EXP_DESIGN_NATURAL_POPULATION,
          ],
        ], 'or', [
          ':input[name="study_design[experimental_design]"]' => ['value' => 0],
        ],
      ],
    ],
  ];

// @TODO Add more fields.
  tppsc_growth_chamber([
    'form' => &$form,
    'is_tppsC' => TRUE,
    'parents' => ['study_info'],
  ]);
  tpps_form_autofocus($form, ['study_design', 'data_type']);
}
