<?php

/**
 * @file
 */

require_once 'page_4_ajax.php';
require_once 'page_4_helper.php';

/**
 *
 */
function page_4_create_form(&$form, &$form_state) {
  if (isset($form_state['saved_values'][TPPS_PAGE_4])) {
    $values = $form_state['saved_values'][TPPS_PAGE_4];
  }
  else {
    $values = array();
  }

  $genotype_upload_location = 'public://' . variable_get('tpps_genotype_files_dir', 'tpps_genotype');
  $phenotype_upload_location = 'public://' . variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');

  $form['#tree'] = TRUE;

  $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
  $data_type = $form_state['saved_values'][TPPS_PAGE_2]['data_type'];
  for ($i = 1; $i <= $organism_number; $i++) {

    $name = $form_state['saved_values'][TPPS_PAGE_1]['organism']["$i"];

    $form["organism-$i"] = array(
      '#type' => 'fieldset',
      '#title' => t("<div class=\"fieldset-title\">$name:</div>"),
      '#tree' => TRUE,
      // '#collapsible' => TRUE.
    );

    if (preg_match('/P/', $data_type)) {
      if ($i > 1) {
        $form["organism-$i"]['phenotype-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Phenotype information for $name is the same as phenotype information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]}.",
          '#default_value' => isset($values["organism-$i"]['phenotype-repeat-check']) ? $values["organism-$i"]['phenotype-repeat-check'] : 1,
        );
      }

      $form["organism-$i"]['phenotype'] = phenotype($form, $values, "organism-$i", $form_state, $phenotype_upload_location);

      if ($i > 1) {
        $form["organism-$i"]['phenotype']['#states'] = array(
          'invisible' => array(
            ":input[name=\"organism-$i\[phenotype-repeat-check]\"]" => array('checked' => TRUE),
          ),
        );
      }

      $form["organism-$i"]['phenotype']['file'] = array(
        '#type' => 'managed_file',
        '#title' => t('Phenotype file: Please upload a file containing columns for Tree Identifier, Phenotype Name, and value for all of your phenotypic data: *'),
        '#upload_location' => "$phenotype_upload_location",
        '#upload_validators' => array(
          'file_validate_extensions' => array('csv tsv xlsx'),
        ),
        '#tree' => TRUE,
      );

      $form["organism-$i"]['phenotype']['file']['empty'] = array(
        '#default_value' => isset($values["organism-$i"]['phenotype']['file']['empty']) ? $values["organism-$i"]['phenotype']['file']['empty'] : 'NA',
      );

      $form["organism-$i"]['phenotype']['file']['columns'] = array(
        '#description' => 'Please define which columns hold the required data: Tree Identifier, Phenotype name, and Value(s)',
      );

      $column_options = array(
        'N/A',
        'Tree Identifier',
        'Phenotype Name/Identifier',
        'Value(s)',
      );

      $form["organism-$i"]['phenotype']['file']['columns-options'] = array(
        '#type' => 'hidden',
        '#value' => $column_options,
      );

      $form["organism-$i"]['phenotype']['file']['no-header'] = array();
    }

    if (preg_match('/G/', $data_type)) {
      if ($i > 1) {
        $form["organism-$i"]['genotype-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Genotype information for $name is the same as genotype information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]}.",
          '#default_value' => isset($values["organism-$i"]['genotype-repeat-check']) ? $values["organism-$i"]['genotype-repeat-check'] : 1,
        );
      }

      $form["organism-$i"]['genotype'] = genotype($form, $form_state, $values, "organism-$i", $genotype_upload_location);

      if ($i > 1) {
        $form["organism-$i"]['genotype']['#states'] = array(
          'invisible' => array(
            ":input[name=\"organism-$i\[genotype-repeat-check]\"]" => array('checked' => TRUE),
          ),
        );
      }

    }

    if (preg_match('/E/', $data_type)) {
      if ($i > 1) {
        $form["organism-$i"]['environment-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Environmental information for $name is the same as environmental information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]}.",
          '#default_value' => isset($values["organism-$i"]['environment-repeat-check']) ? $values["organism-$i"]['environment-repeat-check'] : 1,
        );
      }

      $form["organism-$i"]['environment'] = environment($form, $form_state, $values, "organism-$i");

      if ($i > 1) {
        $form["organism-$i"]['environment']['#states'] = array(
          'invisible' => array(
            ":input[name=\"organism-$i\[environment-repeat-check]\"]" => array('checked' => TRUE),
          ),
        );
      }
    }
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

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Review Information and Submit'),
  );

  return $form;
}
