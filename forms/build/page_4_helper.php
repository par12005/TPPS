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

  $form[$id]['phenotype']['iso-check'] = array(
    '#type' => 'checkbox',
    '#title' => t('My phenotypes are results from a mass spectrometry or isotope analysis'),
    '#ajax' => array(
      'callback' => 'tpps_update_phenotype',
      'wrapper' => "phenotype-main-$id",
    ),
  );

  $iso_check = tpps_get_ajax_value($form_state, array(
    $id,
    'phenotype',
    'iso-check',
  ), NULL);

  if (!empty($iso_check)) {
    $fields['iso'] = array(
      '#type' => 'managed_file',
      '#title' => t('Phenotype Isotope/Mass Spectrometry file: *'),
      '#upload_location' => $phenotype_upload_location,
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#description' => 'Please upload a file containing all of your isotope/mass spectrometry data. The format of this file is very important! The first column of your file should contain plant identifiers which match the plant identifiers you provided in your plant accession file, and all of the remaining columns should contain isotope or mass spectrometry data.',
    );

    return $fields;
  }

  $attr_options = array();
  $terms = array(
    'age' => 'Age',
    'alive' => 'Alive',
    'amount' => 'Amount',
    'angle' => 'Angle',
    'area' => 'Area',
    'bent' => 'Bent',
    'circumference' => 'Circumerence',
    'color' => 'Color',
    'composition' => 'Composition',
    'concentration_of' => 'Concentration of',
    'damage' => 'Damage',
    'description' => 'Description',
    'diameter' => 'Diameter',
    'distance' => 'Distance',
    'growth_quality_of_occurrent' => 'Growth Quality of Occurrent',
    'growth_rate' => 'Growth Rate',
    'has_number_of' => 'Has number of',
    'height' => 'Height',
    'humidity_level' => 'Humidity Level',
    'intensity' => 'Intensity',
    'length' => 'Length',
    'lesioned' => 'Lesioned',
    'maturity' => 'Maturity',
    'position' => 'Position',
    'pressure' => 'Pressure',
    'proportionality_to' => 'Proportionality to',
    'rate' => 'Rate',
    'rough' => 'Rough',
    'shape' => 'Shape',
    'size' => 'Size',
    'temperature' => 'Temperature',
    'texture' => 'Texture',
    'thickness' => 'Thickness',
    'time' => 'Time',
    'volume' => 'Volume',
    'weight' => 'Weight',
    'width' => 'Width',
  );
  foreach ($terms as $term => $label) {
    $attr_id = tpps_load_cvterm($term)->cvterm_id;
    $attr_options[$attr_id] = $label;
  }
  $attr_options['other'] = 'My attribute term is not in this list';

  $unit_options = array();
  $terms = array(
    'centimeter' => 'Centimeter',
    'cubic_centimeter' => 'Cubic Centimeter',
    'day' => 'Day',
    'degrees_celsius' => 'Degrees Celsius',
    'degrees_fahrenheit' => 'Degrees Fahrenheit',
    'grams_per_square_meter' => 'Grams per Square Meter',
    'gram' => 'Gram',
    'luminous_intensity_unit' => 'Luminous Intensity Unit',
    'kilogram' => 'Kilogram',
    'kilogram_per_cubic_meter' => 'Kilogram per Cubic Meter',
    'liter' => 'Liter',
    'cubic_meter' => 'Cubic Meter',
    'pascal' => 'Pascal',
    'meter' => 'Meter',
    'milligram' => 'Milligram',
    'milliliter' => 'Milliliter',
    'millimeter' => 'Millimeter',
    'micrometer' => 'Micrometer',
    'percent' => 'Percent',
    'qualitative' => 'Qualitative',
    'square_micrometer' => 'Square Micrometer',
    'square_millimeter' => 'Square Millimeter',
    'watt_per_square_meter' => 'Watt per Square Meter',
    'year' => 'Year',
  );
  foreach ($terms as $term => $label) {
    $unit_id = tpps_load_cvterm($term)->cvterm_id;
    $unit_options[$unit_id] = $label;
  }
  $unit_options['other'] = 'My unit is not in this list';

  $struct_options = array();
  $terms = array(
    'whole plant' => 'Whole Plant',
    'bark' => 'Bark',
    'branch' => 'Branch',
    'bud' => 'Bud',
    'catkin_inflorescence' => 'Catkin Inflorescence',
    'endocarp' => 'Endocarp',
    'floral_organ' => 'Floral Organ',
    'flower' => 'Flower',
    'flower_bud' => 'Flower Bud',
    'flower_fascicle' => 'Flower Fascicle',
    'fruit' => 'Fruit',
    'leaf' => 'Leaf',
    'leaf_rachis' => 'Leaf Rachis',
    'leaflet' => 'Leaflet',
    'nut_fruit' => 'Nut Fruit (Acorn)',
    'petal' => 'Petal',
    'petiole' => 'Petiole',
    'phloem' => 'Phloem',
    'plant_callus' => 'Plant Callus (Callus)',
    'primary_thickening_meristem' => 'Primary Thickening Meristem',
    'root' => 'Root',
    'secondary_xylem' => 'Secondary Xylem (Wood)',
    'seed' => 'Seed',
    'shoot_system' => 'Shoot System (Crown)',
    'stem' => 'Stem (Trunk, Primary Stem)',
    'stomatal_complex' => 'Stomatal Complex (Stomata)',
    'strobilus' => 'Strobilus',
    'terminal_bud' => 'Terminal Bud',
    'vascular_leaf' => 'Vascular Leaf (Needle)',
  );
  foreach ($terms as $term => $label) {
    $struct_id = tpps_load_cvterm($term)->cvterm_id;
    $struct_options[$struct_id] = $label;
  }
  $struct_options['other'] = 'My structure term is not in this list';

  $field = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    'name' => array(
      '#type' => 'textfield',
      '#title' => 'Phenotype !num Name: *',
      '#autocomplete_path' => 'tpps/autocomplete/phenotype',
      '#prefix' => "<label><b>Phenotype !num:</b></label>",
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('If your phenotype name is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
      ),
      '#description' => t('Phenotype "name" is the human-readable name of the phenotype, where "attribute" is the thing that the phenotype is describing. Phenotype "name" should match the data in the "Phenotype Name/Identifier" column that you select in your <a href="@url">Phenotype file</a> below.', array('@url' => url('/tpps', array('fragment' => "edit-$id-phenotype-file-ajax-wrapper")))),
    ),
    'attribute' => array(
      '#type' => 'select',
      '#title' => 'Phenotype !num Attribute: *',
      '#options' => $attr_options,
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
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][attribute]"]' => array('value' => 'other'),
        ),
      ),
    ),
    'description' => array(
      '#type' => 'textfield',
      '#title' => 'Phenotype !num Description: *',
      '#description' => 'Please provide a short description of Phenotype !num',
    ),
    'units' => array(
      '#type' => 'select',
      '#title' => 'Phenotype !num Units: *',
      '#options' => $unit_options,
    ),
    'unit-other' => array(
      '#type' => 'textfield',
      '#title' => 'Phenotype !num Custom Units: *',
      '#autocomplete_path' => 'tpps/autocomplete/units',
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('If your unit is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
      ),
      '#description' => t('Some examples of units include: "m", "meters", "in", "inches", "Degrees Celsius", "°C", etc.'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][units]"]' => array('value' => 'other'),
        ),
      ),
    ),
    'structure' => array(
      '#type' => 'select',
      '#title' => 'Phenotype !num Structure: *',
      '#options' => $struct_options,
      #'#default_value' => tpps_load_cvterm('whole plant')->cvterm_id,
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
    'val-check' => array(
      '#type' => 'checkbox',
      '#title' => 'Phenotype !num has a value range',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][bin-check]"]' => array('checked' => FALSE),
        ),
      ),
    ),
    'bin-check' => array(
      '#type' => 'checkbox',
      '#title' => 'Phenotype !num is a binary phenotype',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][val-check]"]' => array('checked' => FALSE),
        ),
      ),
    ),
    'min' => array(
      '#type' => 'textfield',
      '#title' => 'Phenotype !num Minimum Value (type 1 for binary): *',
      '#states' => array(
        'invisible' => array(
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][val-check]"]' => array('checked' => FALSE),
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][bin-check]"]' => array('checked' => FALSE),
        ),
      ),
    ),
    'max' => array(
      '#type' => 'textfield',
      '#title' => 'Phenotype !num Maximum Value (type 2 for binary): *',
      '#states' => array(
        'invisible' => array(
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][val-check]"]' => array('checked' => FALSE),
          ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][bin-check]"]' => array('checked' => FALSE),
        ),
      ),
    ),
    'env-check' => array(
      '#type' => 'checkbox',
      '#title' => 'Phenotype !num is an environmental phenotype',
    ),
  );

  tpps_dynamic_list($form, $form_state, 'phenotypes-meta', $field, array(
    'label' => 'Phenotype',
    'title' => "",
    'callback' => 'tpps_update_phenotype',
    'parents' => array($id, 'phenotype'),
    'wrapper' => "phenotype-main-$id",
    'name_suffix' => $id,
    'alternative_buttons' => array(
      "Add 5 Phenotypes" => 5,
      "Add 10 Phenotypes" => 10,
      "Clear All Phenotypes" => 'tpps_phenotype_number_clear',
    ),
    'button_weights' => array(
      "Add Phenotype" => -5,
      "Add 5 Phenotypes" => -4,
      "Add 10 Phenotypes" => -3,
      "Remove Phenotype" => -2,
      "Clear All Phenotypes" => -1,
    ),
    'substitute_fields' => array(
      array('name', '#title'),
      array('name', '#prefix'),
      array('attribute', '#title'),
      array('attr-other', '#title'),
      array('description', '#title'),
      array('description', '#description'),
      array('units', '#title'),
      array('unit-other', '#title'),
      array('structure', '#title'),
      array('struct-other', '#title'),
      array('val-check', '#title'),
      array('bin-check', '#title'),
      array('min', '#title'),
      array('max', '#title'),
      array('env-check', '#title'),
    ),
    'substitute_keys' => array(
      array('attr-other', '#states', 'visible', ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][attribute]"]'),
      array('unit-other', '#states', 'visible', ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][units]"]'),
      array('struct-other', '#states', 'visible', ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][structure]"]'),
      array('val-check', '#states', 'visible', ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][bin-check]"]'),
      array('bin-check', '#states', 'visible', ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][val-check]"]'),
      array('min', '#states', 'invisible', ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][val-check]"]'),
      array('min', '#states', 'invisible', ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][bin-check]"]'),
      array('max', '#states', 'invisible', ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][val-check]"]'),
      array('max', '#states', 'invisible', ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][bin-check]"]'),
    ),
  ));

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
    '#description' => 'Please define which columns hold the required data: Phenotype name',
  );

  $column_options = array(
    'N/A',
    'Phenotype Name/Identifier',
    'Attribute',
    'Description',
    'Units',
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
  $meta = tpps_get_ajax_value($form_state, array($id, 'phenotype', 'phenotypes-meta'));
  $number = tpps_get_ajax_value($form_state, array($id, 'phenotype', 'phenotypes-meta', 'number'));
  $phenotype_names = array();
  for ($i = 1; $i <= $number; $i++) {
    if (!empty($meta[$i]['name'])) {
      $phenotype_names[] = is_array($meta[$i]['name']) ? $meta[$i]['name']['#value'] : $meta[$i]['name'];
    }
  }

  // Get names of phenotypes in metadata file.
  $columns = tpps_get_ajax_value($form_state, array($id, 'phenotype', 'metadata', 'columns'), array(), 'metadata');
  $meta_fid = tpps_get_ajax_value($form_state, array($id, 'phenotype', 'metadata'));
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

  $time_check = tpps_get_ajax_value($form_state, array($id, 'phenotype', 'time', 'time-check'), $time_default);
  if ($time_check) {
    $time_options = array();
    foreach ($phenotype_names as $name) {
      $time_options[strtolower($name)] = $name;
    }
    $form[$id]['phenotype']['time']['time_phenotypes'] = array(
      '#type' => 'checkboxes',
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
        0 => '- Select -',
        'Haploid' => 'Haploid',
        'Diploid' => 'Diploid',
        'Polyploid' => 'Polyploid',
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

  $fields['files']['file-type'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Genotype File Types (select all that apply): *'),
    '#options' => $options,
    '#ajax' => array(
      'callback' => 'tpps_genotype_files_callback',
      'wrapper' => "$id-genotype-files",
    ),
  );

  $parents = array_merge($file_type_parents, array('VCF'));
  $vcf_file_check = tpps_get_ajax_value($form_state, $parents);

  if (!empty($snps_assay_check)) {
    $fields['files']['snps-assay'] = array(
      '#type' => 'managed_file',
      '#title' => t('SNPs Genotype Assay File: please provide a spreadsheet with columns for the Plant ID of genotypes used in this study: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#description' => "Please upload a spreadsheet file containing SNP Genotype Assay data. The format of this file is very important! The first column of your file should contain plant identifiers which match the plant identifiers you provided in your plant accession file, and all of the remaining columns should contain SNP data.",
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
    $fields['files']['snps-assay'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($assay_design_check)) {
    $fields['files']['assay-design'] = array(
      '#type' => 'managed_file',
      '#title' => 'Genotype Assay Design File: *',
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#tree' => TRUE,
    );

    if (isset($fields['files']['assay-design']['#value'])) {
      $fields['files']['assay-design']['#default_value'] = $fields['files']['assay-design']['#value'];
    }
    if (!empty($fields['files']['assay-design']['#default_value']) and ($file = file_load($fields['files']['assay-design']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }
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
      '#description' => "Please upload a spreadsheet file containing SNPs Association data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may set that drop-down menu to \"N/A\". Your file must contain columns with the SNP ID, Scaffold, Position (formatted like \"start:stop\"), Allele (formatted like \"major:minor\"), Associated Trait Name (must match a phenotype from the above section), and Confidence Value. Optionally, you can also specify a Gene ID (which should match the gene reference) and a SNP Annotation (non synonymous, coding, etc).",
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
        '- Select -',
        'P value' => 'P value',
        'Genomic Inflation Factor (GIF)' => 'Genomic Inflation Factor (GIF)',
        'P-adjusted (FDR) / Q value' => 'P-adjusted (FDR) / Q value',
        'P-adjusted (FWE)' => 'P-adjusted (FWE)',
        'P-adjusted (Bonferroni)' => 'P-adjusted (Bonferroni)',
      ),
    );

    $fields['files']['snps-association-tool'] = array(
      '#type' => 'select',
      '#title' => t('Association Analysis Tool: *'),
      '#options' => array(
        '- Select -',
        'GEMMA' => 'GEMMA',
        'EMMAX' => 'EMMAX',
        'Plink' => 'Plink',
        'Tassel' => 'Tassel',
        'Sambada' => 'Sambada',
        'Bayenv' => 'Bayenv',
        'BayeScan' => 'BayeScan',
        'LFMM' => 'LFMM',
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
          0 => '- Select -',
          'Haploid' => 'Haploid',
          'Diploid' => 'Diploid',
          'Polyploid' => 'Polyploid',
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
      '#description' => "Please upload a spreadsheet file containing Genotype data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may set that drop-down menu to \"N/A\". Your file must contain one column with the Plant Identifier.",
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
      '#description' => t(''),
      '#default_value' => $default_dynamic,
    );

    $dynamic = tpps_get_ajax_value($form_state, array($id, 'genotype', 'files', 'other', 'dynamic'), $default_dynamic, 'other');
    if ($dynamic) {
      $fields['files']['other']['columns'] = array(
        '#description' => 'Please define which columns hold the required data: Plant Identifier, Genotype Data',
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
        #'file_validate_extensions' => array('gz tar zip'),
        'file_validate_extensions' => array('zip'),
      ),
      '#tree' => TRUE,
    );

    if (isset($fields['files']['vcf']['#value'])) {
      $fields['files']['vcf']['#default_value'] = $fields['files']['vcf']['#value'];
    }
    if (!empty($fields['files']['vcf']['#default_value']) and ($file = file_load($fields['files']['vcf']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
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
      '#title' => 'CartograPlant Environmental Layers: *',
      '#collapsible' => TRUE,
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
          '#description' => "Please select the parameters you used from the $layer_title layer.",
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
    '#value' => 'bioproject'
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
      } catch (\Exception $e) {
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
    '#description' => 'Remember to click the "Upload" button below to send your file to the server.  This interface is capable of uploading very large files.  If you are disconnected you can return, reload the file and it will resume where it left off.  Once the file is uploaded the "Upload Progress" will indicate "Complete".  If the file is already present on the server then the status will quickly update to "Complete".',
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

  $fields['marker-type'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Marker Type (select all that apply): *'),
    '#options' => drupal_map_assoc(array(
      t('SNPs'),
      t('SSRs/cpSSRs'),
      t('Indels'),
      t('Other'),
    )),
  );

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
      0 => '- Select -',
      1 => 'GBS',
      2 => 'Targeted Capture',
      3 => 'Whole Genome Resequencing',
      4 => 'RNA-Seq',
      5 => 'Genotyping Array',
    ),
  );

  $fields['SNPs']['GBS'] = array(
    '#type' => 'select',
    '#title' => t('GBS Type: *'),
    '#options' => array(
      0 => '- Select -',
      1 => 'RADSeq',
      2 => 'ddRAD-Seq',
      3 => 'NextRAD',
      4 => 'RAPTURE',
      5 => 'Other',
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
      0 => '- Select -',
      1 => 'Exome Capture',
      2 => 'Other',
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
