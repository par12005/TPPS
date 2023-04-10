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
  $phenotype_upload_location = 'public://' . variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');

  $form[$id]['phenotype'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Phenotype Information:</div>'),
    '#tree' => TRUE,
    '#prefix' => "<div id=\"phenotype-main-$id\">",
    '#suffix' => '</div>',
    '#description' => t('Upload a file and/or fill in form fields below to provide us with metadata about your phenotypes.'),
    '#collapsible' => TRUE,
  );

  $form[$id]['phenotype']['normal-check'] = array(
    '#type' => 'checkbox',
    '#title' => t('My phenotypes include traits and/or environmental information other than mass spectrometry or isotope analysis'),
    '#ajax' => array(
      'callback' => 'tpps_update_phenotype',
      'wrapper' => "phenotype-main-$id",
    ),
    '#default_value' => tpps_get_ajax_value($form_state, array(
      $id,
      'phenotype',
      'normal-check',
    ), TRUE),
  );

  $form[$id]['phenotype']['iso-check'] = array(
    '#type' => 'checkbox',
    '#title' => t('My phenotypes include results from a mass spectrometry or isotope analysis'),
    '#ajax' => array(
      'callback' => 'tpps_update_phenotype',
      'wrapper' => "phenotype-main-$id",
    ),
  );

  $normal_check = tpps_get_ajax_value($form_state, array(
    $id,
    'phenotype',
    'normal-check',
  ), NULL);

  $iso_check = tpps_get_ajax_value($form_state, array(
    $id,
    'phenotype',
    'iso-check',
  ), NULL);

  if (!empty($iso_check)) {
    $form[$id]['phenotype']['iso'] = array(
      '#type' => 'managed_file',
      '#title' => t('Phenotype Isotope/Mass Spectrometry file: *'),
      '#upload_location' => $phenotype_upload_location,
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#description' => t('Please upload a file containing all of your isotope/mass spectrometry data. The format of this file is very important! The first column of your file should contain plant identifiers which match the plant identifiers you provided in your plant accession file, and all of the remaining columns should contain isotope or mass spectrometry data.'),
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
      // drupal_set_message($term . "," . $label . "," . $attr_id);
    }
    $attr_options['other'] = 'My attribute term is not in this list';

    // [VS] #8669rmrw5
    // Synonyms.
    $synonym_list = tpps_synonym_get_list();
    $default_synonym = array_key_first($synonym_list);
    // Unit.
    $unit_list = tpps_synonym_get_unit_list($default_synonym, TRUE);
    $default_unit = array_key_first($unit_list);
    // [/VS] #8669rmrw5

    $struct_options = array();
    $terms = array(
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
    );
    foreach ($terms as $term => $label) {
      $struct_id = tpps_load_cvterm($term)->cvterm_id;
      $struct_options[$struct_id] = $label;
      // drupal_set_message($term . "," . $label . "," . $struct_id);
    }
    $struct_options['other'] = 'My structure term is not in this list';

    $field = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#prefix' => "<div id=\"org_{$id}_phenotype_!num_meta\">",
      '#suffix' => "</div>",
      // [VS] Synonym form.
      'synonym_name' => tpps_build_field_name($id) + [
        '#prefix' => "<label><b>Phenotype !num:</b></label>",
        '#states' => ['visible' => [
          tpps_synonym_selector($id) => ['!value' => 0],
      ]]],
      'synonym_description' => tpps_build_field_description() + [
        '#states' => ['visible' => [
          tpps_synonym_selector($id) => ['!value' => 0],
      ]]],
      'synonym_id' => [
        '#type' => 'select',
        '#title' => 'Synonym: *',
        '#options' => $synonym_list,
        '#default_value' => $default_synonym,
        // Unit dropdown must be updated in each synonym field change.
        '#ajax' => [
          'callback' => 'tpps_synonym_update_unit_list',
          'wrapper' => 'unit-list-!num-wrapper',
          'method' => 'replace',
          'event' => 'change',
        ],
      ],
      // [/VS]

      // Main form.
      'name' => tpps_build_field_name($id) + ['#states' => ['visible' => [
          tpps_synonym_selector($id) => ['value' => 0],
      ]]],
      'env-check' => array(
        '#type' => 'checkbox',
        '#title' => 'Phenotype !num is an environmental phenotype',
        '#ajax' => array(
          'callback' => 'tpps_update_phenotype_meta',
          'wrapper' => "org_{$id}_phenotype_!num_meta",
        ),
        '#states' => ['visible' => [
          tpps_synonym_selector($id) => ['value' => 0],
        ]],
      ),
      'attribute' => array(
        '#type' => 'select',
        '#title' => 'Phenotype !num Attribute: *',
        '#options' => $attr_options,
        '#ajax' => array(
          'callback' => 'tpps_update_phenotype_meta',
          'wrapper' => "org_{$id}_phenotype_!num_meta",
        ),
        '#states' => ['visible' => [
          tpps_synonym_selector($id) => ['value' => 0],
        ]],
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
              => array('value' => 'other'),
            tpps_synonym_selector($id) => ['value' => 0],
          ),
        ),
      ),
      'description' => tpps_build_field_description()
        + array('#states' => array('visible' => array(
            tpps_synonym_selector($id) => ['value' => 0],
        ))),
      // [VS] #8669rmrw5.
      'unit' => [
        '#type' => 'select',
        '#title' => 'Phenotype !num Unit: *',
        '#options' => $unit_list,
        '#default_value' => $default_unit,
        '#prefix' => '<div id="unit-list-!num-wrapper">',
        '#suffix' => '</div>',
        '#validated' => TRUE,
      ],
      'custom-unit' => [
        '#type' => 'textfield',
        '#title' => 'Phenotype !num Custom Unit: *',
        '#autocomplete_path' => 'tpps/autocomplete/unit',
        '#attributes' => array(
          'data-toggle' => array('tooltip'),
          'data-placement' => array('right'),
          'title' => array('If your unit is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
        ),
        '#description' => t('Some examples of units include: "m", "meters", "in", "inches", "Degrees Celsius", "°C", etc.'),

        // @TODO Major. Not work because unit field added by
        // ajax has no name or id in browser.

        '#states' => ['visible' => [
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][unit]"]'
            => ['value' => 0],
        ]],
      ],
      // [/VS]
      'structure' => array(
        '#type' => 'select',
        '#title' => 'Phenotype !num Structure: *',
        '#options' => $struct_options,
        '#default_value' => tpps_load_cvterm('whole plant')->cvterm_id,
        '#states' => array(
          'visible' => [
            tpps_synonym_selector($id) => ['value' => 0],
          ],
        ),
      ),
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

    tpps_dynamic_list($form, $form_state, 'phenotypes-meta', $field, array(
      'label' => 'Phenotype',
      'title' => "",
      'callback' => 'tpps_update_phenotype',
      'parents' => array($id, 'phenotype'),
      'wrapper' => "phenotype-main-$id",
      'name_suffix' => $id,
      // [VS] #8669py3z7
      'alternative_buttons' => [
        //"Add 5 Phenotypes" => 5,
        "Add 20 Phenotypes" => 20,
        "Clear All Phenotypes" => 'tpps_phenotype_number_clear',
      ],
      'button_weights' => [
        "Add Phenotype" => -5,
        //"Add 5 Phenotypes" => -4,
        "Add 20 Phenotypes" => -3,
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
        ['custom-unit', '#title'],
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
          'custom-unit',
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
    ));

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

    $form[$id]['phenotype']['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('I would like to upload a phenotype metadata file'),
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('Upload a file'),
      ),
      '#description' => t('We encourage that you only upload a phenotype metadata file if you have > 20 phenotypes. Using the fields above instead of uploading a metadata file allows you to select from standardized controlled vocabulary terms, which makes your data more findable, interoperable, and reusable.'),
    );


    $form[$id]['phenotype']['metadata'] = array(
      '#type' => 'managed_file',
      '#title' => t('Phenotype Metadata File: Please upload a file containing columns with the name, attribute, structure, description, and units of each of your phenotypes: *'),
      '#upload_location' => "$phenotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE),
        ),
      ),
      '#tree' => TRUE,
    );

    $form[$id]['phenotype']['metadata']['empty'] = array(
      '#default_value' => isset($values["$id"]['phenotype']['metadata']['empty']) ? $values["$id"]['phenotype']['metadata']['empty'] : 'NA',
    );

    $form[$id]['phenotype']['metadata']['columns'] = array(
      '#description' => t('Please define which columns hold the required data: Phenotype name'),
    );

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
    $phenotype_names = array();
    for ($i = 1; $i <= $number; $i++) {
      if (!empty($meta[$i]['name'])) {
        $phenotype_names[] = is_array($meta[$i]['name']) ? $meta[$i]['name']['#value'] : $meta[$i]['name'];
      }
    }

    // Get names of phenotypes in metadata file.
    $columns = tpps_get_ajax_value($form_state, array(
      $id,
      'phenotype',
      'metadata',
      'columns',
    ), array(), 'metadata');
    $meta_fid = tpps_get_ajax_value($form_state, array(
      $id,
      'phenotype',
      'metadata',
    ));
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
    if (!empty($name_col) and !is_array($meta_fid) and !empty(file_load($meta_fid))) {
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
      '#title' => t('Time options'),
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
      ),
    );

    $time_check = tpps_get_ajax_value($form_state, array(
      $id,
      'phenotype',
      'time',
      'time-check',
    ), $time_default);
    if ($time_check) {
      $time_options = array();
      foreach ($phenotype_names as $name) {
        $time_options[strtolower($name)] = $name;
      }
      $form[$id]['phenotype']['time']['time_phenotypes'] = array(
        '#type' => 'select',
        '#title' => t('Time-based Phenotypes: *'),
        '#options' => $time_options,
        '#description' => t('Please select the phenotypes which are time-based'),
      );

      $form[$id]['phenotype']['time']['time_values'] = array(
        '#type' => 'fieldset',
        '#title' => t('Phenotype Time values:'),
      );

      foreach ($time_options as $key => $name) {
        $form[$id]['phenotype']['time']['time_values'][$key] = array(
          '#type' => 'textfield',
          '#title' => t('(Optional) @name time:', array('@name' => $name)),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][time][time_phenotypes][' . $key . ']"]' => array('checked' => TRUE),
            ),
          ),
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
 * Creates fields describing the genotype data for the submission.
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
function tpps_genotype(array &$form, array &$form_state, array $values, $id) {

  $genotype_upload_location = 'public://' . variable_get('tpps_genotype_files_dir', 'tpps_genotype');

  $fields = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Genotype Information:</div>'),
    '#collapsible' => TRUE,
  );

  tpps_page_4_marker_info($fields, $id);

  tpps_page_4_ref($fields, $form_state, $id);

  $marker_parents = array(
    $id,
    'genotype',
    'marker-type',
  );
  $parents = array_merge($marker_parents, array('SNPs'));
  $snps_check = tpps_get_ajax_value($form_state, $parents);

  $parents = array_merge($marker_parents, array('SSRs/cpSSRs'));
  $ssrs_check = tpps_get_ajax_value($form_state, $parents);

  $parents = array_merge($marker_parents, array('Indels'));
  $indel_check = tpps_get_ajax_value($form_state, $parents);

  $parents = array_merge($marker_parents, array('Other'));
  $other_marker_check = tpps_get_ajax_value($form_state, $parents);

  $fields['files'] = array(
    '#type' => 'fieldset',
    '#prefix' => "<div id='$id-genotype-files'>",
    '#suffix' => '</div>',
  );

  if (!empty($ssrs_check)) {
    $fields['files']['ploidy'] = array(
      '#type' => 'select',
      '#title' => t('Ploidy'),
      '#options' => array(
        0 => t('- Select -'),
        'Haploid' => t('Haploid'),
        'Diploid' => t('Diploid'),
        'Polyploid' => t('Polyploid'),
      ),
      '#ajax' => array(
        'callback' => 'tpps_genotype_files_callback',
        'wrapper' => "$id-genotype-files",
      ),
    );
  }

  $file_type_parents = array(
    $id,
    'genotype',
    'files',
    'file-type',
  );
  $options = array();
  if (!empty($snps_check)) {
    $options['SNPs Genotype Assay'] = 'SNPs Genotype Assay';
    $parents = array_merge($file_type_parents, array('SNPs Genotype Assay'));
    $snps_assay_check = tpps_get_ajax_value($form_state, $parents);

    if (!empty($snps_assay_check)) {
      $options['Assay Design'] = 'Assay Design';
    }
    $parents = array_merge($file_type_parents, array('Assay Design'));
    $assay_design_check = tpps_get_ajax_value($form_state, $parents);

    if (!empty($snps_assay_check) and !empty($form[$id]['phenotype'])) {
      $options['SNPs Associations'] = 'SNPs Associations';
    }
    $parents = array_merge($file_type_parents, array('SNPs Associations'));
    $association_check = tpps_get_ajax_value($form_state, $parents);
  }
  if (!empty($ssrs_check)) {
    $options['SSRs/cpSSRs Genotype Spreadsheet'] = 'SSRs/cpSSRs Genotype Spreadsheet';
    $parents = array_merge($file_type_parents, array('SSRs/cpSSRs Genotype Spreadsheet'));
    $ssrs_file_check = tpps_get_ajax_value($form_state, $parents);
  }
  if (!empty($indel_check)) {
    $options['Indel Genotype Spreadsheet'] = 'Indel Genotype Spreadsheet';
    $parents = array_merge($file_type_parents, array('Indel Genotype Spreadsheet'));
    $indel_file_check = tpps_get_ajax_value($form_state, $parents);
  }
  if (!empty($other_marker_check)) {
    $options['Other Marker Genotype Spreadsheet'] = 'Other Marker Genotype Spreadsheet';
    $parents = array_merge($file_type_parents, array('Other Marker Genotype Spreadsheet'));
    $other_file_check = tpps_get_ajax_value($form_state, $parents);
  }
  $options['VCF'] = 'VCF';

  // [VS] #8669rmvf2
  $fields['files']['file-type'] = [
    '#type' => 'select',
    '#title' => t('Genotype File Types (select all that apply): *'),
    '#options' => $options,
    '#ajax' => [
      'callback' => 'tpps_genotype_files_callback',
      'wrapper' => "$id-genotype-files",
    ],
  ];
  // [/VS]

  $parents = array_merge($file_type_parents, array('VCF'));
  $vcf_file_check = tpps_get_ajax_value($form_state, $parents);


  if (!empty($snps_assay_check)) {
    $fields['files']['file-selector'] = array(
      '#type' => 'checkbox',
      '#title' => t('Reference Existing SNP File'),
      '#ajax' => array(
        'callback' => 'tpps_genotype_files_type_change_callback',
        'wrapper' => "$id-genotype-files",
      ),
    );
    $file_type_parents = array(
      $id,
      'genotype',
      'files',
    );

    $parents = array_merge($file_type_parents, array('file-selector'));
    $file_selector_check = tpps_get_ajax_value($form_state, $parents);

    if (empty($file_selector_check)) {
      $fields['files']['snps-assay'] = array(
        '#type' => 'managed_file',
        '#title' => t('SNPs Genotype Assay File: please provide a spreadsheet with columns for the Plant ID of genotypes used in this study: *'),
        '#upload_location' => "$genotype_upload_location",
        '#upload_validators' => array(
          'file_validate_extensions' => array('csv tsv xlsx'),
        ),
        '#description' => t("Please upload a spreadsheet file containing SNP Genotype Assay data. The format of this file is very important! The first column of your file should contain plant identifiers which match the plant identifiers you provided in your plant accession file, and all of the remaining columns should contain SNP data."),
        '#tree' => TRUE,
      );

      if (isset($fields['files']['snps-assay']['#value']['fid'])) {
        $fields['files']['snps-assay']['#default_value'] = $fields['files']['snps-assay']['#value']['fid'];
      }
      if (!empty($fields['files']['snps-assay']['#default_value']) and ($file = file_load($fields['files']['snps-assay']['#default_value']))) {
        // Stop using the file so it can be deleted if the user clicks 'remove'.
        file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      }
    }
    else {
      // // Add autocomplete field.
      $fields['files']['snps-assay'] = array(
        '#type' => 'textfield',
        '#title' => t('SNPs Genotype Assay File: please select an already existing spreadsheet with columns for the Plant ID of genotypes used in this study: *'),
        '#upload_location' => "$genotype_upload_location",
        '#autocomplete_path' => 'snp-assay-file/upload',
        '#description' => t("Please select an already existing spreadsheet file containing SNP Genotype Assay data. The format of this file is very important! The first column of your file should contain plant identifiers which match the plant identifiers you provided in your plant accession file, and all of the remaining columns should contain SNP data."),
      );
    }
  }
  else {
    $fields['files']['snps-assay'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($assay_design_check)) {
    $design_options = array(0 => '- Select -');
    $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
    for ($i = 1; $i <= $firstpage['organism']['number']; $i++) {
      $parts = explode(" ", $firstpage['organism'][$i]['name']);
      $genus = $parts[0];
      $query = db_select('chado.organism', 'o');
      $query->join('chado.project_organism', 'po', 'o.organism_id = po.organism_id');
      $query->join('public.tpps_project_file_managed', 'pf', 'pf.project_id = po.project_id');
      $query->join('public.file_managed', 'f', 'f.fid = pf.fid');
      $query->join('chado.project', 'p', 'p.project_id = po.project_id');
      $query->fields('f');
      $query->fields('p');
      $query->condition('o.genus', $genus);
      $query->condition('f.filename', '%assay_design%', 'ILIKE');
      $results = $query->execute();
      while (($record = $results->fetchObject())) {
        $design_options[$record->fid] = "{$record->filename} (from \"{$record->name}\")";
      }
    }
    $design_options['new'] = 'I would like to upload a new assay design file';
    $fields['files']['assay-load'] = array(
      '#type' => 'select',
      '#title' => 'Genotype Assay Design: *',
      '#options' => $design_options,
      '#description' => t('Please select an assay design. Some design files from the same genus as this species are available, or you can choose to upload your own assay design file.'),
    );

    $fields['files']['assay-design'] = array(
      '#type' => 'managed_file',
      '#title' => 'Genotype Assay Design File: *',
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#tree' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][files][assay-load]"]' => array('value' => 'new'),
        ),
      ),
    );

    if (isset($fields['files']['assay-design']['#value'])) {
      $fields['files']['assay-design']['#default_value'] = $fields['files']['assay-design']['#value'];
    }
    if (!empty($fields['files']['assay-design']['#default_value']) and ($file = file_load($fields['files']['assay-design']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }

    $fields['files']['assay-citation'] = array(
      '#type' => 'textfield',
      '#title' => t('Genotype Assay Design Citation (Optional):'),
      '#description' => t('If your assay design file is from a different paper, please include the citation for that paper here.'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][files][assay-load]"]' => array('value' => 'new'),
        ),
      ),
    );
  }
  else {
    $fields['files']['assay-design'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($association_check)) {
    $fields['files']['snps-association'] = array(
      '#type' => 'managed_file',
      '#title' => t('SNPs Association File: *'),
      '#upload_location' => $genotype_upload_location,
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#description' => t('Please upload a spreadsheet file containing SNPs Association data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may set that drop-down menu to "N/A". Your file must contain columns with the SNP ID, Scaffold, Position (formatted like "start:stop"), Allele (formatted like "major:minor"), Associated Trait Name (must match a phenotype from the above section), and Confidence Value. Optionally, you can also specify a Gene ID (which should match the gene reference) and a SNP Annotation (non synonymous, coding, etc).'),
      '#tree' => TRUE,
      'empty' => array(
        '#default_value' => $values[$id]['genotype']['files']['snps-association']['empty'] ?? 'NA',
      ),
      'columns' => array(
        '#description' => t('Please define which columns hold the required data: SNP ID, Scaffold, Position, Allele, Associated Trait, Confidence Value.'),
      ),
      'columns-options' => array(
        '#type' => 'hidden',
        '#value' => array(
          'N/A',
          'SNP ID',
          'Scaffold',
          'Position',
          'Allele',
          'Associated Trait',
          'Confidence Value',
          'Gene ID',
          'Annotation',
        ),
        'no-header' => array(),
      ),
    );

    $fields['files']['snps-association-type'] = array(
      '#type' => 'select',
      '#title' => t('Confidence Value Type: *'),
      '#options' => array(
        0 => t('- Select -'),
        'P value' => t('P value'),
        'Genomic Inflation Factor (GIF)' => t('Genomic Inflation Factor (GIF)'),
        'P-adjusted (FDR) / Q value' => t('P-adjusted (FDR) / Q value'),
        'P-adjusted (FWE)' => t('P-adjusted (FWE)'),
        'P-adjusted (Bonferroni)' => t('P-adjusted (Bonferroni)'),
      ),
    );

    $fields['files']['snps-association-tool'] = array(
      '#type' => 'select',
      '#title' => t('Association Analysis Tool: *'),
      '#options' => array(
        0 => t('- Select -'),
        'GEMMA' => t('GEMMA'),
        'EMMAX' => t('EMMAX'),
        'Plink' => t('Plink'),
        'Tassel' => t('Tassel'),
        'Sambada' => t('Sambada'),
        'Bayenv' => t('Bayenv'),
        'BayeScan' => t('BayeScan'),
        'LFMM' => t('LFMM'),
      ),
    );

    $fields['files']['snps-pop-struct'] = array(
      '#type' => 'managed_file',
      '#title' => 'SNPs Population Structure File: ',
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#tree' => TRUE,
    );

    if (isset($fields['files']['snps-pop-struct']['#value'])) {
      $fields['files']['snps-pop-struct']['#default_value'] = $fields['files']['snps-pop-struct']['#value'];
    }
    if (!empty($fields['files']['snps-pop-struct']['#default_value']) and ($file = file_load($fields['files']['snps-pop-struct']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }

    $fields['files']['snps-kinship'] = array(
      '#type' => 'managed_file',
      '#title' => 'SNPs Kinship File: ',
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#tree' => TRUE,
    );

    if (isset($fields['files']['snps-kinship']['#value'])) {
      $fields['files']['snps-kinship']['#default_value'] = $fields['files']['snps-kinship']['#value'];
    }
    if (!empty($fields['files']['snps-kinship']['#default_value']) and ($file = file_load($fields['files']['snps-kinship']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }
  }
  else {
    $fields['files']['snps-association'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );

    $fields['files']['snps-pop-struct'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );

    $fields['files']['snps-kinship'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($ssrs_file_check)) {
    $fields['files']['ssrs'] = array(
      '#type' => 'managed_file',
      '#title' => t('SSRs/cpSSRs Spreadsheet: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#description' => t('Please upload a spreadsheet containing your SSRs/cpSSRs data. The format of this file is very important! TPPS will parse your file based on the ploidy you have selected above. For any ploidy, TPPS will assume that the first column of your file is the column that holds the Plant Identifier that matches your accession file.'),
      '#tree' => TRUE,
      'empty' => array(
        '#default_value' => isset($values["organism-$id"]['genotype']['files']['ssrs']) ? $values["organism-$id"]['genotype']['files']['ssrs'] : 'NA',
      ),
    );

    $ploidy = tpps_get_ajax_value($form_state, array(
      $id,
      'genotype',
      'files',
      'ploidy',
    ));

    switch ($ploidy) {
      case 'Haploid':
        $fields['files']['ssrs']['#description'] .= ' For haploid, TPPS assumes that each remaining column in the spreadsheet is a marker.';
        break;

      case 'Diploid':
        $fields['files']['ssrs']['#description'] .= ' For diploid, TPPS will assume that pairs of columns together are describing an individual marker, so the second and third columns would be the first marker, the fourth and fifth columns would be the second marker, etc.';
        break;

      case 'Polyploid':
        $fields['files']['ssrs']['#description'] .= ' For polyploid, TPPS will read columns until it arrives at a non-empty column with a different name from the last.';
        break;

      default:
        break;
    }

    if (isset($fields['files']['ssrs']['#value']['fid'])) {
      $fields['files']['ssrs']['#default_value'] = $fields['files']['ssrs']['#value']['fid'];
    }
    if (!empty($fields['files']['ssrs']['#default_value']) and ($file = file_load($fields['files']['ssrs']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }

    $fields['files']['ssr-extra-check'] = array(
      '#type' => 'checkbox',
      '#title' => t('I would like to include an additional SSRs/cpSSRs Spreadsheet (this is typically used when the study includes both SSR and cpSSR data)'),
      '#ajax' => array(
        'callback' => 'tpps_genotype_files_callback',
        'wrapper' => "$id-genotype-files",
      ),
    );

    $ssrs_extra_check = tpps_get_ajax_value($form_state, array(
      $id,
      'genotype',
      'files',
      'ssr-extra-check',
    ));

    if ($ssrs_extra_check) {
      $fields['files']['extra-ssr-type'] = array(
        '#type' => 'textfield',
        '#title' => t('Define Additional SSRs/cpSSRs Type: *'),
      );

      $fields['files']['extra-ploidy'] = array(
        '#type' => 'select',
        '#title' => t('Additional SSRs/cpSSRs Ploidy'),
        '#options' => array(
          0 => t('- Select -'),
          'Haploid' => t('Haploid'),
          'Diploid' => t('Diploid'),
          'Polyploid' => t('Polyploid'),
        ),
        '#ajax' => array(
          'callback' => 'tpps_genotype_files_callback',
          'wrapper' => "$id-genotype-files",
        ),
      );

      $fields['files']['ssrs_extra'] = array(
        '#type' => 'managed_file',
        '#title' => t('SSRs/cpSSRs Additional Spreadsheet: *'),
        '#upload_location' => "$genotype_upload_location",
        '#upload_validators' => array(
          'file_validate_extensions' => array('csv tsv xlsx'),
        ),
        '#description' => t('Please upload an additional spreadsheet containing your SSRs/cpSSRs data. The format of this file is very important! TPPS will parse your file based on the ploidy you have selected above. For any ploidy, TPPS will assume that the first column of your file is the column that holds the Plant Identifier that matches your accession file.'),
        '#tree' => TRUE,
      );

      $extra_ploidy = tpps_get_ajax_value($form_state, array(
        $id,
        'genotype',
        'files',
        'extra-ploidy',
      ));

      switch ($extra_ploidy) {
        case 'Haploid':
          $fields['files']['ssrs_extra']['#description'] .= ' For haploid, TPPS assumes that each remaining column in the spreadsheet is a marker.';
          break;

        case 'Diploid':
          $fields['files']['ssrs_extra']['#description'] .= ' For diploid, TPPS will assume that pairs of columns together are describing an individual marker, so the second and third columns would be the first marker, the fourth and fifth columns would be the second marker, etc.';
          break;

        case 'Polyploid':
          $fields['files']['ssrs_extra']['#description'] .= ' For polyploid, TPPS will read columns until it arrives at a non-empty column with a different name from the last.';
          break;

        default:
          break;
      }

      if (isset($fields['files']['ssrs_extra']['#value']['fid'])) {
        $fields['files']['ssrs_extra']['#default_value'] = $fields['files']['ssrs_extra']['#value']['fid'];
      }
      if (!empty($fields['files']['ssrs_extra']['#default_value']) and ($file = file_load($fields['files']['ssrs_extra']['#default_value']))) {
        // Stop using the file so it can be deleted if the user clicks 'remove'.
        file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      }
    }
    else {
      $fields['files']['ssrs-extra'] = array(
        '#type' => 'managed_file',
        '#tree' => TRUE,
        '#access' => FALSE,
      );
    }
  }
  else {
    $fields['files']['ssrs'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );

    $fields['files']['ssrs-extra'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($indel_file_check)) {
    $fields['files']['indels'] = array(
      '#type' => 'managed_file',
      '#title' => t('Indel Genotype Spreadsheet: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#description' => t('Please upload a spreadsheet containing your Indels data. The first column of your file should contain plant identifiers which match the plant identifiers you provided in your plant accession file, and all of the remaining columns should contain Indel data.'),
      '#tree' => TRUE,
    );

    if (isset($fields['files']['indels']['#value']['fid'])) {
      $fields['files']['indels']['#default_value'] = $fields['files']['indels']['#value']['fid'];
    }
    if (!empty($fields['files']['indels']['#default_value']) and ($file = file_load($fields['files']['indels']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }
  }
  else {
    $fields['files']['indels'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($other_file_check)) {
    $fields['files']['other'] = array(
      '#type' => 'managed_file',
      '#title' => t('Other Marker Genotype Spreadsheet: please provide a spreadsheet with columns for the Plant ID of genotypes used in this study: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#description' => t('Please upload a spreadsheet file containing Genotype data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may set that drop-down menu to "N/A". Your file must contain one column with the Plant Identifier.'),
      '#tree' => TRUE,
    );

    $fields['files']['other']['empty'] = array(
      '#default_value' => $values[$id]['genotype']['files']['other']['empty'] ?? 'NA',
    );

    $default_dynamic = !empty($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['other-columns']);
    $fields['files']['other']['dynamic'] = array(
      '#type' => 'checkbox',
      '#title' => t('This file needs dynamic dropdown options for column data type specification'),
      '#ajax' => array(
        'wrapper' => "edit-$id-genotype-files-other-ajax-wrapper",
        'callback' => 'tpps_page_4_file_dynamic',
      ),
      '#default_value' => $default_dynamic,
    );

    $dynamic = tpps_get_ajax_value($form_state, array(
      $id,
      'genotype',
      'files',
      'other',
      'dynamic',
    ), $default_dynamic, 'other');
    if ($dynamic) {
      $fields['files']['other']['columns'] = array(
        '#description' => t('Please define which columns hold the required data: Plant Identifier, Genotype Data'),
      );

      $fields['files']['other']['columns-options'] = array(
        '#type' => 'hidden',
        '#value' => array(
          'Genotype Data',
          'Plant Identifier',
          'N/A',
        ),
      );
    }

    $fields['files']['other']['no-header'] = array();
  }
  else {
    $fields['files']['other'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($vcf_file_check)) {
    $fields['files']['vcf'] = array(
      '#type' => 'managed_file',
      '#title' => t('Genotype VCF File: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('gz tar zip'),
      ),
      '#tree' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][files][local_vcf_check]"]' => array('checked' => FALSE),
        ),
      ),
    );

    if (isset($fields['files']['vcf']['#value'])) {
      $fields['files']['vcf']['#default_value'] = $fields['files']['vcf']['#value'];
    }
    if (!empty($fields['files']['vcf']['#default_value']) and ($file = file_load($fields['files']['vcf']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }

    if (isset($form_state['tpps_type']) and $form_state['tpps_type'] == 'tppsc') {
      global $base_url;
      $parts = explode('://', $base_url);
      $hostname = $parts[1];
      $fields['files']['local_vcf_check'] = array(
        '#type' => 'checkbox',
        '#title' => t("My VCF file is stored locally on @hostname", array('@hostname' => $hostname)),
      );

      $fields['files']['local_vcf'] = array(
        '#type' => 'textfield',
        '#title' => t('Path to local VCF File: *'),
        '#states' => array(
          'visible' => array(
            ':input[name="' . $id . '[genotype][files][local_vcf_check]"]' => array('checked' => TRUE),
          ),
        ),
        '#description' => t("Please provide the full path to your vcf file stored locally on @hostname", array('@hostname' => $hostname)),
      );
    }
  }
  else {
    $fields['files']['vcf'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  return $fields;
}

/**
 * Creates fields describing the environmental data for the submission.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_environment(array &$form, array &$form_state, $id) {
  $cartogratree_env = variable_get('tpps_cartogratree_env', FALSE);

  $form[$id]['environment'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Environmental Information:</div>'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
  );

  if ($cartogratree_env and db_table_exists('cartogratree_groups') and db_table_exists('cartogratree_layers') and db_table_exists('cartogratree_fields')) {

    $query = db_select('variable', 'v')
      ->fields('v')
      ->condition('name', db_like('tpps_layer_group_') . '%', 'LIKE');

    $results = $query->execute();
    $options = array();

    while (($result = $results->fetchObject())) {
      $group_id = substr($result->name, 17);
      $group = db_select('cartogratree_groups', 'g')
        ->fields('g', array('group_id', 'group_name'))
        ->condition('group_id', $group_id)
        ->execute()
        ->fetchObject();
      $group_is_enabled = variable_get("tpps_layer_group_$group_id", FALSE);

      if ($group_is_enabled) {
        if ($group->group_name == 'WorldClim v.2 (WorldClim)') {
          $subgroups_query = db_select('cartogratree_layers', 'c')
            ->distinct()
            ->fields('c', array('subgroup_id'))
            ->condition('c.group_id', $group_id)
            ->execute();
          while (($subgroup = $subgroups_query->fetchObject())) {
            $subgroup_title = db_select('cartogratree_subgroups', 's')
              ->fields('s', array('subgroup_name'))
              ->condition('subgroup_id', $subgroup->subgroup_id)
              ->execute()
              ->fetchObject()->subgroup_name;
            $options["worldclim_subgroup_{$subgroup->subgroup_id}"] = array(
              'group_id' => $group_id,
              'group' => $group->group_name,
              'title' => $subgroup_title,
              'params' => NULL,
            );
          }
        }
        else {
          $layers_query = db_select('cartogratree_layers', 'c')
            ->fields('c', array('title', 'group_id', 'layer_id'))
            ->condition('c.group_id', $group_id);
          $layers_results = $layers_query->execute();
          while (($layer = $layers_results->fetchObject())) {
            $params_query = db_select('cartogratree_fields', 'f')
              ->fields('f', array('display_name', 'field_id'))
              ->condition('f.layer_id', $layer->layer_id);
            $params_results = $params_query->execute();
            $params = array();
            while (($param = $params_results->fetchObject())) {
              $params[$param->field_id] = $param->display_name;
            }
            $options[$layer->layer_id] = array(
              'group_id' => $layer->group_id,
              'group' => $group->group_name,
              'title' => $layer->title,
              'params' => $params,
            );
          }
        }
      }
    }

    $form[$id]['environment']['env_layers_groups'] = array(
      '#type' => 'fieldset',
      '#title' => 'CartograPlant Environmental Layer Groups: *',
      '#collapsible' => TRUE,
    );

    $form[$id]['environment']['layer_search'] = array(
      '#type' => 'textfield',
      '#title' => t('Layers Search'),
      '#description' => t('You can use this field to filter the layers in the following section.'),
    );

    $form[$id]['environment']['env_layers'] = array(
      '#type' => 'fieldset',
      '#title' => 'CartograPlant Environmental Layers: *',
      '#collapsible' => TRUE,
    );

    $form[$id]['environment']['env_params'] = array(
      '#type' => 'fieldset',
      '#title' => 'CartograPlant Environmental Layer Parameters: *',
      '#collapsible' => TRUE,
    );

    foreach ($options as $layer_id => $layer_info) {
      $layer_title = $layer_info['title'];
      $layer_group = $layer_info['group'];
      $layer_params = $layer_info['params'];

      $form[$id]['environment']['env_layers_groups'][$layer_group] = array(
        '#type' => 'checkbox',
        '#title' => $layer_group,
        '#return_value' => $layer_info['group_id'],
      );

      $form[$id]['environment']['env_layers'][$layer_title] = array(
        '#type' => 'checkbox',
        '#title' => "<strong>$layer_title</strong> - $layer_group",
        '#states' => array(
          'visible' => array(
            ':input[name="' . $id . '[environment][env_layers_groups][' . $layer_group . ']"]' => array('checked' => TRUE),
          ),
        ),
        '#return_value' => $layer_id,
      );

      if (!empty($layer_params)) {
        $form[$id]['environment']['env_params']["$layer_title"] = array(
          '#type' => 'fieldset',
          '#title' => "$layer_title Parameters",
          '#description' => t('Please select the parameters you used from the @title layer.', array('@title' => $layer_title)),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[environment][env_layers_groups][' . $layer_group . ']"]' => array('checked' => TRUE),
              ':input[name="' . $id . '[environment][env_layers][' . $layer_title . ']"]' => array('checked' => TRUE),
            ),
          ),
        );

        foreach ($layer_params as $param_id => $param) {
          $form[$id]['environment']['env_params']["$layer_title"][$param] = array(
            '#type' => 'checkbox',
            '#title' => $param,
            '#return_value' => $param_id,
          );
        }
      }
    }

    $form[$id]['environment']['env_layers']['other'] = array(
      '#type' => 'checkbox',
      '#title' => "<strong>Other custom layer</strong>",
      '#return_value' => 'other',
    );

    $form[$id]['environment']['env_layers']['other_db'] = array(
      '#type' => 'textfield',
      '#title' => t('Layer DB URL: *'),
      '#description' => t('The url of the DB providing this layer'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][env_layers][other]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form[$id]['environment']['env_layers']['other_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Layer Name: *'),
      '#description' => t('The name of the layer'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][env_layers][other]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form[$id]['environment']['env_layers']['other_params'] = array(
      '#type' => 'textfield',
      '#title' => t('Parameters Used: *'),
      '#description' => t('Comma-delimited list of parameters from the layer used. For example, when using parameters "rainfall" and "humidity", this field should look something like "rainfall,humidity"'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][env_layers][other]"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  return $form[$id]['environment'];
}

/**
 * Creates fields for the user to specify a reference genome.
 *
 * @param array $fields
 *   The form element being populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @global stdClass $user
 *   The user accessing the form.
 */
function tpps_page_4_ref(array &$fields, array &$form_state, $id) {
  global $user;
  $uid = $user->uid;

  $options = array(
    'key' => 'filename',
    'recurse' => FALSE,
  );

  $genome_dir = variable_get('tpps_local_genome_dir', NULL);
  $ref_genome_arr = array();
  $ref_genome_arr[0] = '- Select -';

  if ($genome_dir) {
    $existing_genomes = array();
    $results = file_scan_directory($genome_dir, '/^([A-Z][a-z]{3})$/', $options);
    $code_cvterm = tpps_load_cvterm('organism 4 letter code')->cvterm_id;
    foreach ($results as $key => $value) {
      $org_id_query = chado_select_record('organismprop', array('organism_id'), array(
        'value' => $key,
        'type_id' => $code_cvterm,
      ));

      if (!empty($org_id_query)) {
        $org_query = chado_select_record('organism', array('genus', 'species'), array(
          'organism_id' => current($org_id_query)->organism_id,
        ));
        $result = current($org_query);

        $versions = file_scan_directory("$genome_dir/$key", '/^v([0-9]|.)+$/', $options);
        foreach ($versions as $item) {
          $opt_string = $result->genus . " " . $result->species . " " . $item->filename;
          $existing_genomes[$opt_string] = $opt_string;
        }
      }
    }
    ksort($existing_genomes);
    $ref_genome_arr += $existing_genomes;
  }

  $ref_genome_arr["url"] = 'I can provide a URL to the website of my reference file(s)';
  $ref_genome_arr["bio"] = 'I can provide a GenBank accession number (BioProject, WGS, TSA) and select assembly file(s) from a list';
  $ref_genome_arr["manual"] = 'I can upload my own reference genome file';
  $ref_genome_arr["manual2"] = 'I can upload my own reference transcriptome file';
  $ref_genome_arr["none"] = 'I am unable to provide a reference assembly';

  $fields['ref-genome'] = array(
    '#type' => 'select',
    '#title' => t('Reference Assembly used: *'),
    '#options' => $ref_genome_arr,
  );

  require_once drupal_get_path('module', 'tripal') . '/includes/tripal.importer.inc';
  $class = 'EutilsImporter';
  tripal_load_include_importer_class($class);
  $eutils = tripal_get_importer_form(array(), $form_state, $class);
  $eutils['#type'] = 'fieldset';
  $eutils['#title'] = 'Tripal Eutils BioProject Loader';
  $eutils['#states'] = array(
    'visible' => array(
      ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'bio'),
    ),
  );
  $eutils['accession']['#description'] = t('Valid examples: 12384, 394253, 66853, PRJNA185471');
  $eutils['db'] = array(
    '#type' => 'hidden',
    '#value' => 'bioproject',
  );
  unset($eutils['options']);
  $eutils['options']['linked_records'] = array(
    '#type' => 'hidden',
    '#value' => 1,
  );
  $eutils['callback']['#ajax'] = array(
    'callback' => 'tpps_ajax_bioproject_callback',
    'wrapper' => "$id-tripal-eutils",
  );
  $eutils['#prefix'] = "<div id=\"$id-tripal-eutils\">";
  $eutils['#suffix'] = '</div>';

  if (!empty($form_state['values'][$id]['genotype']['tripal_eutils'])) {
    $eutils_vals = $form_state['values'][$id]['genotype']['tripal_eutils'];
    if (!empty($eutils_vals['accession']) and !empty($eutils_vals['db'])) {
      $connection = new \EUtils();
      try {
        $connection->setPreview(TRUE);
        $eutils['data'] = $connection->get($eutils_vals['db'], $eutils_vals['accession']);
        foreach ($_SESSION['messages']['status'] as $key => $message) {
          if ($message == '<pre>biosample</pre>') {
            unset($_SESSION['messages']['status'][$key]);
            if (empty($_SESSION['messages']['status'])) {
              unset($_SESSION['messages']['status']);
            }
            break;
          }
        }
      }
      catch (\Exception $e) {
        tripal_set_message($e->getMessage(), TRIPAL_ERROR);
      }
    }
  }
  unset($eutils['button']);
  unset($eutils['instructions']);
  $fields['tripal_eutils'] = $eutils;

  $class = 'FASTAImporter';
  tripal_load_include_importer_class($class);
  $tripal_upload_location = "public://tripal/users/$uid";

  $fasta = tripal_get_importer_form(array(), $form_state, $class);
  $fasta['#type'] = 'fieldset';
  $fasta['#title'] = 'Tripal FASTA Loader';
  $fasta['#states'] = array(
    'visible' => array(
    array(
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'url')),
      'or',
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual')),
      'or',
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual2')),
    ),
    ),
  );

  unset($fasta['file']['file_local']);
  unset($fasta['organism_id']);
  unset($fasta['method']);
  unset($fasta['match_type']);
  $db = $fasta['additional']['db'];
  unset($fasta['additional']);
  $fasta['db'] = $db;
  $fasta['db']['#collapsible'] = TRUE;
  unset($fasta['button']);

  $upload = array(
    '#type' => 'managed_file',
    '#title' => '',
    '#description' => t('Remember to click the "Upload" button below to send your file to the server.  This interface is capable of uploading very large files.  If you are disconnected you can return, reload the file and it will resume where it left off.  Once the file is uploaded the "Upload Progress" will indicate "Complete".  If the file is already present on the server then the status will quickly update to "Complete".'),
    '#upload_validators' => array(
      'file_validate_extensions' => array(implode(' ', $class::$file_types)),
    ),
    '#upload_location' => $tripal_upload_location,
  );

  $fasta['file']['file_upload'] = $upload;
  $fasta['analysis_id']['#required'] = $fasta['seqtype']['#required'] = FALSE;
  $fasta['file']['file_upload']['#states'] = $fasta['file']['file_upload_existing']['#states'] = array(
    'visible' => array(
    array(
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual')),
      'or',
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual2')),
    ),
    ),
  );
  $fasta['file']['file_remote']['#states'] = array(
    'visible' => array(
      ':input[name="]' . $id . '[genotype][ref-genome]"]' => array('value' => 'url'),
    ),
  );

  $fields['tripal_fasta'] = $fasta;
}

/**
 * Creates fields describing the genotype markers used in the submission.
 *
 * @param array $fields
 *   The form element being populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 */
function tpps_page_4_marker_info(array &$fields, $id) {

  // [VS] #8669rmvf2
  $fields['marker-type'] = [
    '#type' => 'select',
    '#title' => t('Marker Type (select all that apply): *'),
    '#options' => drupal_map_assoc([
      t('SNPs'),
      t('SSRs/cpSSRs'),
      t('Indels'),
      t('Other'),
    ]),
  ];
  // [/VS]

  $fields['marker-type']['#ajax'] = array(
    'callback' => 'tpps_genotype_files_callback',
    'wrapper' => "$id-genotype-files",
  );

  $fields['SNPs'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">SNPs Information:</div>'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][marker-type][SNPs]"]' => array('checked' => TRUE),
      ),
    ),
    '#collapsible' => TRUE,
  );

  $fields['SNPs']['genotyping-design'] = array(
    '#type' => 'select',
    '#title' => t('Define Experimental Design: *'),
    '#options' => array(
      0 => t('- Select -'),
      1 => t('GBS'),
      2 => t('Targeted Capture'),
      3 => t('Whole Genome Resequencing'),
      4 => t('RNA-Seq'),
      5 => t('Genotyping Array'),
    ),
  );

  $fields['SNPs']['GBS'] = array(
    '#type' => 'select',
    '#title' => t('GBS Type: *'),
    '#options' => array(
      0 => t('- Select -'),
      1 => t('RADSeq'),
      2 => t('ddRAD-Seq'),
      3 => t('NextRAD'),
      4 => t('RAPTURE'),
      5 => t('Other'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1'),
      ),
    ),
  );

  $fields['SNPs']['GBS-other'] = array(
    '#type' => 'textfield',
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][GBS]"]' => array('value' => '5'),
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1'),
      ),
    ),
  );

  $fields['SNPs']['targeted-capture'] = array(
    '#type' => 'select',
    '#title' => t('Targeted Capture Type: *'),
    '#options' => array(
      0 => t('- Select -'),
      1 => t('Exome Capture'),
      2 => t('Other'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2'),
      ),
    ),
  );

  $fields['SNPs']['targeted-capture-other'] = array(
    '#type' => 'textfield',
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][targeted-capture]"]' => array('value' => '2'),
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2'),
      ),
    ),
  );

  $fields['SSRs/cpSSRs'] = array(
    '#type' => 'textfield',
    '#title' => t('Define SSRs/cpSSRs Type: *'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][marker-type][SSRs/cpSSRs]"]' => array('checked' => TRUE),
      ),
    ),
  );

  $fields['other-marker'] = array(
    '#type' => 'textfield',
    '#title' => t('Define Other Marker Type: *'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][marker-type][Other]"]' => array('checked' => TRUE),
      ),
    ),
  );
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
  return array(
    '#type' => 'textfield',
    '#title' => 'Phenotype !num Name: *',
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('right'),
      // 'title' => array('If your phenotype name is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
      'title' => array('If your phenotype name does not exist in our database, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
    ),
    '#description' => t('Phenotype "name" is the human-readable name of the phenotype, where "attribute" is the thing that the phenotype is describing. Phenotype "name" should match the data in the "Phenotype Name/Identifier" column that you select in your <a href="@url">Phenotype file</a> below.', array('@url' => url('/tpps', array('fragment' => "edit-$id-phenotype-file-ajax-wrapper")))),
  );
}

/**
 * Builds Phenotype Description form field.
 *
 * @param string $id
 *   Organism Id.
 *
 * @return array
 *   Returns Form API field.
 */
function tpps_build_field_description() {
  return array(
    '#type' => 'textfield',
    '#title' => 'Phenotype !num Description: *',
    '#description' => t('Please provide a short description of Phenotype !num'),
  );
}

/**
 * Builds a selector for 'Synonym Missing' checkbox for form states.
 *
 * @param string $id
 *   Organism Id.
 *
 * @return string
 *   Ready for form state selector.
 */
function tpps_synonym_selector($id) {
  return ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][synonym_id]"]';
}
