<?php

/**
 * @file
 * Contains 'TppsFile' class.
 */

/**
 * Contains common methods to manage files.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class TppsFile {

  /**
   * Gets parents of the field.
   *
   * WARNING: Must be overwritten by child class.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return array
   *   Returns list of parents.
   *
   * @throws Exception
   *   When child class doesn't overwrite this method.
   */
  public static function getParents($organism_index, array $form_state) {
    if (static::getClassName() == 'TppsFile') {
      $message = t('Class must implement @method', ['@method' => __METHOD__]);
      throw new Exception($message);
    }
    return [];
  }

  /**
   * Gets parents for checkbox which makes field required.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   *
   * @return array
   *   Returns list of paretns.
   *
   * @throws Exception
   *   When child class doesn't overwrite this method.
   */
  public static function getCheckboxParents($organism_index) {
    if (static::getClassName() == 'TppsFile') {
      $message = t('Class must implement @method', ['@method' => __METHOD__]);
      throw new Exception($message);
    }
    return [];
  }

  /**
   * Gets number of columns in the file.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return int
   *   Returns number of columns (width) of the file.
   */
  public static function countColumns($organism_index, array $form_state) {
    $fid = static::getFileId($organism_index, $form_state);
    return tpps_file_width($fid);
  }

  /**
   * Gets length of the file.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return int
   *   Returns length of the file.
   */
  public static function countRows($organism_index, array $form_state) {
    $fid = static::getFileId($organism_index, $form_state);
    return tpps_file_len($fid);
  }

  /**
   * Gets 1st row (header) of the file.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return int
   *   Returns 1st row (header) of the file.
   */
  public static function getHeader($organism_index, array $form_state) {
    if (empty($fid = static::getFileId($organism_index, $form_state))) {
      return NULL;
    }
    return tpps_file_get_header($fid);
  }

  /**
   * Checks if "Iso" checkbox was selected.
   *
   * Checkbox title:
   * "My phenotypes include results from a mass spectrometry or isotope analysis".
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns TRUE if checkbox was selected and FALSE otherwise.
   *   Returns NULL if field wasn't found.
   *
   * @todo merge with getFileId().
   * @todo Move to 'Submisison' class.
   */
  public static function isSelected($organism_index, array $form_state) {
    $parents = static::getCheckboxParents($organism_index);
    $key_exists = NULL;
    if (!empty($form_state['values'])) {
      $value = drupal_array_get_nested_value(
        $form_state['values'],
        $parents,
        $key_exists
      );
      if ($key_exists) {
        return $value;
      }
    }
    if (!empty($form_state['saved_values'][TPPS_PAGE_4])) {
      $value = drupal_array_get_nested_value(
        $form_state['saved_values'][TPPS_PAGE_4],
        $parents,
        $key_exists
      );
      if ($key_exists) {
        return $value;
      }
    }
    return NULL;
  }

  /**
   * Alias for getValue() method.
   */
  public static function getFileId($organism_index, array $form_state) {
    // @TODO Only for 'managed_file' type fields.
    return static::getValue($organism_index, $form_state);
  }

  /**
   * Gets file id.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns File Id or NULL if file wasn't uploaded.
   *
   * @todo Update to re-use for Page 4 fields.
   * @todo merge with isSelected().
   * @todo Move/combine with 'Submisison' class.
   */
  public static function getValue($organism_index, array $form_state) {
    $parents = static::getParents($organism_index);
    $key_exists = NULL;
    if (!empty($form_state['values'])) {
      $value = drupal_array_get_nested_value(
        $form_state['values'],
        $parents,
        $key_exists
      );
      if ($key_exists) {
        return $value;
      }
    }

    if (!empty($form_state['saved_values'][TPPS_PAGE_4])) {
      $value = drupal_array_get_nested_value(
        $form_state['saved_values'][TPPS_PAGE_4],
        $parents,
        $key_exists
      );
      if ($key_exists) {
        return $value;
      }
    }
    return NULL;
  }

  /**
   * Gets Class Name.
   *
   * Since 5.5 you can use class keyword for the class name resolution,
   * which would be a lot faster than making function calls.
   * Also works with interfaces.
   *
   * C extends B extends A
   *
   * static::class  // MyNamespace\ClassC when run in A
   * self::class    // MyNamespace\ClassA when run in A
   * parent::class  // MyNamespace\ClassB when run in C
   * MyClass::class // MyNamespace\MyClass
   *
   * See https://stackoverflow.com/questions/506705
   *
   * @return string
   *   Returns class name when called from static method.
   */
  public static function getClassName() {
    return static::class;
  }

}
