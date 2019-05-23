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
function page_2_create_form(array &$form, array $form_state) {

  if (isset($form_state['saved_values'][TPPS_PAGE_2])) {
    $values = $form_state['saved_values'][TPPS_PAGE_2];
  }
  else {
    $values = array();
  }

  study_date('Starting', $form, $values, $form_state);

  study_date('Ending', $form, $values, $form_state);

  study_location($form, $values, $form_state);

  $form['dataType'] = array(
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
  );

  $form['studyType'] = array(
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
  );

  natural_population($form, $values);

  growth_chamber($form, $values);

  greenhouse($form, $values);

  common_garden($form, $values);

  plantation($form, $values);

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
