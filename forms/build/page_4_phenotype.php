<?php
/**
 * @file
 * Define the helper functions for the GxPxE Data page.
 */

/**
 * Creates fields describing the phenotype data for the submission.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_phenotype(array &$form, array &$form_state, array $values, $id) {
  $phenotype_dir = variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');

  $form[$id]['phenotype'] = [
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">PHENOTYPE INFORMATION:</div>'),
    '#tree' => TRUE,
    '#prefix' => "<div id=\"phenotype-main-$id\">",
    '#suffix' => '</div>',
    '#description' => t('Upload a file and/or fill in form fields below '
      . 'to provide us with metadata about your phenotypes.'),
    '#collapsible' => TRUE,
  ];
  // 'Normal Check' must be enabled by default to have buttons
  // to add phenotypes shown.
  $normal_check = tpps_get_ajax_value(
    $form_state, [$id, 'phenotype', 'normal-check'], TRUE
  );

  $form[$id]['phenotype']['normal-check'] = [
    '#type' => 'checkbox',
    '#title' => t('My phenotypes include traits and/or environmental '
      . 'information other than mass spectrometry or isotope analysis'),
    '#ajax' => [
      'callback' => 'tpps_update_phenotype',
      'wrapper' => "phenotype-main-$id",
      'effect' => 'slide',
    ],
    '#default_value' => $normal_check,
  ];

  $form[$id]['phenotype']['iso-check'] = [
    '#type' => 'checkbox',
    '#title' => t('My phenotypes include results from a mass spectrometry or isotope analysis'),
    '#ajax' => [
      'callback' => 'tpps_update_phenotype',
      'wrapper' => "phenotype-main-$id",
      'effect' => 'slide',
    ],
  ];

  $iso_check = tpps_get_ajax_value(
    $form_state, [$id, 'phenotype', 'iso-check'], NULL
  );
  if (!empty($iso_check)) {
    $form[$id]['phenotype']['iso'] = array(
      '#type' => 'managed_file',
      '#title' => t('Phenotype Isotope/Mass Spectrometry file: *'),
      '#upload_location' => 'public://' . $phenotype_dir,
      '#upload_validators' => ['file_validate_extensions' => ['csv tsv']],
      '#description' => t('Please upload a file containing all of your '
        . 'isotope/mass spectrometry data. The format of this file is very '
        . 'important! The first column of your file should contain plant '
        . 'identifiers which match the plant identifiers you provided in '
        . 'your plant accession file, and all of the remaining columns '
        . 'should contain isotope or mass spectrometry data.'),
    );
  }

  if (!empty($normal_check)) {
    // @TODO Use static call of build() method.
    $field = tpps_phenotype_manual()->build($id);
    // Loop phenotypes to get unique form fields for each phenotype.
    tpps_dynamic_list($form, $form_state, 'phenotypes-meta', $field, array(
      'label' => 'Phenotype',
      'title' => "",
      'callback' => 'tpps_update_phenotype',
      'parents' => [$id, 'phenotype'],
      'wrapper' => "phenotype-main-$id",
      'states' => [
        'visible' => [
          ':input[name="' . $id . '[phenotype][check]"]' => ['checked' => FALSE],
        ],
      ],
      'name_suffix' => $id,
      // [VS] #8669py3z7
      'alternative_buttons' => [
        "Clear All Phenotypes" => 'tpps_phenotype_number_clear',
      ],
      'button_weights' => [
        "Add Phenotype" => -5,
        "Remove Phenotype" => -2,
        "Clear All Phenotypes" => -1,
      ],
      // Replaces '!num'.
      'substitute_fields' => array(
        // Synonym form.
        ['synonym_name', '#title'],
        ['synonym_name', '#prefix'],
        ['synonym_description', '#title'],
        ['synonym_description', '#description'],
        ['synonym_id', '#ajax', 'wrapper'],
      // [/VS]

        // Main form.
        array('#prefix'),
        array('name', '#title'),
        array('name', '#prefix'),
        array('env-check', '#title'),
        array('env-check', '#ajax', 'wrapper'),
        array('attribute', '#title'),
        array('attribute', '#ajax', 'wrapper'),
        array('attr-other', '#title'),
        array('description', '#title'),
        array('description', '#description'),
        // [VS] #8669rmrw5
        ['unit', '#title'],
        ['unit', '#prefix'],
        ['unit-other', '#title'],
        array('structure', '#title'),
        array('struct-other', '#title'),
        // [/VS]
      ),
      // [VS] Replace '!num' in attributes.
      'substitute_keys' => array(
        // Synonym form.
        ['synonym_name', '#states', 'visible', tpps_synonym_selector($id)],
        ['synonym_description', '#states', 'visible', tpps_synonym_selector($id)],
        // State of the Main form related to Synonym form.
        array('name', '#states', 'visible', tpps_synonym_selector($id)),
        array('env-check', '#states', 'visible', tpps_synonym_selector($id)),
        array('attribute', '#states', 'visible', tpps_synonym_selector($id)),
        array('attr-other', '#states', 'visible', tpps_synonym_selector($id)),
        array('description', '#states', 'visible', tpps_synonym_selector($id)),
        array('structure', '#states', 'visible', tpps_synonym_selector($id)),
        // Main form.
        array(
          'attr-other',
          '#states',
          'visible',
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][attribute]"]',
        ),
        array(
          'unit-other',
          '#states',
          'visible',
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][unit]"]',
        ),
        array(
          'struct-other',
          '#states',
          'visible',
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][structure]"]',
        ),
      ),
    )); // End of tpps_dynamic_list().

    $phenotypes = tpps_get_ajax_value($form_state, array(
      $id,
      'phenotype',
      'phenotypes-meta',
    ), NULL);
    $phenotype_number = tpps_get_ajax_value($form_state, array(
      $id,
      'phenotype',
      'phenotypes-meta',
      'number',
    ), NULL);
    for ($i = 1; $i <= $phenotype_number; $i++) {
      if (empty($phenotypes[$i])) {
        continue;
      }
      // [VS]
      // Set default value for 'Synonym Id' and 'Unit' for each phenotype
      // using previously submitted value.
      // Get from $phenotypes array and set in $form_state

      // Synonym Id.
      $synonym_id = (
        $phenotypes[$i]['synonym_id']
        ?? array_key_first($form[$id]['phenotype']['phenotypes-meta'][$i]['synonym_id']['#options'])
        ?? NULL
      );
      $form[$id]['phenotype']['phenotypes-meta'][$i]['synonym_id']['#default_value'] = $synonym_id;

      // Unit.
      $form[$id]['phenotype']['phenotypes-meta'][$i]['unit']['#options']
        = tpps_unit_get_list($synonym_id ?? 'all');
      $form[$id]['phenotype']['phenotypes-meta'][$i]['unit']['#default_value'] = (
        $phenotypes[$i]['unit']
        ?? array_key_first($form[$id]['phenotype']['phenotypes-meta'][$i]['unit']['#options'])
        ?? NULL
      );

      // Restore previous value of the Phenotype Structure.
      $form[$id]['phenotype']['phenotypes-meta'][$i]['structure']['#default_value']
        = ($phenotypes[$i]['structure']
          ?? array_key_first($form[$id]['phenotype']['phenotypes-meta'][$i]['structure']['#options'])
          ?? tpps_load_cvterm('whole plant')->cvterm_id
          ?? NULL
        );
      // [/VS]

      switch ($phenotypes[$i]['attribute']) {
        case tpps_load_cvterm('alive')->cvterm_id:
        case tpps_load_cvterm('bent')->cvterm_id:
        case tpps_load_cvterm('lesioned')->cvterm_id:
        case tpps_load_cvterm('rough')->cvterm_id:
          $terms = array(
            'boolean',
          );
          break;

        case tpps_load_cvterm('age')->cvterm_id:
        case tpps_load_cvterm('time')->cvterm_id:
          $terms = array(
            'day',
            'year',
          );
          break;

        case tpps_load_cvterm('area')->cvterm_id:
          $terms = array(
            'square_micrometer',
            'square_millimeter',
          );
          break;

        case tpps_load_cvterm('circumference')->cvterm_id:
        case tpps_load_cvterm('diameter')->cvterm_id:
        case tpps_load_cvterm('distance')->cvterm_id:
        case tpps_load_cvterm('height')->cvterm_id:
        case tpps_load_cvterm('length')->cvterm_id:
        case tpps_load_cvterm('thickness')->cvterm_id:
        case tpps_load_cvterm('width')->cvterm_id:
          $terms = array(
            'centimeter',
            'meter',
            'millimeter',
            'micrometer',
          );
          break;

        case tpps_load_cvterm('volume')->cvterm_id:
          $terms = array(
            'cubic_centimeter',
            'cubic_meter',
            'liter',
            'milliliter',
          );
          break;

        case tpps_load_cvterm('weight')->cvterm_id:
          $terms = array(
            'gram',
            'kilogram',
            'milligram',
          );
          break;

        case tpps_load_cvterm('temperature')->cvterm_id:
          $terms = array(
            'degrees_celsius',
            'degrees_fahrenheit',
          );
          break;

        case tpps_load_cvterm('pressure')->cvterm_id:
          $terms = array(
            'grams_per_square_meter',
            'pascal',
          );
          break;

        default:
          $terms = array();
          break;
      }

      // Note: When 'is phenotype environmental' ('env-check' field) checked
      // AJAX-request updates form to have 'Structure' limited number of items.
      // @TODO Use JS to reduce list of structure field options.
      if ($phenotypes[$i]['env-check']) {
        $terms = [
          'whole plant' => 'Whole Plant',
        ];
        $new_options = [];
        foreach ($terms as $term => $label) {
          $new_options[tpps_load_cvterm($term)->cvterm_id] = $label;
        }
        $form[$id]['phenotype']['phenotypes-meta'][$i]['structure']['#options'] = $new_options;
      }
    }

    // This was the beginning of code for phenotype reuse check but then Rish realized it would
    // need the new phenotype code from Vlad to make sense (import wise): 6/8/2023
    // $form[$id]['phenotype']['reuse_check'] = array(
    //   '#type' => 'checkbox',
    //   '#title' => t('I would like to reuse phenotype metadata and data from previous study'),
    //   '#attributes' => array(
    //     'data-toggle' => array('tooltip'),
    //     'data-placement' => array('right'),
    //     'title' => array('Upload a file'),
    //   ),
    //   '#states' => array(
    //     'invisible' => array(
    //       ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE),
    //     ),
    //   ),
    //   '#description' => t(''),
    // );

    // $studies_options = [];
    // $studies_results = chado_query('SELECT * FROM public.tpps_submission ORDER BY SUBSTRING(accession, 5) ASC;', []);
    // foreach ($studies_results as $study_row) {
    //   $studies_options[$study_row->accession] = $study_row->accession;
    // }

    // $form[$id]['phenotype']['reuse_check_study'] = array(
    //   '#type' => 'select',
    //   '#title' => t('Study with phenotype data'),
    //   '#options' => $studies_options
    // );

    $form[$id]['phenotype']['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('I would like to upload a phenotype metadata file'),
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('Upload a file'),
      ),
      '#description' => t('We encourage that you only upload a phenotype '
      . 'metadata file if you have > 20 phenotypes. Using the fields above '
      . 'instead of uploading a metadata file allows you to select from '
      . 'standardized controlled vocabulary terms, which makes your data '
      . 'more findable, interoperable, and reusable.'),
    );
    // @TODO Minor. Rename to $phenotype. Name $phenotypeFields used to avoid conflicts.
    // See Phenotype data 'managed_file' field in tpps_page_4_create_form().
    $form[$id]['phenotype']['metadata'] = [
      '#type' => 'managed_file',
      '#title' => t('Phenotype Metadata File: <br/ >Please upload a file '
      . 'containing columns with the name, attribute, structure, '
      . 'description, and units of each of your phenotypes: *'),
      '#upload_location' => 'public://' . $phenotype_dir,
      '#upload_validators' => ['file_validate_extensions' => ['csv tsv']],
      '#states' => [
        'visible' => [
          ':input[name="' . $id . '[phenotype][check]"]' => ['checked' => TRUE],
        ],
      ],
      '#tree' => TRUE,
      // Columns.
      'empty' => [
        '#default_value' => $values["$id"]['phenotype']['metadata']['empty'] ?? 'NA',
      ],
      'columns' => [
        '#description' => t('Please define which columns hold the required '
          . 'data: Phenotype name'),
      ],
      'columns-options' => [
        '#type' => 'hidden',
        '#value' => PhenotypeMeta::getColumnOptions(),
      ],
      'no-header' => [],
    ];

    // Get names of manual phenotypes.
    $meta = tpps_get_ajax_value($form_state, [
      $id,
      'phenotype',
      'phenotypes-meta',
    ]);
    $number = tpps_get_ajax_value($form_state, [
      $id,
      'phenotype',
      'phenotypes-meta',
      'number',
    ]);
    $phenotype_names = [];
    for ($i = 1; $i <= $number; $i++) {
      if (!empty($meta[$i]['name'])) {
        $phenotype_names[] = is_array($meta[$i]['name'])
          ? $meta[$i]['name']['#value'] : $meta[$i]['name'];
      }
    }

    // Get names of phenotypes in metadata file.
    $columns = tpps_get_ajax_value($form_state, [
      $id,
      'phenotype',
      'metadata',
      'columns',
    ], [], 'metadata');
    $meta_fid = tpps_get_ajax_value($form_state, [$id, 'phenotype', 'metadata']);
    $name_col = NULL;
    foreach ($columns as $key => $info) {
      if (preg_match('/^[A-Z]+$/', $key)) {
        $val = !empty($info['#value']) ? $info['#value'] : $info;
        if (
          !empty($val)
          && $val == PhenotypeMeta::DATA_TYPE_IDENTIFIER
        ) {
          $name_col = $key;
          break;
        }
      }
    }

    // Merge names.
    if (!empty($name_col) && tpps_file_load($meta_fid)) {
      $names = tpps_parse_file_column($meta_fid, $name_col);
      $phenotype_names = array_merge($phenotype_names, $names);
    }

    // If name ends in 4 digits (year), then time-check default = TRUE.
    $time_default = NULL;
    foreach ($phenotype_names as $name) {
      if (preg_match('/[0-9]{4}$/', $name)) {
        $time_default = TRUE;
      }
    }

    $form[$id]['phenotype']['time'] = array(
      '#type' => 'fieldset',
      '#title' => t('TIME OPTIONS'),
    );

    if ($time_default) {
      $message = t('It looks like some of your phenotypes might be time-based. '
        . 'If this is the case, please indicate which ones are time-based '
        . 'with the section below.');
      // @TODO Use l() to build a link to avoid HTML in PHP-code.
      $form[$id]['phenotype']['time']['#prefix'] = '<div class="alert '
        . 'alert-block alert-dismissible alert-warning messages warning">'
        . "<a class=\"close\" data-dismiss=\"alert\" href=\"#\">×</a>
        <h4 class=\"element-invisible\">Warning message</h4>
        {$message}</div>";
    }

    $form[$id]['phenotype']['time']['time-check'] = [
      '#type' => 'checkbox',
      '#title' => t('Some of my phenotypes are time-based'),
      '#default_value' => $time_default,
      '#ajax' => [
        'callback' => 'tpps_update_phenotype',
        'wrapper' => "phenotype-main-$id",
        'effect' => 'slide',
      ],
    ];

    $time_check = tpps_get_ajax_value($form_state,
      [$id, 'phenotype', 'time', 'time-check'],
      $time_default
    );
    if ($time_check) {
      $time_options = array();
      foreach ($phenotype_names as $name) {
        $time_options[strtolower($name)] = $name;
      }
      $form[$id]['phenotype']['time']['time_phenotypes'] = [
        '#type' => 'checkboxes',
        '#title' => t('Time-based Phenotypes: *'),
        // @TODO Convert to 'select' but with '#multiple' => TRUE.
        // @TODO Dropdown menu is always empty but $time_options is not empty...
        // See TGDR1224 which has timebased phenotypes.
        '#options' => $time_options,
        '#description' => t('Please select the phenotypes which are time-based'),
      ];

      $form[$id]['phenotype']['time']['time_values'] = [
        '#type' => 'fieldset',
        '#title' => t('PHENOTYPE TIME VALUES:'),
      ];

      foreach ($time_options as $key => $name) {
        $form[$id]['phenotype']['time']['time_values'][$key] = array(
          '#type' => 'textfield',
          '#title' => t('(Optional) @name time:', ['@name' => $name]),
          '#states' => [
            'visible' => [
              ':input[name="' . $id . '[phenotype][time][time_phenotypes]['
              . $key . ']"]' => ['checked' => TRUE],
            ],
          ],
        );
      }
    }
  }

  return $form[$id]['phenotype'];
}

/**
 * Returns the phenotype number when the "Clear Phenotypes" button is pressed.
 *
 * @param string $button_name
 *   The button being pressed.
 * @param int $value
 *   The value before the button was pressed.
 *
 * @return int
 *   The resulting value from pressing the button.
 */
function tpps_phenotype_number_clear($button_name, $value) {
  return 0;
}
