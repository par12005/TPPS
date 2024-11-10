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
  protected $name = '';

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
  protected $form;

  /**
   * Drupal Form API states array.
   *
   * @var array
   */
  protected $formState;

  /**
   * Form Field element.
   *
   * @var array
   */
  protected $formElement;

  /**
   * List of the parent element's names in $form.
   *
   * Note: list includes name of the field as latest element.
   * Used for validation to highlight field and to get submitted value for
   * validation from $formState array.
   *
   * @var array
   */
  protected $parents;

  /**
   * Other name of the field used for validation.
   *
   * @var string
   */
  protected $label;

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
  protected $isRequired;

  /**
   * Number of currently processed organism.
   *
   * Used to get organism name in validate() method.
   *
   * @var int
   */
  protected $organismNumber;

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
      $this->name = $last_item;
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
   * Gets $formElement property.
   *
   * @return mixed
   *   Returns file field form element.
   */
  public function getFormElement() {
    if (empty($this->formElement) && !empty($parents = $this->getParents())) {
      // Form State.
      if (!empty($form_state = $this->formState)) {
        $this->formElement = drupal_array_get_nested_value(
          $form_state['complete form'],
          $parents
        );
      }
      // Form.
      if (!empty($form = $this->form)) {
        $this->formElement = drupal_array_get_nested_value($form, $parents);
      }
    }
    return $this->formElement;
  }

  /**
   * Validates $formElement property and logs error using watchdog.
   */
  public function validateFormElement() {
    if ($this->isFormElementEmpty()) {
      watchdog('tpps', "Field wasn't found in 'complete form'. Path: @path",
        ['@path' => implode('/', $this->parents)], WATCHDOG_ERROR
      );
    }
  }

  /**
   * Checks if $formElement property is empty.
   *
   * @return bool
   *   Returns TRUE when property is empty and FALSE otherwise.
   */
  public function isFormElementEmpty() {
    return empty($this->getFormElement());
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
      $key_exists = NULL;
      $this->value = drupal_array_get_nested_value(
        $this->formState['values'], $this->getParents(), $key_exists
      );
      if ($key_exists === FALSE) {
        // @todo Throw an exception.
        $message = "Validation of field @field_name failed because field "
          . "wasn't found. Field parents: @field_parents.";
        $params = [
          '@field_name' => $this->name,
          '@field_parents_list' => implode(', ', $this->parents),
        ];
        watchdog('tpps', $message, $params, WATCHDOG_ERROR);
        drupal_set_message(t($message, $params), 'error');
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
    if (is_bool($condition)) {
      $this->isRequired = $condition;
    }
    return $this;
  }

  /**
   * Gets prefix for filename.
   *
   * Used in validate() method to build standardized filename.
   * See $form_state['file_info'] element.
   *
   * @return mixed
   *   Returns predefined string for existing fields and empty string for
   *   non-existing fields.
   */
  public function getFileNamePrefix() {
    switch ($this->name) {
      case 'snps-kinship':
        return 'SNPs_Kinship';

      case 'snps-pop-struct':
        return 'SNPs_Population_Structure';
    }
    return '';
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
    if ($this->isRequired) {
      $this->validateFormElement();
      if ($this->isEmpty()) {
        // Field is required but empty. Show error.
        tpps_form_error_required($this->formState, $this->getParents());
      }
      else {
        // Field is required and non-empty.
        // Preserve file if it is valid.
        tpps_preserve_valid_file(
          $this->formState,
          $this->getValue(),
          $this->getOrganismNumber(),
          $this->getFileNamePrefix()
        );
      }
    }
    return $this;
  }

}
