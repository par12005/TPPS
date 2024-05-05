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
    $attr_options = array(0 => '- Select -');
    $terms = array(
      'absorbance' => t('Absorbance'),
      'age' => t('Age'),
      'alive' => t('Alive'),
      'amount' => t('Amount'),
      'angle' => t('Angle'),
      'area' => t('Area'),
      'bent' => t('Bent'),
      'carbon-13 atom' => t('Carbon-13 Atom'),
      'chlorophyll' => t('Chlorophyll'),
      'circumference' => t('Circumerence'),
      'color' => t('Color'),
      'composition' => t('Composition'),
      'concentration_of' => t('Concentration of'),
      'damage' => t('Damage'),
      'delta' => t('Delta'),
      'description' => t('Description'),
      'diameter' => t('Diameter'),
      'distance' => t('Distance'),
      'gravity' => t('Gravity'),
      'growth_quality_of_occurrent' => t('Growth Quality of Occurrent'),
      'growth_rate' => t('Growth Rate'),
      'has_number_of' => t('Has number of'),
      'height' => t('Height'),
      'humidity_level' => t('Humidity Level'),
      'intensity' => t('Intensity'),
      'length' => t('Length'),
      'lesioned' => t('Lesioned'),
      'maturity' => t('Maturity'),
      'photosynthesis' => t('Photosynthesis'),
      'position' => t('Position'),
      'precipitation' => t('Precipitation'),
      'pressure' => t('Pressure'),
      'proportionality_to' => t('Proportionality to'),
      'rate' => t('Rate'),
      'rough' => t('Rough'),
      'sex' => t('Sex'),
      'shape' => t('Shape'),
      'size' => t('Size'),
      'temperature' => t('Temperature'),
      'texture' => t('Texture'),
      'thickness' => t('Thickness'),
      'time' => t('Time'),
      'transpiration' => t('Transpiration'),
      'volume' => t('Volume'),
      'water use efficiency' => t('Water use efficiency'),
      'weight' => t('Weight'),
      'width' => t('Width'),
    );
    foreach ($terms as $term => $label) {
      $attr_id = tpps_load_cvterm($term)->cvterm_id;
      $attr_options[$attr_id] = $label;
    }
    $attr_options['other'] = 'My attribute term is not in this list';

    $struct_options = [];
    $terms = [
      'whole plant' => t('Whole Plant'),
      'bark' => t('Bark'),
      'branch' => t('Branch'),
      'bud' => t('Bud'),
      'catkin_inflorescence' => t('Catkin Inflorescence'),
      'endocarp' => t('Endocarp'),
      'floral_organ' => t('Floral Organ'),
      'flower' => t('Flower'),
      'flower_bud' => t('Flower Bud'),
      'flower_fascicle' => t('Flower Fascicle'),
      'fruit' => t('Fruit'),
      'leaf' => t('Leaf'),
      'leaf_rachis' => t('Leaf Rachis'),
      'leaflet' => t('Leaflet'),
      'nut_fruit' => t('Nut Fruit (Acorn)'),
      'petal' => t('Petal'),
      'petiole' => t('Petiole'),
      'phloem' => t('Phloem'),
      'plant_callus' => t('Plant Callus (Callus)'),
      'primary_thickening_meristem' => t('Primary Thickening Meristem'),
      'root' => t('Root'),
      'secondary_xylem' => t('Secondary Xylem (Wood)'),
      'seed' => t('Seed'),
      'shoot_system' => t('Shoot System (Crown)'),
      'stem' => t('Stem (Trunk, Primary Stem)'),
      'stomatal_complex' => t('Stomatal Complex (Stomata)'),
      'strobilus' => t('Strobilus'),
      'terminal_bud' => t('Terminal Bud'),
      'vascular_leaf' => t('Vascular Leaf (Needle)'),
    ];
    foreach ($terms as $term => $label) {
      $struct_id = tpps_load_cvterm($term)->cvterm_id;
      $struct_options[$struct_id] = $label;
    }
    $struct_options['other'] = 'My structure term is not in this list';

    // List of Phenotype synonyms. This list will be the same for all
    // phenotypes. but unit list is unique per phenotype and
    // will be obtained later because depends on selected synonym.
    $synonym_list = tpps_synonym_get_list();
    $phenotype_cid = 'tpps_phenotype_field';
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    $cache = cache_get($phenotype_cid, $cache_bin);

    if (!empty($cache)) {
      $field = $cache->data;
    }
    else {
      $field = array(
        '#type' => 'fieldset',
        '#tree' => TRUE,
        '#prefix' => "<div id=\"org_{$id}_phenotype_!num_meta\">",
        '#suffix' => "</div>",
        // [VS] Synonym form.
        'synonym_name' => tpps_build_field_name($id) + [
          '#prefix' => "<label><b>Phenotype !num:</b></label>",
          '#states' => [
            'visible' => [tpps_synonym_selector($id) => ['!value' => 0]],
          ],
        ],
        'synonym_description' => tpps_build_field_description() + [
          '#states' => [
            'visible' => [tpps_synonym_selector($id) => ['!value' => 0]],
          ],
        ],

        'synonym_id' => [
          '#type' => 'select',
          // The label should just be "phenotype", no synonym anywhere. We use
          // synonym to describe our phenotype setup behind the scenes but it's
          // not relevant and would be confusing for the scientists using TPPS.
          '#title' => 'Phenotype: *',
          '#options' => $synonym_list,
          '#default_value' => array_key_first($synonym_list) ?? NULL,
          // Unit dropdown must be updated in each synonym field change.
          '#ajax' => [
            'callback' => 'tpps_unit_update_list',
            'wrapper' => 'unit-list-!num-wrapper',
            'method' => 'replace',
            'event' => 'change',
          ],
        ],
        // [/VS]

        // Main form.
        'name' => tpps_build_field_name($id) + [
          '#states' => [
            'visible' => [tpps_synonym_selector($id) => ['value' => 0]]
          ],
        ],
        'env-check' => array(
          '#type' => 'checkbox',
          '#title' => 'Phenotype !num is an environmental phenotype',
          '#ajax' => array(
            'callback' => 'tpps_update_phenotype_meta',
            'wrapper' => "org_{$id}_phenotype_!num_meta",
          ),
          '#states' => [
            'visible' => [tpps_synonym_selector($id) => ['value' => 0]]
          ],
        ),
        'attribute' => array(
          '#type' => 'select',
          '#title' => 'Phenotype !num Attribute: *',
          '#options' => $attr_options,
          '#ajax' => array(
            'callback' => 'tpps_update_phenotype_meta',
            'wrapper' => "org_{$id}_phenotype_!num_meta",
          ),
          '#states' => [
            'visible' => [tpps_synonym_selector($id) => ['value' => 0]]
          ],
        ),
        'attr-other' => array(
          '#type' => 'textfield',
          '#title' => 'Phenotype !num Custom Attribute: *',
          '#autocomplete_path' => 'tpps/autocomplete/attribute',
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('right'),
            'title' => array('If your attribute is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
          ),
          '#description' => t('Some examples of attributes include: "amount", "width", "mass density", "area", "height", "age", "broken", "time", "color", "composition", etc.'),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][attribute]"]'
              => ['value' => 'other'],
              tpps_synonym_selector($id) => ['value' => 0],
            ),
          ),
        ),
        'description' => tpps_build_field_description() + [
          '#states' => [
            'visible' => [tpps_synonym_selector($id) => ['value' => 0]]
          ],
        ],
        'unit' => [
          '#type' => 'select',
          '#title' => 'Phenotype !num Unit: *',
          // List of units depends on selected synonym. Will be populated later.
          // The same for default value.
          '#options' => tpps_unit_get_list(array_key_first($synonym_list) ?? 'all'),
          '#prefix' => '<div id="unit-list-!num-wrapper">',
          '#suffix' => '</div>',
          '#validated' => TRUE,
        ],
        // [VS] #8669rmrw5.
        'unit-other' => [
          '#type' => 'textfield',
          '#title' => 'Phenotype !num Custom Unit: *',
          '#autocomplete_path' => 'tpps/autocomplete/unit',
          '#attributes' => [
            'data-toggle' => ['tooltip'],
            'data-placement' => ['right'],
            'title' => ['If your unit is not in the autocomplete list, '
              . 'don\'t worry about it! We will create new phenotype '
              . 'metadata in the database for you.'
            ],
          ],
          '#description' => t('Some examples of units include: "m", "meters", '
            . '"in", "inches", "Degrees Celsius", "°C", etc.'),
          '#states' => [
            'visible' => [
              ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][unit]"]'
              => ['value' => 'other'],
            ],
          ],
        ],
        'structure' => [
          '#type' => 'select',
          '#title' => 'Phenotype !num Structure: *',
          '#options' => $struct_options,
          '#states' => [
            'visible' => [tpps_synonym_selector($id) => ['value' => 0]],
          ],
          '#validated' => TRUE,
        ],
        // [/VS]
        'struct-other' => array(
          '#type' => 'textfield',
          '#title' => 'Phenotype !num Custom Structure: *',
          '#autocomplete_path' => 'tpps/autocomplete/structure',
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('right'),
            'title' => array('If your structure is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
          ),
          '#description' => t('Some examples of structure descriptors include: "stem", "bud", "leaf", "xylem", "whole plant", "meristematic apical cell", etc.'),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][structure]"]' => array('value' => 'other'),
            ),
          ),
        ),
      );
      cache_set($phenotype_cid, $field, $cache_bin);
    }

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

      // @TODO Reduces list of structures. Fix it!

      if ($phenotypes[$i]['env-check']) {
        $terms = array(
          'whole plant' => 'Whole Plant',
          'soil_type' => 'Soil',
          'atmosphere' => 'Atmosphere',
        );

        $new_options = array();
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
    ];

    $form[$id]['phenotype']['metadata']['empty'] = [
      '#default_value' => $values["$id"]['phenotype']['metadata']['empty'] ?? 'NA',
    ];

    $form[$id]['phenotype']['metadata']['columns'] = [
      '#description' => t('Please define which columns hold the required data: Phenotype name'),
    ];

    $column_options = array(
      'N/A',
      'Phenotype Name/Identifier',
      'Attribute',
      'Description',
      'Unit',
      'Structure',
      'Minimum Value',
      'Maximum Value',
    );

    $form[$id]['phenotype']['metadata']['columns-options'] = array(
      '#type' => 'hidden',
      '#value' => $column_options,
    );

    $form[$id]['phenotype']['metadata']['no-header'] = array();

    // Get names of manual phenotypes.
    $meta = tpps_get_ajax_value($form_state, array(
      $id,
      'phenotype',
      'phenotypes-meta',
    ));
    $number = tpps_get_ajax_value($form_state, array(
      $id,
      'phenotype',
      'phenotypes-meta',
      'number',
    ));
    $phenotype_names = [];
    for ($i = 1; $i <= $number; $i++) {
      if (!empty($meta[$i]['name'])) {
        $phenotype_names[] = is_array($meta[$i]['name'])
          ? $meta[$i]['name']['#value'] : $meta[$i]['name'];
      }
    }

    // Get names of phenotypes in metadata file.
    $columns = tpps_get_ajax_value($form_state, array(
      $id,
      'phenotype',
      'metadata',
      'columns',
    ), array(), 'metadata');
    $meta_fid = tpps_get_ajax_value($form_state, [$id, 'phenotype', 'metadata']);
    $name_col = NULL;
    foreach ($columns as $key => $info) {
      if (preg_match('/^[A-Z]+$/', $key)) {
        $val = !empty($info['#value']) ? $info['#value'] : $info;
        if (!empty($val) and $column_options[$val] == 'Phenotype Name/Identifier') {
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
      $message = t('It looks like some of your phenotypes might be time-based. If this is the case, please indicate which ones are time-based with the section below.');
      $form[$id]['phenotype']['time']['#prefix'] = "<div class=\"alert alert-block alert-dismissible alert-warning messages warning\">
        <a class=\"close\" data-dismiss=\"alert\" href=\"#\">×</a>
        <h4 class=\"element-invisible\">Warning message</h4>
        {$message}</div>";
    }

    $form[$id]['phenotype']['time']['time-check'] = array(
      '#type' => 'checkbox',
      '#title' => t('Some of my phenotypes are time-based'),
      '#default_value' => $time_default,
      '#ajax' => array(
        'callback' => 'tpps_update_phenotype',
        'wrapper' => "phenotype-main-$id",
        'effect' => 'slide',
      ),
    );

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

      $form[$id]['phenotype']['time']['time_values'] = array(
        '#type' => 'fieldset',
        '#title' => t('PHENOTYPE TIME VALUES:'),
      );

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

/**
 * Builds Phenotype Name form field.
 *
 * @param string $id
 *   Organism Id.
 *
 * @return array
 *   Returns Form API field.
 */
function tpps_build_field_name($id) {
  return [
    '#type' => 'textfield',
    '#title' => 'Phenotype !num Name: *',
    '#attributes' => [
      'data-toggle' => ['tooltip'],
      'data-placement' => ['right'],
      'title' => [
        'If your phenotype name does not exist in our database, '
        . 'don\'t worry about it! We will create new phenotype metadata '
        . 'in the database for you.',
      ],
      // Alternative title for one of fields:
      // 'title' => ['If your phenotype name is not in the '
      // . 'autocomplete list, don\'t worry about it! We will create new '
      // . 'phenotype metadata in the database for you.'],
    ],
    '#description' => t('<strong>WARNING: <br />Phenotype "name" should match the '
      . 'data in the "Phenotype Name/Identifier" column that you select '
      . 'in your !link below.</strong>',
      [
        '!link' => l(t('Phenotype file'), $_GET['q'],
          ['fragment' => "edit-$id-phenotype-file-ajax-wrapper"]
        ),
      ]
    ),
  ];
}

/**
 * Builds Phenotype Description form field.
 *
 * @return array
 *   Returns Form API field.
 */
function tpps_build_field_description() {
  return [
    '#type' => 'textfield',
    '#title' => 'Phenotype !num Description: *',
    '#description' => t('Please provide a short description of Phenotype !num'),
  ];
}
