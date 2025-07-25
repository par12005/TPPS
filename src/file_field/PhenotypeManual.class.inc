<?php

/**
 * @file
 * Contains 'PhenotypeManual' class.
 *
 * The main reason to have PhenotypeManual class is that each organism could have
 * multiple number of phenotypes so form fields must be processed in a
 * different way.
 */

/**
 * Manages fields to manually add phenotypes on TPPS Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PhenotypeManual {

  /**
   * Creates set of fields for single phenotype.
   *
   * Note: Each organism could have multiple number of phenotypes and
   * this number unique for each organism.
   *
   * @param int $id
   *   Organism Id.
   *
   * @return array
   *   Returns Drupal Form API Array with fields for single phenotype.
   */
  public function build($id) {
    $phenotype_cid = 'tpps_phenotype_field';
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    $cache = cache_get($phenotype_cid, $cache_bin);

    // Use cached data if possible.
    if (!empty($cache)) {
      $field = $cache->data;
      return $field;
    }

    // List of Phenotype synonyms. This list will be the same for all
    // phenotypes. but unit list is unique per phenotype and
    // will be obtained later because depends on selected synonym.
    $synonym_list = tpps_synonym_get_list();
    $attr_options = $this->getAttributeOptionList();
    $struct_options = $this->getStructureOptionList();
    $field = [
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#prefix' => "<div id=\"org_{$id}_phenotype_!num_meta\">",
      '#suffix' => "</div>",
      // [VS] Synonym form.
      'synonym_name' => $this->buildFieldName($id) + [
        '#prefix' => "<label><b>Phenotype !num:</b></label>",
        '#states' => [
          'visible' => [tpps_synonym_selector($id) => ['!value' => 0]],
        ],
      ],
      'synonym_description' => $this->buildFieldDescription() + [
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
      'name' => $this->buildFieldName($id) + [
        '#states' => [
          'visible' => [tpps_synonym_selector($id) => ['value' => 0]]
        ],
      ],
      'env-check' => [
        '#type' => 'checkbox',
        '#title' => 'Phenotype !num is an environmental phenotype',
        '#ajax' => [
          'callback' => 'tpps_update_phenotype_meta',
          'wrapper' => "org_{$id}_phenotype_!num_meta",
        ],
        '#states' => [
          'visible' => [tpps_synonym_selector($id) => ['value' => 0]],
        ],
      ],
      'attribute' => array(
        '#type' => 'select',
        '#title' => 'Phenotype !num Attribute: *',
        '#options' => $attr_options,
        '#ajax' => array(
          'callback' => 'tpps_update_phenotype_meta',
          'wrapper' => "org_{$id}_phenotype_!num_meta",
        ),
        '#states' => [
          'visible' => [tpps_synonym_selector($id) => ['value' => 0]],
        ],
      ),
      'attr-other' => [
        '#type' => 'textfield',
        '#title' => 'Phenotype !num Custom Attribute: *',
        '#autocomplete_path' => 'tpps/autocomplete/attribute',
        '#attributes' => [
          'data-toggle' => ['tooltip'],
          'data-placement' => ['right'],
          'title' => [
            t("If your attribute is not in the autocomplete list, don't "
            . "worry about it! We will create new phenotype metadata "
            . "in the database for you."),
          ],
        ],
        '#description' => t('Some examples of attributes include: "amount", '
          . '"width", "mass density", "area", "height", "age", "broken", '
          . '"time", "color", "composition", etc.'),
        '#states' => [
          'visible' => [
            ':input[name="' . $id . '[phenotype][phenotypes-meta][!num]'
              . '[attribute]"]' => ['value' => 'other'],
            tpps_synonym_selector($id) => ['value' => 0],
          ],
        ],
      ],
      'description' => $this->buildFieldDescription() + [
        '#states' => [
          'visible' => [tpps_synonym_selector($id) => ['value' => 0]],
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
      'struct-other' => [
        '#type' => 'textfield',
        '#title' => 'Phenotype !num Custom Structure: *',
        '#autocomplete_path' => 'tpps/autocomplete/structure',
        '#attributes' => [
          'data-toggle' => ['tooltip'],
          'data-placement' => ['right'],
          'title' => [
            t("If your structure is not in the autocomplete list, don't "
              . "worry about it! We will create new phenotype metadata "
              . "in the database for you."),
          ],
        ],
        '#description' => t('Some examples of structure descriptors include: '
          . '"stem", "bud", "leaf", "xylem", "whole plant", '
          . '"meristematic apical cell", etc.'),
        '#states' => [
          'visible' => [
            ':input[name="' . $id . '[phenotype][phenotypes-meta][!num]'
              . '[structure]"]' => ['value' => 'other'],
          ],
        ],
      ],
    ];

    // Extra tuning of the 'Phenotype N is an environmental phenotype' field.
    if (variable_get('tpps_page_4_env_check_always_visible', TRUE)) {
      unset($field['env-check']['#states']);
    }
    if (!variable_get('tpps_page_4_env_check_ajax_update', TRUE)) {
      unset($field['env-check']['#ajax']);
    }
    cache_set($phenotype_cid, $field, $cache_bin);
    return $field;
  }

  /**
   * Gets list of options for phenotype's 'Structure' field.
   *
   * Note:
   * When 'is phenotype environmental' checkbox checked and phenotype value is
   * 'I can't find my phenotype' this list will have only one item 'Whole Plant'.
   *
   * @return array
   *   Returns list of options.
   */
  public function getStructureOptionList() {
    $options = [];
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
      $id = tpps_load_cvterm($term)->cvterm_id;
      $options[$id] = $label;
    }
    $options['other'] = t('My structure term is not in this list');
    return $options;
  }

  /**
   * Gets list of options for phenotype's 'Attribute' field.
   *
   * @return array
   *   Returns list of options.
   */
  public function getAttributeOptionList() {
    $options = [0 => t('- Select -')];
    $terms = [
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
    ];
    foreach ($terms as $term => $label) {
      $id = tpps_load_cvterm($term)->cvterm_id;
      $options[$id] = $label;
    }
    $options['other'] = t('My attribute term is not in this list');
    return $options;
  }

  /**
   * Builds Phenotype Description form field.
   *
   * @return array
   *   Returns Form API field.
   */
  public function buildFieldDescription() {
    return [
      '#type' => 'textfield',
      '#title' => 'Phenotype !num Description: *',
      '#description' => t('Please provide a short description of Phenotype !num'),
    ];
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
  public function buildFieldName($id) {
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
   * Processes a file.
   *
   * Note: called from /forms/submit/submit_all.php script.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $shared_state
   *   Submission's Shared State.
   * @param array $options
   *   Necessary data.
   */
  public static function process($organism_index, array $shared_state, array &$options = []) {
  }

}
