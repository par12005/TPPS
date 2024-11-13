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
          'SNP ID' => ['id' => [1]],
          'Scaffold' => ['scaffold' => [2]],
          'Position' => ['position' => [3]],
          'Allele' => ['allele' => [4]],
          'Associated Trait' => ['trait' => [5]],
          'Confidence Value' => ['confidence' => [6]],
        ],
      ],
      'snps-association' => [
        'prefix' => 'SNPs_Association',
        'required_groups' => [
          // Name => key => <id in the option's list>.
          'SNP ID' => ['id' => [1]],
          'Scaffold' => ['scaffold' => [2]],
          'Position' => ['position' => [3]],
          'Allele' => ['allele' => [4]],
          'Associated Trait' => ['trait' => [5]],
          'Confidence Value' => ['confidence' => [6]],
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
   *
   * @return object
   *   Returns current object to allow method chaining.
   */
  public function setFormState(array $value) {
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
   * Builds field name used in browser's DOM model.
   *
   * WARNING: no trailing ']'.
   * Ready for form_set_error().
   * See https://api.drupal.org/api/drupal/includes%21form.inc/function/form_set_error/7.x
   *
   * @return string
   *   Returns $this->parents separated with ']['.
   *   E.g., "organism-1[genotype][SNPs][snps-association][empty".
   */
  public function buildDomName() {
    if ($this->parents) {
      return implode('][', $this->parents);
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

dpm('validate ' . $this->name);
    // Field could became required depending on state of the other fields.
    if ($this->isRequired) {
      // Check if form had this field at all. We use data from the form to
      // get human readable field name automatically.
      if ($this->isFormElementEmpty()) {
        throw new Exception(t("Field wasn't found at form. Field's path: @path.",
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

          $field_dom_name = $this->buildDomName();
          $snps = $this->getFormParent();
          $id = $this->getOrganismNumber();

          // @TODO Check if value is correct.
          // $assoc_file = $this->getValue();
          $assoc_file = $snps['snps-association'] ?? 0;
          $snps_assay = $snps['snps-assay'] ?? 0;


          // Check if file has all of the required columns.
          $groups = tpps_file_validate_columns(
            $this->formState,
            $this->getMetaData('required_groups'),
            $this->getFormElement()
          );

          if (!form_get_errors()) {
            // Check that SNP IDs match Genotype Assay.
            $snps_id_col = $groups['SNP ID'][1];
            $assoc_no_header = $snps[$this->name . '-no-header'] ?? FALSE;

            $assay_snps = tpps_file_headers($snps_assay);
            unset($assay_snps[key($assay_snps)]);
            $assoc_snps = tpps_parse_file_column($assoc_file, $snps_id_col, $assoc_no_header);
            $missing_snps = array_diff($assoc_snps, $assay_snps);

            if ($missing_snps !== []) {
              form_set_error($field_dom_name, t('@title: We detected SNP IDs '
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
              $groups['Associated Trait'][5],
              $assoc_no_header
            );

            $phenotype = $this->formState['values'][$id]['phenotype'];
            $phenotype_meta = $phenotype['metadata'];
            $phenotype_number = $phenotype['phenotypes-meta']['number'];

            $phenotype_meta_names = [];
            $phenotype_name_col = $phenotype['metadata-groups']['Phenotype Id']['1'] ?? NULL;
            if (isset($phenotype_name_col)) {
              $phenotype_meta_names = tpps_parse_file_column($phenotype_meta, $phenotype_name_col);
            }

            for ($i = 1; $i <= $phenotype_number; $i++) {
              $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
            }

            $missing_phenotypes = array_diff(
              $association_phenotypes,
              $phenotype_meta_names
            );
            if ($missing_phenotypes !== []) {
              form_set_error($field_dom_name, t('@title: We detected '
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
              $groups['Position'][3],
              $assoc_no_header
            );
            foreach ($positions as $position) {
              if (!preg_match('/^(\d+):(\d+)$/', $position)) {
                form_set_error($field_dom_name, t('@title: We detected SNP '
                    . 'positions that do not match the required format. '
                    . 'The correct format is: "start:stop".',
                    ['@title' => $this->getTitle()]
                  )
                );
                break;
              }
            }
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
    // Remove colon ':' and required field label.
    return strtok($this->formElement['#title'], ':');
  }
}
