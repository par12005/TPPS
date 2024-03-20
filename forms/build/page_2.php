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
  module_load_include('inc', 'tpps', 'includes/form');
  $is_tppsc = tpps_form_is_tppsc($form_state);
  if (!$is_tppsc) {
    tpps_study_date('Starting', $form, $form_state);
    tpps_study_date('Ending', $form, $form_state);
  }

  $options = array(
    0 => '- Select -',
    'Genotype' => 'Genotype',
    'Phenotype' => 'Phenotype (and/or manual environmental data)',
    'Genotype x Phenotype' => 'Genotype x Phenotype (and/or manual environmental data)',
  );

  if (module_exists('cartogratree') and db_table_exists('cartogratree_groups') and db_table_exists('cartogratree_layers')) {
    $options = array(
      0 => '- Select -',
      'Genotype' => 'Genotype',
      'Phenotype' => 'Phenotype (and/or manual environmental data)',
      'Environment' => 'Environmental layers',
      'Genotype x Phenotype' => 'Genotype x Phenotype (and/or manual environmental data)',
      'Genotype x Environment' => 'Genotype x Environmental layers',
      'Phenotype x Environment' => 'Phenotype (and/or manual environmental data) x Environmental layers',
      'Genotype x Phenotype x Environment' => 'Genotype x Phenotype (and/or manual environmental data) x Environmental layers',
    );
  }

  // @TODO Add fieldset and move those fields under this fieldset.
  // Be sure to update Submission Interface and related code.
  $form['data_type'] = array(
    '#type' => 'select',
    '#title' => t('Data Type: *'),
    '#options' => $options,
    '#prefix' => '<legend><span class="fieldset-legend"><div class="fieldset-title">Study Design</div></span></legend>',
  );
  tpps_form_autofocus($form, ['data_type']);
  $form['study_type'] = [
    '#type' => 'select',
    '#title' => t('Study Type: *'),
    '#options' => tpps_form_get_study_type(),
  ];
  if (!$is_tppsc) {
    $form['study_type']['#ajax'] = [
      'wrapper' => 'study_info',
      'callback' => 'tpps_study_type_callback',
    ];
    $form['study_info'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#collapsible' => TRUE,
      '#prefix' => '<div id="study_info">',
      '#suffix' => '</div>',
    );
    $type = tpps_get_ajax_value($form_state, array('study_type'), 0);

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
  }
  tpps_form_add_buttons(['form' => &$form, 'page' => 'page_2']);
  return $form;
}
