<?php

/**
 * @file
 * Contains 'FileField' class.
 */

/**
 * Manages File fields for TPPS Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class FileField {

  /**
   * Machine name of the field.
   *
   * @var string
   */
  protected $name = NULL;

  /**
   * Submitted value of the field. Usually File Id.
   *
   * @var mixed
   */
  protected $value = NULL;

  /**
   * Drupal Form API array.
   *
   * @var array
   */
  protected $form = NULL;

  /**
   * Drupal Form API states array.
   *
   * @var array
   */
  protected $formState = NULL;

  /**
   * Form Field element.
   *
   * @var array
   */
  protected $formElement = NULL;

  /**
   * List of the parent element's names in $form.
   *
   * Note: list includes name of the field as latest element.
   * Used for validation to highlight field and to get submitted value for
   * validation from $formState array.
   *
   * @var array
   */
  protected $parents = NULL;

  /**
   * Flag which shows if field is required in currect submission.
   *
   * Note: Field could be set be required which was set using Drupal Form API
   * element '#required' and Drupal will validate this field but when field's
   * visibility depends on other fields it can't be used so we need manually
   * validate this field.
   *
   * @var bool
   */
  protected $isRequired = NULL;

  /**
   * Number of currently processed organism.
   *
   * Used to get organism name in validate() method.
   *
   * @var int
   */
  protected $organismNumber = NULL;

  /**
   * Data type which field belongs to.
   *
   * Posible values are: 'phenotype', 'genotype' and 'environmental_layer'.
   * Detected automatically by detectDataType() method from $this->name.
   *
   * @var string
   */
  protected $dataType = NULL;

  /**
   * List of specified column data types.
   *
   * See tpps_file_validate_columns(), FileField::validateColumns().
   *
   * @var array
   */
  protected $columns = NULL;

  /**
   * Flag which holds status of columns validation.
   *
   * See FileField::isColumnsValid() and FileField::validateColumns();
   *
   * @var bool
   */
  protected $isColumnsValid = NULL;
  /**
   * File Field's metadata.
   *
   * @var array
   *
   * @todo Move this list into separate file to avoid update of class's code.
   */
  private $metaData = [
    'phenotype' => [
      'metadata' => ['prefix' => 'Phenotype_Metadata'],
      'file' => ['prefix' => 'Phenotype_Data'],

    ],
    'genotype' => [
      'snps-kinship' => ['prefix' => 'SNPs_Kinship'],
      'snps-pop-struct' => ['prefix' => 'SNPs_Population_Structure'],
      'assay-design' => [
        'prefix' => 'Genotype_Assay_Design',
        'required_groups' => [
          // Name => key => <id in the option's list>.
          'case 1' => [
            'SNP ID' => ['id' => [1]],
            'Scaffold' => ['scaffold' => [2]],
            'Position' => ['position' => [3]],
          ],
          'case 2' => [
            'SNP ID' => ['id' => [1]],
            'Flank sequence' => ['sequence' => [4]],
          ],
        ],
      ],
      'snps-association' => [
        'prefix' => 'SNPs_Association',
        'required_groups' => [
          // Name => key => <id in the option's list>.
          // All columns are required.
          [
            'SNP ID' => ['id' => [1]],
            'Scaffold' => ['scaffold' => [2]],
            'Position' => ['position' => [3]],
            'Allele' => ['allele' => [4]],
            'Associated Trait' => ['trait' => [5]],
            'Confidence Value' => ['confidence' => [6]],
          ],
        ],
      ],
    ],
    'environmental_layers' => [],
  ];

  /**
   * Class constructor.
   *
   * @param array $parents
   *   List of parent elements including field name as last item.
   * @param array $form_state
   *   Drupal Form API State array..
   */
  public function __construct(array $parents = [], array $form_state = []) {
    $this->setParents($parents);
    if (!empty($form_state)) {
      $this->formState = $form_state;
    }
    return $this;
  }

  /**
   * Magic setter.
   *
   * Note: clonned from 'Submission' class.
   *
   * @param string $name
   *   Class property name.
   * @param mixed $value
   *   Value to be set.
   */
  public function __set($name, $value) {
    // Allow to overwrite magic with custom setter.
    $func = 'set' . ucfirst($name);
    if (method_exists($this, $func)) {
      $this->$func($value);
    }
    else {
      if (property_exists($this, $name)) {
        $this->$name = $value;
      }
      else {
        throw new InexistentPropertyException(
          t('Inexistent property: @name', ['@name' => $name])
        );
      }
    }
    return $this;
  }

  /**
   * Magic getter.
   *
   * Note: clonned from 'Submission' class.
   *
   * WARNING:
   *   Custom getters must be declared as reference.
   *   E.g., public function &getName() {}
   *
   * @param string $name
   *   Class property name.
   *
   * @return mixed
   *   Returns property value.
   */
  public function &__get($name) {
    // Allow to overwrite magic with custom getter.
    $func = 'get' . ucfirst($name);
    if (method_exists($this, $func)) {
      return $this->$func();
    }
    else {
      if (property_exists($this, $name)) {
        return $this->$name;
      }
      else {
        throw new Exception("Inexistent property: $name");
      }
    }
    return $this;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Custom setters.

  /**
   * Sets $parents property.
   *
   * Note: last element is field's name.
   *
   * @param array $parents
   *   List of field's parents including field name.
   *
   * @return object
   *   Returns current object to allow method chaining.
   */
  public function setParents(array $parents) {
    $last_item = end($parents);
    if (empty($this->name)) {
      // Name is empty. Use last item as a field name.
      $this->setName($last_item);
    }
    elseif ($last_item != $this->name) {
      // Short version of parents list was used.
      $parents[] = $this->name;
    }
    $this->parents = $parents;
    return $this;
  }

  /**
   * Sets '$name' property.
   *
   * @param string $value
   *   Field machine name.
   *
   * @return object
   *   Returns current object to allow method chaining.
   */
  public function setName(string $value) {
    $this->name = $value;
    $this->detectDataType();
    return $this;
  }

  /**
   * Sets '$formState' property.
   *
   * @param array $value
   *   Drupal Form API State array.
   *   Passed by reference so original array must be updated when 'formState'
   *   property updated.
   *
   * @return object
   *   Returns current object to allow method chaining.
   */
  public function setFormState(array &$value) {
    $this->formState = $value;
    return $this;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Custom getters.

  /**
   * Gets form element using list of parent elements.
   *
   * Form array could be obtained from $form class property which not required
   * and not always available and from $formState['complete form'] element
   * which also contains form array. Class property $formState widly used and
   * more chances to be non-empty (available).
   *
   * Could be used for debugging.
   *
   * @param array $parents
   *   List of parent elements including desired element in the end.
   *
   *   When $parents array is empty then $parents property will be used
   *   to get current field element and search result will be stored in
   *   $formElement property.
   *   When $parents argument is non-empty then $formElement class property
   *   won't be updated.
   *
   * @return mixed
   *   Returns element from $this->form found by $parents list.
   *   When $parents is empty then current field element will be returned.
   */
  public function getFormElement(array $parents = []) {
    $element = NULL;
    if (is_null($this->formElement) || !empty($parents)) {
      if ($update_form_element_property = empty($parents)) {
        $parents = $this->getParents();
      }
      if (empty($array = $this->form ?? $this->formState['complete form'])) {
        return FALSE;
      }
      $is_key_exist = NULL;
      $element = drupal_array_get_nested_value($array, $parents, $is_key_exist);
      if ($is_key_exist) {
        // Set only for current field (when method's argument wasn't used).
        if ($update_form_element_property) {
          $this->formElement = $element;
        }
      }
      else {
        throw new Exception(t("Form element wasn't found for field @field_name. "
          . "Field parents: @field_parents.",
          [
            '@field_name' => $this->name,
            '@field_parents' => implode(', ', $this->parents),
          ]
        ));
      }
    }
    return $element ?? $this->formElement ?? NULL;
  }

  /**
   * Get field's parent form element.
   *
   * @param array $parents
   *   List of parent elements including desired element in the end.
   *   When empty then $this->parents will be used to get current field parent
   *   element.
   *
   * @return mixed
   *   Returns parent element from $this->form found by $parents list.
   *   When $parents is empty then parent element for current field will be
   *   returned.
   */
  public function getFormParent(array $parents = []) {
    if (empty($parents)) {
      $parents = $this->getParents();
    }
    // Remove last element.
    array_pop($parents);
    return $this->getFormElement($parents);
  }

  /**
   * Builds field's name used only by form_set_error().
   *
   * WARNING: no trailing ']' and has first ']'.
   * To get full name see getDomElemntName() method.
   *
   * Difference:
   * getFormSetErrorName(): "organism-1][genotype][SNPs][snps-association".
   *                                   ^                                 ^
   * getDomElemntName(): "organism-1[genotype][SNPs][snps-association]".
   *                               ^                                 ^
   * See https://api.drupal.org/api/drupal/includes%21form.inc/function/form_set_error/7.x
   *
   * @return string
   *   Returns $this->parents separated with ']['.
   *   E.g., "organism-1[genotype][SNPs][snps-association][empty".
   */
  public function getFormSetErrorName() {
    if ($this->parents) {
      return implode('][', $this->parents);
    }
  }

  /**
   * Builds field's form name used in browser's DOM model in 'name' attribute.
   *
   * E.g., "organism-1[genotype][SNPs][snps-association][empty]".
   * Could be used in browser but not for $form or $form_state.
   *
   * WARNING: Can't be used in form_set_error().
   * See getFormSetErrorName() method.
   *
   * @return string
   *   Returns field's form name.
   */
  public function getDomElementName() {
    // Try '#name' element of the form because it's faster.
    // Note: property formElement not always set.
    if (!$this->isFormElementEmpty()) {
      if (!empty($this->formElement['#name'])) {
        return $this->formElement['#name'];
      }
    }
    // Build it from $parents property which always must be set.
    if ($this->parents) {
      reset($this->parents);
      $key = key($this->parents);
      $first = $this->parents[$key];
      return $first . '[' . implode('][', array_slice($this->parents, 1)) . ']';
    }
  }

  /**
   * Checks if $formElement property is empty.
   *
   * @return bool
   *   Returns TRUE when property is empty and FALSE otherwise.
   */
  public function isFormElementEmpty() {
    $this->getFormElement();
    return empty($this->formElement);
  }

  /**
   * Gets $parents property.
   *
   * @return mixed
   *   Returns list of form field parents.
   */
  public function getParents() {
    if (empty($this->parents) && !$this->isformElementEmpty) {
      // Not sure this code should be used here. Require extra testing.
      if (0) {
        // Code from tpps_get_field_path().
        $this->parents = $this->formElement['#parents'] ?? (
          // Why $this->parents is used here when it's empty?
          // See function tpps_get_field_path().
          $this->formElement['#tree'] ? $this->parents : $this->formElement['#name']
        ) ?? [];
      }
    }
    // Fix empty list.
    if (empty($this->parents)) {
      if (!empty($this->name)) {
        $this->parents = [$this->name];
      }
    }
    else {
      // Fix missing field name in the list.
      if (end($this->parents) != $this->name) {
        $this->parents[] = $this->name;
      }
    }

    return $this->parents;
  }

  /**
   * Gets Field's value.
   *
   * @return mixed
   *   Returns value of the field.
   */
  public function getValue() {
    if (
      !isset($this->value)
      && (!empty($this->parents) && !empty($this->formState))
    ) {
      $is_key_exist = NULL;
      $this->value = drupal_array_get_nested_value(
        $this->formState['values'], $this->getParents(), $is_key_exist
      );
      if ($is_key_exist === FALSE) {
        throw new Exception(t("Validation of field @field_name failed because "
          . "field wasn't found. Field parents: @field_parents.",
          [
            '@field_name' => $this->name,
            '@field_parents' => implode(', ', $this->parents),
          ]
        ));
      }
    }
    return $this->value;
  }

  /**
   * Checks if field's value is empty.
   *
   * @return bool
   *   Returns TRUE if value is empty and FALSE otherwise.
   */
  public function isEmpty() {
    // Note: if property $value wasn't yet set we could fill it with value from
    // $formState using getValue() instead of calling $this->value directly.
    return empty($this->getValue());
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Checks if field required using provided conditions and stores result.
   *
   * @param bool $condition
   *   Conditions when field is required.
   *
   * @return object
   *   Returns current object to allow method chaining.
   */
  public function isRequired($condition = '') {
    $this->isRequired = (bool) $condition;
    return $this;
  }

  /**
   * Gets meta data.
   *
   * Used in validate() method to build standardized filename.
   * See $form_state['file_info'] element.
   *
   * @param string $item_name
   *   Name of the item to find in $metaData property.
   *   Possible values are: 'prefix', 'required_groups'.
   *
   * @return mixed
   *   Returns predefined string for existing fields and empty string for
   *   non-existing fields.
   */
  public function getMetaData(string $item_name) {
    $key_exists = NULL;
    $item_value = drupal_array_get_nested_value(
      $this->metaData,
      [$this->dataType, $this->name, $item_name],
      $key_exists
    );
    if ($key_exists) {
      return $item_value;
    }
    else {
      // Field wasn't yet added to $this->metaData list.
      throw new Exception(t('Update list "@class::metaData" to get '
        . ' @item_name for the field "@field_name" of "@data_type".',
        [
          '@item_name' => $item_name,
          '@field_name' => $this->name,
          '@data_type' => $this->dataType,
          '@class' => get_class($this),
        ]
      ));
    }
  }

  /**
   * Detects DataType by field name.
   *
   * Field could have very common name like 'file' and multiple fields with
   * the same name could be used on different TPPS Form pages. To avoid
   * conflicts we use data types to make them more unique.
   * Note:
   * $this->parents couldn't be used to get filename prefixes using
   * getMetaData() method because parents contains very specific
   * $organismNumber.
   */
  public function detectDataType() {
    foreach ($this->metaData as $data_type => $field_list) {
      if (in_array($this->name, array_keys($field_list))) {
        $this->dataType = $data_type;
        return;
      }
    }
    // When not found it means class was used for field which wasn't yet added.
    throw new Exception(t('Update list "@class::metaData" to detect '
      . 'data type of the field "@field_name".',
      ['@field_name' => $this->name, '@class' => get_class($this)]
    ));
  }

  /**
   * Sets $organismNumber property.
   *
   * @param int $value
   *   Ordinal number of organism currently processing.
   *
   * @return object
   *   Returns current object to allow method chaining.
   */
  public function setOrganismNumber(int $value) {
    $this->organismNumber = $value;
    return $this;
  }

  /**
   * Gets $organismNumber property.
   *
   * @return int
   *   Ordinal number of organism currently processing.
   */
  public function getOrganismNumber() {
    if (empty($this->organismNumber)) {
      $first_item = reset($this->getParents());
      $key = 'organism-';
      if (strpos($first_item, $key) === 0) {
        // Get number.
        $this->organismNumber = substr($first_item, strlen($key));
      }
    }
    return $this->organismNumber ?? 1;
  }

  /**
   * Process file if its required and non-empty.
   *
   * @return object
   *   Returns current object to allow method chaining.
   */
  public function validate() {

// @TODO Remove debug code.
if ($this->name == 'assay-design') {

}

dpm('Validate: ' . $this->name);

    // Field could became required depending on state of the other fields.
    if ($this->isRequired) {
      // Check if form had this field at all. We use data from the form to
      // get human readable field name automatically.
      if ($this->isFormElementEmpty()) {
        throw new Eeption(t("Field wasn't found at form. Field's path: @path.",
          ['@path' => implode('/', $this->parents)]
        ));
      }
      // First check if required field has value.
      if ($this->isEmpty()) {
        // Field is required but empty. Show error.
        tpps_form_error_required($this->formState, $this->getParents());
      }
      else {
        // Field is required and non-empty.
        if ($this->isColumns()) {

// @TODO Remove debug code.
dpm('Validate columns');

          if ($this->name == 'snps-association') {
            $error_name = $this->getFormSetErrorName();
            $snps = $this->getFormParent();

            // @TODO Check if value is correct.
            // $assoc_file = $this->getValue();
            $assoc_file = $snps['snps-association'] ?? 0;
            $snps_assay = $snps['snps-assay'] ?? 0;

            // Check if file has all of the required columns.
            $this->validateColumns();
            if (!$this->isColumnsValid()) {
              $file_element = $this->getFormElement();
dpm($file_element['#name'], 'file field #name');

// @todo Get real value for $group.

              $group = 'sdfsdf';
              // Get human readable field name.
              form_set_error(
                $file_element['#name'] . "[columns][$group",
                t('@field_title: Please specify a column or columns that hold @group.',
                  [
                    '@field_title' => $this->getTitle(),
                    '@group' => $group,
                  ]
                )
              );
            }

            if (!form_get_errors()) {
              // Check that SNP IDs match Genotype Assay.
              $snps_id_col = $this->columns['SNP ID'][1];
              $assoc_no_header = $snps[$this->name . '-no-header'] ?? FALSE;

              $assay_snps = tpps_file_headers($snps_assay);
              unset($assay_snps[key($assay_snps)]);
              $assoc_snps = tpps_parse_file_column($assoc_file, $snps_id_col, $assoc_no_header);
              $missing_snps = array_diff($assoc_snps, $assay_snps);

              if ($missing_snps !== []) {
                form_set_error($error_name, t('@title: We detected SNP IDs '
                    . 'that were not in your Genotype Assay. Please either '
                    . 'remove these SNPs from your Association file, or add '
                    . 'them to your Genotype Assay. The SNP Identifiers we '
                    . 'found were: @snps_id_list.',
                    [
                      '@snps_id_list' => implode(', ', array_unique($missing_snps)),
                      '@title' => $this->getTitle(),
                    ]
                  )
                );
              }

              // Check that Phenotype names match phenotype metadata section.
              $association_phenotypes = tpps_parse_file_column(
                $assoc_file,
                $this->columns['Associated Trait'][5],
                $assoc_no_header
              );

              $phenotype = $this->formState['values'][$this->getOrganismNumber()]['phenotype'];
              $phenotype_meta = $phenotype['metadata'];
              $phenotype_number = $phenotype['phenotypes-meta']['number'];

              $phenotype_meta_names = [];
              $phenotype_name_col = $phenotype['metadata-groups']['Phenotype Id']['1'] ?? NULL;
              if (isset($phenotype_name_col)) {
                $phenotype_meta_names = tpps_parse_file_column(
                  $phenotype_meta,
                  $phenotype_name_col
                );
              }

              for ($i = 1; $i <= $phenotype_number; $i++) {
                $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
              }

              $missing_phenotypes = array_diff(
                $association_phenotypes,
                $phenotype_meta_names
              );
              if ($missing_phenotypes !== []) {
                form_set_error($error_name, t('@title: We detected '
                  . 'Associated Traits that were not specified in the '
                  . 'Phenotype Metadata Section. Please either remove '
                  . 'these Traits from your Association file, or add them '
                  . 'to your Phenotype Metadata section. '
                  . 'The Trait names we foud were: @phenotype_names',
                    [
                      '@phenotype_names' => implode(', ',
                        array_unique($missing_phenotypes)),
                      '@title' => $this->getTitle(),
                    ]
                  )
                );
              }

              // Check that position values are correctly formatted.
              $positions = tpps_parse_file_column(
                $assoc_file,
                $this->columns['Position'][3],
                $assoc_no_header
              );
              foreach ($positions as $position) {
                if (!preg_match('/^(\d+):(\d+)$/', $position)) {
                  form_set_error($error_name, t('@title: We detected SNP '
                      . 'positions that do not match the required format. '
                      . 'The correct format is: "start:stop".',
                      ['@title' => $this->getTitle()]
                    )
                  );
                  break;
                }
              }
            }
            // End of the 'snps-association' field.

          }
          elseif ($this->name == 'assay-design') {
            $error_name = $this->getFormSetErrorName();
            $snps = $this->getFormParent();

            // @TODO Check if value is correct.
            // $assoc_file = $this->getValue();
            $assoc_file = $snps['snps-association'] ?? 0;
            $snps_assay = $snps['snps-assay'] ?? 0;



            $this->validateColumns($silent_mode = TRUE);


            if (!form_get_errors()) {
              // Check that SNP IDs match Genotype Assay.
              $snps_id_col = $this->columns['SNP ID'][1];
              $assoc_no_header = $snps[$this->name . '-no-header'] ?? FALSE;

              $assay_snps = tpps_file_headers($snps_assay);
              unset($assay_snps[key($assay_snps)]);
              $assoc_snps = tpps_parse_file_column($assoc_file, $snps_id_col, $assoc_no_header);
              $missing_snps = array_diff($assoc_snps, $assay_snps);

              if ($missing_snps !== []) {
                form_set_error($error_name, t('@title: We detected SNP IDs '
                    . 'that were not in your Genotype Assay. Please either '
                    . 'remove these SNPs from your Association file, or add '
                    . 'them to your Genotype Assay. The SNP Identifiers we '
                    . 'found were: @snps_id_list.',
                    [
                      '@snps_id_list' => implode(', ', array_unique($missing_snps)),
                      '@title' => $this->getTitle(),
                    ]
                  )
                );
              }

              // Check that Phenotype names match phenotype metadata section.
              $association_phenotypes = tpps_parse_file_column(
                $assoc_file,
                $this->columns['Associated Trait'][5],
                $assoc_no_header
              );

              $phenotype = $this->formState['values'][$this->getOrganismNumber()]['phenotype'];
              $phenotype_meta = $phenotype['metadata'];
              $phenotype_number = $phenotype['phenotypes-meta']['number'];

              $phenotype_meta_names = [];
              $phenotype_name_col = $phenotype['metadata-groups']['Phenotype Id']['1'] ?? NULL;
              if (isset($phenotype_name_col)) {
                $phenotype_meta_names = tpps_parse_file_column(
                  $phenotype_meta,
                  $phenotype_name_col
                );
              }

              for ($i = 1; $i <= $phenotype_number; $i++) {
                $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
              }

              $missing_phenotypes = array_diff(
                $association_phenotypes,
                $phenotype_meta_names
              );
              if ($missing_phenotypes !== []) {
                form_set_error($error_name, t('@title: We detected '
                  . 'Associated Traits that were not specified in the '
                  . 'Phenotype Metadata Section. Please either remove '
                  . 'these Traits from your Association file, or add them '
                  . 'to your Phenotype Metadata section. '
                  . 'The Trait names we foud were: @phenotype_names',
                    [
                      '@phenotype_names' => implode(', ',
                        array_unique($missing_phenotypes)),
                      '@title' => $this->getTitle(),
                    ]
                  )
                );
              }

              // Check that position values are correctly formatted.
              $positions = tpps_parse_file_column(
                $assoc_file,
                $this->columns['Position'][3],
                $assoc_no_header
              );
              foreach ($positions as $position) {
                if (!preg_match('/^(\d+):(\d+)$/', $position)) {
                  form_set_error($error_name, t('@title: We detected SNP '
                      . 'positions that do not match the required format. '
                      . 'The correct format is: "start:stop".',
                      ['@title' => $this->getTitle()]
                    )
                  );
                  break;
                }
              }
            }

            // End of the 'snps-association' field.
          }
        }
        // Preserve file if it is valid.
        tpps_preserve_valid_file(
          $this->formState,
          $this->getValue(),
          $this->getOrganismNumber(),
          $this->getMetaData('prefix')
        );
      }

    }
    return $this;
  }

  /**
   * Searches $this->formState for the element using list of parent elements.
   *
   * Could be used for debugging.
   *
   * @param array $parents
   *   List of parent elements including desired element in the end.
   *   When empty $this->parents will be used to get current field element.
   *
   * @return mixed
   *   Returns element from $this->formState found by $parents list.
   *
   * @todo Actually it's getting submitted value of the field.
   * See tpps_get_ajax_value($form_state, $marker_parents, []);
   */
  public function findFormState(array $parents = []) {
    $array = $this->formState;
    if (empty($array)) {
      return FALSE;
    }
    if (empty($parents)) {
      $parents = $this->getParents();
    }
    $is_key_exists = NULL;
    // Currently submitted values.
    $element = drupal_array_get_nested_value($array['input'], $parents, $is_key_exists);
    if ($is_key_exists === FALSE) {
      // If element wasn't found try to search previously submitted steps/pages.
      $element = drupal_array_get_nested_value($array['saved_values'], $parents);
    }
    return $element;
  }

  /**
   * Checks if field uses columns feature to set column's data types.
   *
   * @return bool
   *   Returns TRUE if form element non-empty and has 'columns' sub-element
   *   Returns FALSE otherwise.
   *
   * @todo Rename method to something more meaningful.
   */
  public function isColumns() {
    if ($this->isFormElementEmpty()) {
      return FALSE;
    }
    // Field's value in $this->formState has sub-element 'columns-options'
    // but seems it's less safe way to detect if this feature is in use.
    return array_key_exists('columns', $this->formElement);
  }

  /**
   * Gets field's human readable title.
   *
   * @return string
   *   Returns field's title.
   */
  public function getTitle() {
    $this->getFormElement();
    // Remove colon (':'), remove required field mark ('*').
    return strtok($this->formElement['#title'], ':');
  }

  /**
   * This function validates that the file contains all of the required groups.
   *
   * Based on code from function tpps_file_validate_columns().
   *
   * A "required group" is a column type that must be present within the file for
   * the file to be considered valid. For instance, a required group for a plant
   * accession file would be the "Location" group. The required groups array
   * should satisfy the structure:
   *  array(
   *    '<group name>' => array(
   *      '<type>' => array(<column option index>[, <column option index>...])[,
   *      '<type>' => array(<column option index>[, <column option index>...])...]
   *    )[,
   *    '<group name>' => array(
   *      '<type>' => array(<column option index>[, <column option index>...])[,
   *      '<type>' => array(<column option index>[, <column option index>...])...]
   *    )...]
   *  )
   * where <group name> is the name of the required group, <type> is the name of
   * the set of options that could satisfy the required group, and each of the
   * <column option index>s is a key within the file element['column-options']
   * field.
   *
   * @param bool $silent_mode
   *   Default is FALSE.
   *   When FALSE and silent mode is disabled then form_set_error() will be called
   *   which means error message on page will appear and form validation fails.
   *   When TRUE and silent mode is enabled then no error message will be shown
   *   and form will be left valid. This could be useful when not all columns are
   *   required and multiple groups or columns must be checked.
   *   For example:
   *   case 1: Required columns: A, B, C;
   *   case 2: Required columns: A, D, E, F.
   *
   * @return array
   *   The groups and the column option types that the file was valid for.
   *   Example:
   *   [
   *     // <Name> => [
   *     //   <id in the option's list> => <column index>,
   *     //   '#type' => <key>
   *     // ],
   *     'SNP ID' => [1 => 'B', '#type' => 'id'],
   *     'Scaffold' => [
   *       '#type' => [], // < ---- Not found!
   *     ],
   *     'Position' => [3 => 'D', '#type' => 'position'],
   *     'Allele' => [4 => 'E', '#type' => 'allele'],
   *     'Associated Trait' => [5 => 'F', '#type' => 'trait'],
   *     'Confidence Value' => [6 => 'G', '#type' => 'confidence'],
   *   ];
   *
   *
   * @todo Remove arguments.
   * 2. Make silent mode default (TRUE).
   * 3. Return boolean value.
   * @todo replace tpps_file_validate_columns().
   */
  public function validateColumns($silent_mode = FALSE) {

    $required_groups = $this->getMetaData('required_groups');
    $file_element = $this->getFormElement();
    $error_prompt = $this->getTitle();
    // Check if file has zero length (empty).
    if (!($file_element['#file']->filesize ?? NULL)) {
      form_set_error(
        // We highlight an empty value field because file field not visible
        // when file was uploaded.
        //$file_element['#name'] . '[empty]',
        // There is no 'empty' element for file fields.
        $file_element['#name'],
        $error_prompt . ': File is empty.'
      );
      // No need to validate empty file.
      return [];
    }

dpm($file_element['#name'], 'file field #name');
dpm($this->getFormSetErrorName());
dpm($this->getDomElementName());

    $cols = $file_element['#value']['columns'] ?? NULL;
    $parents = $file_element['#parents'];
    $new_end_columns = end($parents) . "-columns";
    $new_end_no_header = end($parents) . "-no-header";
    $new_end_empty = end($parents) . "-empty";
    $group_path = array_pop($parents) . "-groups";
    $values = drupal_array_get_nested_value($this->formState['values'], $parents);
    // Initialize form column values in form state.
    $values[$new_end_columns] = [];
    // Hold onto the location of the columns in form state.
    $state_column_values = &$values[$new_end_columns];
    $values[$new_end_no_header] = $file_element['#value']['no-header'] ?? NULL;
    $values[$new_end_empty] = $file_element['#value']['empty'] ?? NULL;

    $this->columns = [];

    foreach ($required_groups as $case) {
      $required_groups_flat = [];
      foreach ($case as $group => $combinations) {
        $this->columns[$group] = [];
        $required_groups_flat[$group] = [];
        foreach ($combinations as $name => $combination) {
          $required_groups_flat[$group] = array_merge(
            $required_groups_flat[$group],
            $combination
          );
        }
      }

dpm($required_groups_flat);

      if ($cols) {
        // Note: $cols will be empty when user just added file (not clicked 'Upload'
        // button and submitted form (clicked 'Next' button).
        // Probably columns could be detected automatically because they are shown
        // at page and so we could use default values here.
        // @TODO Get default set of columns.
        foreach ($cols as $name => $type) {
          $state_column_values[$name] = $type;
          foreach ($required_groups_flat as $group => $types) {
            if (in_array($type, $types)) {
              if (!isset($this->columns[$group][$type])) {
                $this->columns[$group][$type] = [$name];
              }
              else {
                $this->columns[$group][$type][] = $name;
              }
              break;
            }
          }
        }
      }

      foreach ($case as $group => $combinations) {
        $group_valid = FALSE;
        $this->columns[$group]['#type'] = [];
        foreach ($combinations as $name => $combination) {
          $combination_valid = TRUE;
          foreach ($combination as $type) {
            if (!isset($groups[$group][$type])) {
              $combination_valid = FALSE;
              break;
            }
          }
          if ($combination_valid) {
            $this->columns[$group]['#type'][] = $name;
            $group_valid = TRUE;
          }
        }

        if (!$group_valid && !$silent_mode) {
          form_set_error(
            $file_element['#name'] . "[columns][$group",
            t('@field_name: Please specify a column or columns that hold @group.',
              [
                '@field_name' => $error_prompt,
                '@group' => $group,
              ]
            )
          );
        }
      }

      // Replaces list of column names with a single value when was found only one. Instead of this:
      // [
      //   'Flank sequence' => [
      //     '4' => ['0' => 'B'],
      //     '#type' => ['0' => 'sequence'],
      //   ]
      // ];
      // we will get this:
      // [
      //   'Flank sequence' => ['4' => 'B', '#type' => 'sequence'],
      // ];
      foreach ($this->columns as $key => $group) {
        foreach ($group as $opt_num => $col_names) {
          if (count($col_names) == 1) {
            $this->columns[$key][$opt_num] = $col_names[0];
          }
        }
      }

      $values[$group_path] = $this->columns;
      drupal_array_set_nested_value($this->formState['values'], $parents, $values);

      // @todo Check if original $form_state array was updated.

    }


    dpm(print_r($this->columns, 1));

    // @todo Return result of validation or rename method.
    $result = TRUE;

    return $result;
  }

  /**
   * Checks if data types in colums are valid.
   *
   * @return bool
   *   Returns TRUE when all required columns has correct data types.
   *   Returns FALSe otherwise.
   */
  public function isColumnsValid() {
    if (is_null($this->isColumnsValid) && is_null($this->columns)) {
      $this->validateColumns();
    }
    return $this->isColumnsValid;
  }

}
