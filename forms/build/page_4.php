<?php

/**
 * @file
 * Creates GxPxE Data form page and includes helper files.
 */

require_once 'page_4_ajax.php';
require_once 'page_4_helper.php';
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
  global $user;

  // Store valueable data to the $chest.
  $chest = ['form' => &$form, 'form_state' => &$form_state];
  for ($i = 1; $i <= 4; $i++) {
    if (isset($form_state['saved_values'][$i])) {
      $chest['page' . $i . '_values'] = &$form_state['saved_values'][$i];
    }
  }
  $form['#tree'] = TRUE;
  for ($i = 1; $i <= tpps_chest_get($chest, 'organism_count'); $i++) {
    $chest['organism_id'] = $i;
    $name = tpps_chest_get($chest, 'organism_name', $i);
    $form["organism-$i"] = [
      '#type' => 'fieldset',
      '#title' => t($name),
      '#tree' => TRUE,
      '#collapsible' => TRUE,
    ];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Data Type includes phenotype.
    if (tpps_is_phenotype_data_type($form_state)) {
      tpps_page4_add_data_type(array_merge(
        $chest, ['type' => 'phenotype', 'type_name' => t('Phenotype')]
      ));
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
          '#default_value' => ($chest['page4_values']["organism-$i"]['phenotype']['format'] ?? 0),
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
        $phenotype_dir = variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');
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
          '#upload_location' => 'public://' . $phenotype_dir,
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
          '#default_value' => $chest['page4_values']["organism-$i"]['phenotype']['file']['empty'] ?? t('NA'),
        ];

        $form["organism-$i"]['phenotype']['file']['columns'] = [
          '#description' => t('Please define which columns hold the required '
            . 'data: Plant Identifier, Phenotype name, and Value(s)'
          ),
        ];

        $format = tpps_get_ajax_value(
          $form_state, ["organism-$i", 'phenotype', 'format'], 0
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
        // Ability to use file without header was disabled to make validation
        // simplier and to avoid problems with processing those files.
        // File without header will not pass validation.
        //$form["organism-$i"]['phenotype']['file']['no-header'] = [];
      }

      // This will check if there are Time Phenotypes saved from the
      // saved values and use this to re-check the checkboxes on the form.
      if (isset($chest['page4_values']["organism-$i"]['phenotype']['time']['time_phenotypes'])) {
        $time_phenotypes = $chest['page4_values']["organism-$i"]['phenotype']['time']['time_phenotypes'] ?? [];
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
    if (tpps_is_genotype_data_type($form_state)) {
      tpps_page4_add_data_type(array_merge(
        $chest, ['type' => 'genotype', 'type_name' => t('Genotype')]
      ));
    }
    if (tpps_is_environment_data_type($form_state)) {
      tpps_page4_add_data_type(array_merge(
        $chest, ['type' => 'environment', 'type_name' => t('Environmental')]
      ));
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Button's weight: -1000 (header) and 1000 (footer).
  tpps_form_add_buttons(array_merge($chest, ['page' => 'page_4']));
  // Curation Tool's weight: 1100 (under button's in footer).
  tpps_add_curation_tool($chest);
  // Now JS is empty but could be used later.
  tpps_add_css_js(TPPS_PAGE_4, $form);
  return $form;
}

/**
 * Generates a Curation Diagnostic Tool form.
 *
 * Note: Form has 7 buttons.
 *
 * @param array $chest
 *   Container for a data. Keys are:
 *   'form', 'form_state'.
 */
function tpps_add_curation_tool(array $chest) {
  global $user;
  // Only for curation team and admins.
  if (!in_array('administrator', $user->roles) && !in_array('Curation', $user->roles)) {
    return;
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
 * @param array $chest
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
function tpps_page4_add_data_type(array $chest) {
  $i = $chest['organism_id'] ?? 0;
  $organism_name = 'organism-' . $i;
  $form = &$chest['form'] ?? [];
  $form_state = &$chest['form_state'] ?? [];

  $type = $chest['type'] ?? '';
  $type_name = $chest['type_name'] ?? '';

  // List of dynamically build function names for code management:
  // tpps_phenotype(),
  // tpps_genotype() or tpps_genotype_subform(),
  // tpps_environment().
  if ($type == 'genotype') {
    $function_name = 'tpps_' . $type . '_subform';
    call_user_func($function_name, $chest);
  }
  elseif (in_array($type, ['environment', 'phenotype'])) {
    $function_name = 'tpps_' . $type;
    // @TODO Rename all functions to have suffix '_subform'.
    if ($type == 'environment') {
      $args = [&$form, &$form_state, $organism_name];
    }
    elseif ($type == 'phenotype') {
      $args = [&$form, &$form_state, $chest['page4_values'], $organism_name];
    }
    $field = call_user_func_array($function_name, $args);
    $form[$organism_name][$type] = $field;
  }
  else {
    $message = t('Unsupported data type: @type.', ['@type' => $type]);
    drupal_set_message($message, 'error');
    return;
  }
  // Main fields.
  // Repeat check.
  if ($i > 1) {
    $form[$organism_name][$type . '-repeat-check'] = [
      '#type' => 'checkbox',
      '#title' => t('@type_name information for <strong>@current_organism_name'
        . '</strong> is the same as @type_lower_name information for <strong>'
        . '@prev_organism_name</strong>.',
        [
          '@type_name' => ucfirst($type_name),
          '@type_lower_name' => strtolower($type_name),
          '@current_organism_name' => tpps_chest_get($chest, 'organism_name', $i),
          '@prev_organism_name' => tpps_chest_get($chest, 'organism_name', ($i - 1)),
        ]
      ),
      '#default_value' => ($chest['page4_values'][$organism_name][$type . '-repeat-check'] ?? 1),
    ];
    $form[$organism_name][$type]['#states'] = [
      'invisible' => [
        ':input[name="' . $organism_name
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
