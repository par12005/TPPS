<?php

/**
 * @file
 * Creates GxPxE Data form page and includes helper files.
 */

require_once 'page_4_ajax.php';
require_once 'page_4_helper.php';

/**
 * Creates the GxPxE Data form page.
 *
 * This function creates the genotype, phenotype, and environmental fieldsets
 * based on the data type selection made on page 2. It will then call all
 * necessary helper functions.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 */
function tpps_page_4_create_form(array &$form, array &$form_state) {
  if (isset($form_state['saved_values'][TPPS_PAGE_4])) {
    $values = $form_state['saved_values'][TPPS_PAGE_4];
  }
  else {
    $values = array();
  }

  $form['#tree'] = TRUE;

  $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
  $data_type = $form_state['saved_values'][TPPS_PAGE_2]['data_type'];
  for ($i = 1; $i <= $organism_number; $i++) {

    $name = $form_state['saved_values'][TPPS_PAGE_1]['organism']["$i"];

    $form["organism-$i"] = array(
      '#type' => 'fieldset',
      '#title' => "<div class=\"fieldset-title\">$name:</div>",
      '#tree' => TRUE,
      '#collapsible' => TRUE,
    );

    if (preg_match('/P/', $data_type)) {
      if ($i > 1) {
        $form["organism-$i"]['phenotype-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Phenotype information for $name is the same as phenotype information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]}.",
          '#default_value' => isset($values["organism-$i"]['phenotype-repeat-check']) ? $values["organism-$i"]['phenotype-repeat-check'] : 1,
        );
      }

      $form["organism-$i"]['phenotype'] = tpps_phenotype($form, $form_state, $values, "organism-$i");

      if ($i > 1) {
        $form["organism-$i"]['phenotype']['#states'] = array(
          'invisible' => array(
            ":input[name=\"organism-$i\[phenotype-repeat-check]\"]" => array('checked' => TRUE),
          ),
        );
      }

      if (!isset($form["organism-$i"]['phenotype']['iso'])) {
        $image_path = drupal_get_path('module', 'tpps') . '/images/';
        $form["organism-$i"]['phenotype']['format'] = array(
          '#type' => 'radios',
          '#title' => t('Phenotype file format: *'),
          '#options' => array(
            'Type 1',
            'Type 2',
          ),
          '#ajax' => array(
            'callback' => 'tpps_phenotype_file_format_callback',
            'wrapper' => "edit-organism-$i-phenotype-file-ajax-wrapper",
          ),
          '#default_value' => (isset($form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['format'])) ? $form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['format'] : 0,
          '#description' => t('Please select a file format type from the listed options. Below please see examples of each format type.'),
          '#states' => array(
            'invisible' => array(
              ":input[name=\"organism-{$i}[phenotype][phenotypes-meta][number]\"]" => array('value' => '0'),
              ":input[name=\"organism-{$i}[phenotype][check]\"]" => array('checked' => FALSE),
            ),
          ),
        );

        $form["organism-$i"]['phenotype']['format'][0]['#prefix'] = "<figure><img src=\"{$image_path}phenotype_format_1.png\"><figcaption>";
        $form["organism-$i"]['phenotype']['format'][0]['#suffix'] = "</figcaption></figure>";
        $form["organism-$i"]['phenotype']['format'][1]['#prefix'] = "<figure><img src=\"{$image_path}phenotype_format_2.png\"><figcaption>";
        $form["organism-$i"]['phenotype']['format'][1]['#suffix'] = "</figcaption></figure>";

        $form["organism-$i"]['phenotype']['file'] = array(
          '#type' => 'managed_file',
          '#title' => t('Phenotype file: Please upload a file containing columns for Tree Identifier, Phenotype Data: *'),
          '#upload_location' => 'public://' . variable_get('tpps_phenotype_files_dir', 'tpps_phenotype'),
          '#upload_validators' => array(
            'file_validate_extensions' => array('csv tsv xlsx'),
          ),
          '#tree' => TRUE,
          '#states' => array(
            'invisible' => array(
              ":input[name=\"organism-{$i}[phenotype][phenotypes-meta][number]\"]" => array('value' => '0'),
              ":input[name=\"organism-{$i}[phenotype][check]\"]" => array('checked' => FALSE),
            ),
          ),
        );

        $form["organism-$i"]['phenotype']['file']['empty'] = array(
          '#default_value' => isset($values["organism-$i"]['phenotype']['file']['empty']) ? $values["organism-$i"]['phenotype']['file']['empty'] : 'NA',
        );

        $form["organism-$i"]['phenotype']['file']['columns'] = array(
          '#description' => 'Please define which columns hold the required data: Tree Identifier, Phenotype name, and Value(s)',
        );

        $format = tpps_get_ajax_value($form_state, array(
          "organism-$i",
          'phenotype',
          'format',
        ), 0);

        if ($format == 0) {
          $column_options = array(
            'Phenotype Data',
            'Tree Identifier',
            'Timepoint',
            'Clone Number',
            'N/A',
          );
        }
        else {
          $column_options = array(
            'N/A',
            'Tree Identifier',
            'Phenotype Name/Identifier',
            'Value(s)',
            'Timepoint',
            'Clone Number',
          );
          $form["organism-$i"]['phenotype']['file']['#title'] = t('Phenotype file: Please upload a file containing columns for Tree Identifier, Phenotype Name, and value for all of your phenotypic data: *');
        }

        $form["organism-$i"]['phenotype']['file']['columns-options'] = array(
          '#type' => 'hidden',
          '#value' => $column_options,
        );

        $form["organism-$i"]['phenotype']['file']['no-header'] = array();
      }
    }

    if (preg_match('/G/', $data_type)) {
      if ($i > 1) {
        $form["organism-$i"]['genotype-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Genotype information for $name is the same as genotype information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]}.",
          '#default_value' => isset($values["organism-$i"]['genotype-repeat-check']) ? $values["organism-$i"]['genotype-repeat-check'] : 1,
        );
      }

      $form["organism-$i"]['genotype'] = tpps_genotype($form, $form_state, $values, "organism-$i");

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

      $form["organism-$i"]['environment'] = tpps_environment($form, $form_state, "organism-$i");

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
}
