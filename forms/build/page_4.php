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
  $page1_values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];
  $page2_values = $form_state['saved_values'][TPPS_PAGE_2] ?? [];
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4] ?? [];

  $organism_number = $page1_values['organism']['number'];
  $data_type = $page2_values['data_type'];

  $form['#tree'] = TRUE;
  for ($i = 1; $i <= $organism_number; $i++) {
    $name = $page1_values['organism'][$i]['name'];
    $form["organism-$i"] = [
      '#type' => 'fieldset',
      '#title' => "<div class=\"fieldset-title\">$name:</div>",
      '#tree' => TRUE,
      '#collapsible' => TRUE,
    ];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Data Type includes phenotype.
    if (preg_match('/P/', $data_type)) {
      tpps_page4_add_data_type([
        'type' => 'phenotype',
        'type_name' => t('Phenotype'),
        'i' => $i,
        'form' => &$form,
        'form_state' => $form_state,
      ]);

      $normal_check = tpps_get_ajax_value(
        $form_state,
        ["organism-$i", 'phenotype', 'normal-check'],
        TRUE
      );
      if (!empty($normal_check)) {
        $form["organism-$i"]['phenotype']['format'] = [
          '#type' => 'radios',
          '#title' => t('Phenotype file format: *'),
          '#options' => ['Type 1', 'Type 2'],
          '#ajax' => [
            'callback' => 'tpps_phenotype_file_format_callback',
            'wrapper' => "edit-organism-$i-phenotype-file-ajax-wrapper",
          ],
          '#default_value' => ($page4_values["organism-$i"]['phenotype']['format'] ?? 0),
          '#description' => t('Please select a file format type from the '
            . 'listed options. Below please see examples of each format type.'
          ),
          '#states' => [
            'invisible' => [
              ":input[name=\"organism-{$i}[phenotype][phenotypes-meta][number]\"]" => ['value' => '0'],
              ":input[name=\"organism-{$i}[phenotype][check]\"]" => ['checked' => FALSE],
            ],
          ],
        ];

        $form["organism-$i"]['phenotype']['format'][0]['#prefix'] =
          '<figure><img src="/' . TPPS_IMAGES_PATH . 'phenotype_format_1.png">'
          . '<figcaption></figcaption></figure>';
        $form["organism-$i"]['phenotype']['format'][1]['#prefix'] =
          '<figure><img src="/' . TPPS_IMAGES_PATH . 'phenotype_format_2.png">'
          . '<figcaption></figcaption></figure>';

        $form["organism-$i"]['phenotype']['file'] = [
          '#type' => 'managed_file',
          '#title' => t('Phenotype file: Please upload a file containing '
            . 'columns for Plant Identifier, Phenotype Data: *'),
          '#upload_location' => 'public://'
            . variable_get('tpps_phenotype_files_dir', 'tpps_phenotype'),
          '#upload_validators' => ['file_validate_extensions' => ['csv tsv']],
          '#tree' => TRUE,
          '#states' => [
            'invisible' => [
              ":input[name=\"organism-{$i}[phenotype][phenotypes-meta][number]\"]" => ['value' => '0'],
              ":input[name=\"organism-{$i}[phenotype][check]\"]" => ['checked' => FALSE],
            ],
          ],
        ];

        $form["organism-$i"]['phenotype']['file']['empty'] = [
          '#default_value' => $page4_values["organism-$i"]['phenotype']['file']['empty'] ?? t('NA'),
        ];

        $form["organism-$i"]['phenotype']['file']['columns'] = [
          '#description' => t('Please define which columns hold the required '
            . 'data: Plant Identifier, Phenotype name, and Value(s)'
          ),
        ];

        $format = tpps_get_ajax_value(
          $form_state,
          ["organism-$i", 'phenotype', 'format'],
          0
        );

        if ($format == 0) {
          $column_options = [
            'Phenotype Data',
            'Plant Identifier',
            'Timepoint',
            'Clone Number',
            'N/A',
          ];
        }
        else {
          $column_options = [
            'N/A',
            'Plant Identifier',
            'Phenotype Name/Identifier',
            'Value(s)',
            'Timepoint',
            'Clone Number',
          ];
          $form["organism-$i"]['phenotype']['file']['#title'] =
            t('Phenotype file: Please upload a file containing columns '
              . 'for Plant Identifier, Phenotype Name, and value for all '
              . 'of your phenotypic data: *'
          );
        }
        $form["organism-$i"]['phenotype']['file']['columns-options'] = [
          '#type' => 'hidden',
          '#value' => $column_options,
        ];
        $form["organism-$i"]['phenotype']['file']['no-header'] = [];
      }

      // This will check if there are Time Phenotypes saved from the
      // saved values and use this to re-check the checkboxes on the form.
      if (isset($page4_values["organism-$i"]['phenotype']['time']['time_phenotypes'])) {
        $time_phenotypes = $page4_values["organism-$i"]['phenotype']['time']['time_phenotypes'] ?? [];
        $count_time_phenotypes = count($time_phenotypes);
        if ($count_time_phenotypes > 0) {
          foreach ($time_phenotypes as $time_phenotype) {
            // If time_phenotype is not 0, meaning the phenotype name
            // and phenotype value would be the same, so not 0.
            if ($time_phenotype != "0") {
              // Adjust the checkbox element on the form to TRUE (checkbox).
              $form["organism-$i"]['phenotype']['time']['time_phenotypes'][$time_phenotype]['#default_value'] = TRUE;
            }
          }
        }
      }
    }

    // Data Type includes Genotype.
    if (preg_match('/G/', $data_type)) {
      tpps_form_add_yesno_field([
        'field_name' => 'genotype_are_markers_identical',
        '#title' => t('Are your genotype markers identical accross species?'),
        '#width' => 100,
        'form' => &$form,
        'form_state' => &$form_state,
      ]);
      tpps_page4_add_data_type([
        'type' => 'genotype',
        'type_name' => t('Genotype'),
        'i' => $i,
        'form' => &$form,
        'form_state' => $form_state,
      ]);
    }
    // Data Type contains 'Environment'.
    if (preg_match('/E/', $data_type)) {
      tpps_page4_add_data_type([
        'type' => 'environment',
        'type_name' => t('Environmental'),
        'i' => $i,
        'form' => &$form,
        'form_state' => $form_state,
      ]);
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  tpps_form_add_buttons([
    'form' => &$form,
    'page' => 'page_4',
    'organism_number' => $organism_number,
    'data_type' => $data_type,
  ]);
  tpps_add_curation_tool([
    'form' => &$form,
    'form_state' => $form_state,
  ]);
  tpps_add_css_js(TPPS_PAGE_4, $form);
  return $form;
}

/**
 * Generates a Curation Diagnostic Tool form.
 *
 * Form has 7 buttons.
 *
 * @param array $chest
 *   Container for a data. Keys are:
 *   'form', 'form_state'.
 */
function tpps_add_curation_tool(array $chest) {
  global $user;
  // Only for curation team and admins.
  if (!in_array('administrator', $user->roles) && !in_array('Curation', $user->roles)) {
    //return;
  }
  $form = &$chest['form'];
  $form_state = $chest['form_state'];
  $form['#attached']['js'][] = [
    'type' => 'setting',
    'data' => [
      'tpps' => [
        'accession' => $form_state['accession'],
        'curationDiagnosticResultsElementId' => '#diagnostic-curation-results',
        'organismNumber' => $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'],
      ],
    ],
  ];

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
    '#attributes' => ['class' => [$key]],
  ];
}

