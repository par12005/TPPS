<?php

/**
 * @file
 * Creates the Study Design page and includes helper files.
 */

require_once 'page_2_ajax.php';
require_once 'page_2_helper.php';

/**
 * Creates the Study Design form page.
 *
 * This function mainly calls the helper functions for the start date, end date,
 * and various study types.
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

  if (isset($form_state['saved_values'][TPPS_PAGE_2])) {
    $values = $form_state['saved_values'][TPPS_PAGE_2];
  }
  else {
    $values = array();
  }

  tpps_study_date('Starting', $form, $values, $form_state);

  tpps_study_date('Ending', $form, $values, $form_state);

  $form['data_type'] = array(
    '#type' => 'select',
    '#title' => t('Data Type: *'),
    '#options' => array(
      0 => '- Select -',
      'Genotype' => 'Genotype',
      'Phenotype' => 'Phenotype',
      'Environment' => 'Environment',
      'Genotype x Phenotype' => 'Genotype x Phenotype',
      'Genotype x Environment' => 'Genotype x Environment',
      'Phenotype x Environment' => 'Phenotype x Environment',
      'Genotype x Phenotype x Environment' => 'Genotype x Phenotype x Environment',
    ),
    '#prefix' => '<legend><span class="fieldset-legend"><div class="fieldset-title">Study Design</div></span></legend>',
  );

  $form['study_type'] = array(
    '#type' => 'select',
    '#title' => t('Study Type: *'),
    '#options' => array(
      0 => '- Select -',
      1 => 'Natural Population (Landscape)',
      2 => 'Growth Chamber',
      3 => 'Greenhouse',
      4 => 'Experimental/Common Garden',
      5 => 'Plantation',
    ),
    '#ajax' => array(
      'wrapper' => 'study_info',
      'callback' => 'tpps_study_type_callback',
    ),
  );

  $form['study_info'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#prefix' => '<div id="study_info">',
    '#suffix' => '</div>',
  );

  if (!empty($form_state['values']['study_type'])) {
    $type = $form_state['values']['study_type'];
  }
  elseif (!empty($form_state['saved_values'][TPPS_PAGE_2]['study_type'])) {
    $type = $form_state['saved_values'][TPPS_PAGE_2]['study_type'];
  }
  else {
    $type = 0;
  }

  switch ($type) {
    case '1':
      tpps_natural_population($form['study_info']);
      break;

    case '2':
      tpps_growth_chamber($form['study_info']);
      break;

    case '3':
      tpps_greenhouse($form['study_info']);
      unset($form['study_info']['humidity']['uncontrolled']);
      unset($form['study_info']['light']['uncontrolled']);
      unset($form['study_info']['rooting']['ph']['uncontrolled']);
      break;

    case '4':
      tpps_common_garden($form['study_info']);
      break;

    case '5':
      tpps_plantation($form['study_info']);
      break;

    default:
      $form['study_info']['#prefix'] = '<div id="study_info" style="display:none;">';
      break;
  }

  $form['Back'] = array(
    '#type' => 'submit',
    '#value' => t('Back'),
    '#prefix' => '<div class="input-description">* : Required Field</div>',
  );

  $form['Save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['Next'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );

  return $form;
}
