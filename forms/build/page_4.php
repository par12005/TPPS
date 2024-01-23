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
  global $user;
  if (isset($form_state['saved_values'][TPPS_PAGE_4])) {
    $values = $form_state['saved_values'][TPPS_PAGE_4];
  }
  else {
    $values = array();
  }
  // dpm("FORM CREATED SAVED VALUES:");
  // dpm($values);

  $form['#tree'] = TRUE;
  // Submission metadata.
  $meta = [
    'organism_number' => $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'],
    'data_type' => $form_state['saved_values'][TPPS_PAGE_2]['data_type'],
  ];

  for ($i = 1; $i <= $meta['organism_number']; $i++) {
    $name = $form_state['saved_values'][TPPS_PAGE_1]['organism']["$i"]['name'];
    $form["organism-$i"] = array(
      '#type' => 'fieldset',
      '#title' => "<div class=\"fieldset-title\">$name:</div>",
      '#tree' => TRUE,
      '#collapsible' => TRUE,
    );
    if (preg_match('/P/', $meta['data_type'])) {
      if ($i > 1) {
        $form["organism-$i"]['phenotype-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Phenotype information for $name is the same as phenotype information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]['name']}.",
          '#default_value' => ($values["organism-$i"]['phenotype-repeat-check'] ?? 1),
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

      $normal_check = tpps_get_ajax_value($form_state, array(
        "organism-$i",
        'phenotype',
        'normal-check',
      ), TRUE);
      if (!empty($normal_check)) {
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

        $form["organism-$i"]['phenotype']['format'][0]['#prefix'] =
          '<figure><img src="/' . TPPS_IMAGES_PATH . 'phenotype_format_1.png">'
          . '<figcaption></figcaption></figure>';
        $form["organism-$i"]['phenotype']['format'][1]['#prefix'] =
          '<figure><img src="/' . TPPS_IMAGES_PATH . 'phenotype_format_2.png">'
          . '<figcaption></figcaption></figure>';

        $form["organism-$i"]['phenotype']['file'] = array(
          '#type' => 'managed_file',
          '#title' => t('Phenotype file: Please upload a file containing columns for Plant Identifier, Phenotype Data: *'),
          '#upload_location' => 'public://' . variable_get('tpps_phenotype_files_dir', 'tpps_phenotype'),
          '#upload_validators' => array(
            'file_validate_extensions' => array('csv tsv'),
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
          '#description' => t('Please define which columns hold the required data: Plant Identifier, Phenotype name, and Value(s)'),
        );


        $format = tpps_get_ajax_value($form_state, array(
          "organism-$i",
          'phenotype',
          'format',
        ), 0);

        if ($format == 0) {
          $column_options = array(
            'Phenotype Data',
            'Plant Identifier',
            'Timepoint',
            'Clone Number',
            'N/A',
          );
        }
        else {
          $column_options = array(
            'N/A',
            'Plant Identifier',
            'Phenotype Name/Identifier',
            'Value(s)',
            'Timepoint',
            'Clone Number',
          );
          $form["organism-$i"]['phenotype']['file']['#title'] = t('Phenotype file: Please upload a file containing columns for Plant Identifier, Phenotype Name, and value for all of your phenotypic data: *');
        }

        $form["organism-$i"]['phenotype']['file']['columns-options'] = array(
          '#type' => 'hidden',
          '#value' => $column_options,
        );

        $form["organism-$i"]['phenotype']['file']['no-header'] = array();
      }

      // This will check if there are Time Phenotypes saved from the saved values
      // and use this to re-check the checkboxes on the form
      if (isset($form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['time']['time_phenotypes'])) {
        $count_time_phenotypes = count($form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['time']['time_phenotypes']);
        // dpm('TIME PHENOTYPES DETECTED: ' . $count_time_phenotypes);
        if($count_time_phenotypes > 0) {
          $time_phenotypes = $form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['time']['time_phenotypes'];
          // dpm($time_phenotypes);
          // Go through all time phenotypes
          foreach ($time_phenotypes as $time_phenotype) {
            // dpm($time_phenotype);
            // if time_phenotype is not 0, meaning the phenotype name and phenotype value would be the same, so not 0
            if($time_phenotype != "0") {
              // dpm($time_phenotype);
              // Adjust the checkbox element on the form to TRUE (checkbox)
              $form["organism-$i"]['phenotype']['time']['time_phenotypes'][$time_phenotype]['#default_value'] = TRUE;
            }
          }
        }
      }
    }

    if (preg_match('/G/', $meta['data_type'])) {
      if ($i > 1) {
        $form["organism-$i"]['genotype-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Genotype information for $name is the same as genotype information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]['name']}.",
          '#default_value' => ($values["organism-$i"]['genotype-repeat-check'] ?? 1),
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

    if (preg_match('/E/', $meta['data_type'])) {
      if ($i > 1) {
        $form["organism-$i"]['environment-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Environmental information for $name is the same as "
            . "environmental information for "
            . "{$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]['name']}.",
          '#default_value' => ($values["organism-$i"]['environment-repeat-check'] ?? 1),
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

  // [VS].
  tpps_add_buttons($form, 'page_4', $meta);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Diagnostic utilities for curation.
  if (
    in_array('administrator', $user->roles)
    || in_array('Curation', $user->roles)
  ) {
    $js_data = [
      'tpps' => [
        'accession' => $form_state['accession'],
        'curationDiagnosticResultsElementId' => '#diagnostic-curation-results',
        'organismNumber' => $meta['organism_number'],
      ],
    ];
    $form['#attached']['js'][] = ['type' => 'setting', 'data' => $js_data];
    tpps_add_css_js(TPPS_PAGE_4, $form);
    tpps_add_curation_tool($form);
  }
  return $form;
}

/**
 * Generates a Curation Diagnostic Tool form.
 *
 * Form has 7 buttons.
 *
 * @param array $form
 *   Drupal Form API array.
 */
function tpps_add_curation_tool(array &$form) {
  $form['diagnostics-curation'] = [
    '#type' => 'fieldset',
    '#title' => '🌟 Curation Diagnostics',
    '#description' => 'These diagnostics <b>require you to save this package</b> '
      . 'with data before functions will work',
    // Must be below navigation buttons Back/Next which has weight 100.
    '#weight' => 200,
  ];

  // 1st row of buttons.
  tpps_add_curation_tool_button(
    $form,
    'button-check-accession-file-tree-ids',
    'Check Accession File Tree Ids'
  );
  tpps_add_curation_tool_button(
    $form, 'button-check-vcf-tree-ids', 'Check VCF Tree IDs'
  );
  tpps_add_curation_tool_button(
    $form,
    'button-compare-accession-tree-ids-vs-vcf-tree-ids',
    'Compare Accession and VCF Tree IDs'
  );
  // 2nd row of buttons.
  tpps_add_curation_tool_button(
    $form, 'button-check-vcf-markers', 'Check VCF Markers'
  );
  tpps_add_curation_tool_button(
    $form, 'button-check-snps-assay-markers', 'Check SNPs Assay Markers'
  );
  tpps_add_curation_tool_button(
    $form, 'button-check-snps-design-markers', 'Check SNPs Design Markers'
  );
  // 3rd row of buttons.
  tpps_add_curation_tool_button(
    $form,
    'button-compare-vcf-makers-vs-snps-assay-markers',
    'Compare VCF and SNPs Assay markers'
  );

  $form['diagnostics-curation']['diagnostic-curation-results'] = [
    '#type' => 'container',
    '#attributes' => ['id' => 'diagnostic-curation-results'],
  ];
}

/**
 * Generates an action button for Curation Diagnostic Tool.
 *
 * @param array $form
 *   Drupal Form API array.
 * @param string $key
 *   Unique button key.
 * @param string $name
 *   Human readable name of button.
 */
function tpps_add_curation_tool_button(array &$form, $key, $name) {
  $form['diagnostics-curation'][$key] = [
    '#type' => 'button',
    '#value' => t($name),
    '#attributes' => ['class' => [$key, 'form-button']],
  ];
}
// [/VS].
