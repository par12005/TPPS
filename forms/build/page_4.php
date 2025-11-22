<?php

/**
 * @file
 * Creates GxPxE Data form page and includes helper files.
 */

require_once 'page_4_ajax.php';
require_once 'page_4_genotype.php';
require_once 'page_4_environment.php';
require_once 'page_4_phenotype.php';

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
  // Store valueable data to the $form_bus.
  $form_bus = ['form' => &$form, 'form_state' => &$form_state];
  for ($i = 1; $i <= 4; $i++) {
    if (isset($form_state['saved_values'][$i])) {
      $form_bus['page' . $i . '_values'] = &$form_state['saved_values'][$i];
    }
    else {
      $form_bus['page' . $i . '_values'] = [];
    }
  }
  $form['#tree'] = TRUE;
  for ($i = 1; $i <= tpps_form_bus_get($form_bus, 'organism_number'); $i++) {
    $organism_key = 'organism-' . $i;
    $form_bus['organism_id'] = $i;
    $name = tpps_form_bus_get($form_bus, 'organism_name', $i);
    $form[$organism_key] = [
      '#type' => 'fieldset',
      '#title' => t(strtoupper($name)),
      '#tree' => TRUE,
      '#collapsible' => TRUE,
    ];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Data Type includes phenotype.
    if (tpps_is_phenotype_data_type($form_state)) {
      tpps_page4_add_data_type(array_merge(
        $form_bus, ['type' => 'phenotype', 'type_name' => t('Phenotype')]
      ));
      PhenotypeData::build($i, $form, $form_state);
    }
    if (tpps_is_genotype_data_type($form_state)) {
      tpps_page4_add_data_type(array_merge(
        $form_bus, ['type' => 'genotype', 'type_name' => t('Genotype')]
      ));
    }
    if (tpps_is_environment_data_type($form_state)) {
      tpps_page4_add_data_type(array_merge(
        $form_bus, ['type' => 'environment', 'type_name' => t('Environmental')]
      ));
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Button's weight: -1000 (header) and 1000 (footer).
  tpps_form_add_buttons(array_merge($form_bus, [
    'page' => 'page_4',
    'organism_number' => tpps_form_bus_get($form_bus, 'organism_number'),
  ]));
  // Curation Tool's weight: 1100 (under button's in footer).
  tpps_add_curation_tool($form_bus);
  // Now JS is empty but could be used later.
  tpps_add_css_js(TPPS_PAGE_4, $form);
  return $form;
}

/**
 * Generates a Curation Diagnostic Tool form.
 *
 * Note: Form has 7 buttons.
 *
 * @param array $form_bus
 *   Container for a data. Keys are:
 *   'form', 'form_state'.
 */
function tpps_add_curation_tool(array $form_bus) {
  if (!tpps_is_admin_or_curation()) {
    return;
  }
  $form = &$form_bus['form'];
  $form_state = $form_bus['form_state'];
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
    '#title' => '🌟 CURATION DIAGNOSTICS',
    '#description' => 'These diagnostics <b>require you to save this package</b> '
      . 'with data before functions will work',
    // Must be below navigation buttons Back/Next which has weight 1000 in footer.
    '#weight' => 1100,
  ];
  $list = [
    // 1st row of buttons.
    'button-check-accession-file-tree-ids' => 'Check Accession File Tree Ids',
    'button-check-vcf-tree-ids' => 'Check VCF Tree IDs',
    'button-compare-accession-tree-ids-vs-vcf-tree-ids' => 'Compare Accession and VCF Tree IDs',
    // 2nd row of buttons.
    'button-check-vcf-markers' => 'Check VCF Markers',
    'button-check-snps-assay-markers' => 'Check SNPs Assay Markers',
    'button-check-snps-design-markers' => 'Check SNPs Design Markers',
    // 3rd row of buttons.
    'button-compare-vcf-makers-vs-snps-assay-markers' => 'Compare VCF and SNPs Assay markers',
  ];
  // Add an action buttons.
  foreach ($list as $key => $title) {
    $form['diagnostics-curation'][$key] = [
      '#type' => 'button',
      '#value' => t($title),
      '#attributes' => ['class' => [$key, 'form-button']],
    ];
  }
  $form['diagnostics-curation']['diagnostic-curation-results'] = [
    '#type' => 'container',
    '#attributes' => ['id' => 'diagnostic-curation-results'],
  ];
  tpps_add_css_js('page_4_curation_tool', $form);
}