/**
 * Adds feature '... information is the same as ...'.
 *
 * @param array $chest
 *   Metadata where keys are:
 *     'type' string Data type. E.g., 'environment'.
 *     'name'string Localized name of data type. E.g., t('Environmental').
 *     'i' int Number of organism.
 *     'form' array Drupal Form Array passed by reference. E.g., &$form.
 *     'form_state' array Drupal Form State Array. E.g., $form_state.
 */
function tpps_page4_add_data_type(array $chest) {
  $i = $chest['i'] ?? 0;
  $organism_name = 'organism-' . $i;
  $form = &$chest['form'] ?? [];
  $form_state = $chest['form_state'] ?? [];
  $page1_values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4] ?? [];
  $type = $chest['type'] ?? '';
  $type_name = $chest['type_name'] ?? '';

  // List of dynamically build function names for code management:
  // tpps_phenotype(),
  // tpps_genotype(),
  // tpps_environment().
  $function_name = 'tpps_' . $type;
  if ($type == 'environment') {
    $args = [&$form, &$form_state, $organism_name];
  }
  elseif (in_array($type, ['phenotype', 'genotype'])) {
    $args = [&$form, &$form_state, $page4_values, $organism_name];
  }
  else {
    $message = t('Unsupported data type: @type.', ['@type' => $type]);
    drupal_set_message($message, 'error');
    return;
  }
  $form[$organism_name][$type] = call_user_func_array($function_name, $args);
  if ($i > 1) {
    $form[$organism_name][$type . '-repeat-check'] = [
      '#type' => 'checkbox',
      '#title' => t('@type_name information for <strong>@current_organism_name'
        . '</strong> is the same as @type_lower_name information for <strong>'
        . '@prev_organism_name</strong>.',
        [
          '@type_name' => ucfirst($type_name),
          '@type_lower_name' => strtolower($type_name),
          '@current_organism_name' => $page1_values['organism'][$i]['name'] ?? '',
          '@prev_organism_name' => $page1_values['organism'][$i - 1]['name'] ?? '',
        ]
      ),
      '#default_value' => ($page4_values[$organism_name][$type . '-repeat-check'] ?? 1),
      '#weight' => -100,
    ];
    $form[$organism_name][$type]['#states'] = [
      'invisible' => [
        ':input[name="' . $organism_name
        . '[' . $type . '-repeat-check]"]' => ['checked' => TRUE],
      ],
    ];
  }
}
// [/VS].