/**
 * Adds feature '... information is the same as ...'.
 *
 * @param array $form_bus
 *   Metadata where keys are:
 *   'form' array
 *     Drupal Form Array passed by reference. E.g., &$form.
 *   'form_state' array
 *     Drupal Form State Array. E.g., $form_state.
 *
 *   Specific data:
 *   'organism_id' int
 *     Currently processed organism number.
 *
 *   'name'string
 *     Localized name of data type. E.g., t('Environmental').
 *
 *   'type' string
 *     Machine data type name. E.g., 'phenotype'.
 *   'type_name' string
 *     Localized human readable data type name. E.g., 'Phenotype'.
 */
function tpps_page4_add_data_type(array $form_bus) {
  $i = $form_bus['organism_id'] ?? 0;
  $organism_key = 'organism-' . $i;
  $form = &$form_bus['form'] ?? [];
  $form_state = &$form_bus['form_state'] ?? [];

  $type = $form_bus['type'] ?? '';
  $type_name = $form_bus['type_name'] ?? '';

  // List of dynamically build function names for code management:
  // tpps_phenotype(),
  // tpps_genotype() or tpps_genotype_subform(),
  // tpps_environment().
  if ($type == 'genotype') {
    $function_name = 'tpps_' . $type . '_subform';
    call_user_func($function_name, $form_bus);
  }
  elseif (in_array($type, ['environment', 'phenotype'])) {
    $function_name = 'tpps_' . $type;
    // @TODO Rename all functions to have suffix '_subform'.
    if ($type == 'environment') {
      $args = [&$form, &$form_state, $organism_key];
    }
    elseif ($type == 'phenotype') {
      $args = [&$form, &$form_state, $form_bus['page4_values'], $organism_key];
    }
    $field = call_user_func_array($function_name, $args);
    $form[$organism_key][$type] = $field;
  }
  else {
    $message = t('Unsupported data type: @type.', ['@type' => $type]);
    drupal_set_message($message, 'error');
    return;
  }
  // Main fields.
  // Repeat check.
  if ($i > 1) {
    $form[$organism_key][$type . '-repeat-check'] = [
      '#type' => 'checkbox',
      '#title' => t('@type_name information for <strong>@current_organism_name'
        . '</strong> is the same as @type_lower_name information for <strong>'
        . '@prev_organism_name</strong>.',
        [
          '@type_name' => ucfirst($type_name),
          '@type_lower_name' => strtolower($type_name),
          '@current_organism_name' => tpps_form_bus_get($form_bus, 'organism_name', $i),
          '@prev_organism_name' => tpps_form_bus_get($form_bus, 'organism_name', ($i - 1)),
        ]
      ),
      '#default_value' => ($form_bus['page4_values'][$organism_key][$type . '-repeat-check'] ?? 1),
    ];
    $form[$organism_key][$type]['#states'] = [
      'invisible' => [
        ':input[name="' . $organism_key
        . '[' . $type . '-repeat-check]"]' => ['checked' => TRUE],
      ],
    ];
  }
}

/**
 * Checks if study has phenotype data.
 *
 * @param array $form_state
 *   Drupal Form State.
 *
 * @return bool
 *   Returns TRUE if it has and FALSE otherwise.
 *
 * @todo Better to detect is once on Page 2 submit and store in $form_state.
 */
function tpps_is_phenotype_data_type(array $form_state) {
  $data_type = $form_state['saved_values'][TPPS_PAGE_2]['data_type'] ?? '';
  return (bool) preg_match('/P/', $data_type);
}

/**
 * Checks if study has genotype data.
 *
 * @param array $form_state
 *   Drupal Form State.
 *
 * @return bool
 *   Returns TRUE if it has and FALSE otherwise.
 *
 * @todo Better to detect is once on Page 2 submit and store in $form_state.
 */
function tpps_is_genotype_data_type(array $form_state) {
  $data_type = $form_state['saved_values'][TPPS_PAGE_2]['data_type'] ?? '';
  return (bool) preg_match('/G/', $data_type);
}

/**
 * Checks if study has environment data.
 *
 * @param array $form_state
 *   Drupal Form State.
 *
 * @return bool
 *   Returns TRUE if it has and FALSE otherwise.
 *
 * @todo Better to detect is once on Page 2 submit and store in $form_state.
 */
function tpps_is_environment_data_type(array $form_state) {
  $data_type = $form_state['saved_values'][TPPS_PAGE_2]['data_type'] ?? '';
  return (bool) preg_match('/E/', $data_type);
}
