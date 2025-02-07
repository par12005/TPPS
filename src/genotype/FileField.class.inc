<?php

/**
 * @file
 * Contains 'FileField' class.
 */

/**
 * Common methods for file field classes.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
abstract class FileField {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Common settings.
  //
  // Name of the form field.
  const FIELD_NAME = '';
  // Name of the parent fieldset on form (!) not shared form state.
  // Use in classes: self::FORM_PARENT_FIELDSET.
  // Use here: static::FORM_PARENT_FIELDSET.
  const FORM_PARENT_FIELDSET = '';
  // Name of the parent element in shared state array.
  // Note: Differs if field was relocated. See versions in function
  // tpps_submission_form_version_changes().
  const SHARED_STATE_PARENT_FIELDSET = '';
  // Human readable field name in english (not localized).
  const FIELD_TITLE = '';
  // Type of the data.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  const DATA_TYPE = '';

  /**
   * Gets name of the field for form_set_error().
   *
   * WARNING: no trailing ']' but has first ']'.
   *
   * Difference:
   * getFormErrorName(): "organism-1][genotype][SNPs][snps-association".
   *                                ^                                 ^
   * getDomName(): "organism-1[genotype][SNPs][snps-association]".
   *                         ^                                 ^
   * See https://api.drupal.org/api/drupal/includes%21form.inc/function/form_set_error/7.x
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   * @param array $parents
   *   Optional. Path to field (including field name) from
   *   $form['organism-' . $organism_index][static::DATA_TYPE].
   *   When empty path to the current field will be used.
   *
   * @return string
   *   Returns field name in correct format for form_set_error().
   *   E.g., "organism-1][genotype][SNPs][snps-association][empty".
   */
  public static function getFormErrorName($organism_index, array $parents = []) {
    if (empty($parents)) {
      $parents = [static::FORM_PARENT_FIELDSET, static::FIELD_NAME];
    }
    $parents = array_merge(
      ['organism-' . $organism_index, static::DATA_TYPE],
      $parents
    );
    return implode('][', $parents);
  }

  /**
   * Gets DOM name of the field.
   *
   * Note: could be used for Drupal Form State API.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   * @param array $parents
   *   Optional. Path to field (including field name) from
   *   $form['organism-' . $organism_index][static::DATA_TYPE].
   *   When empty path to the current field will be used.
   *
   * @return string
   *   Returns field name. E.g., 'organism-1[genotype][SNPs][assay-design]'.
   */
  public static function getDomName($organism_index, array $parents = []) {
    if (empty($parents)) {
      $parents = [static::FORM_PARENT_FIELDSET, static::FIELD_NAME];
    }
    $parents = array_merge([static::DATA_TYPE], $parents);
    return 'organism-' . $organism_index . '[' . implode('][', $parents) . ']';
  }

  /**
   * Gets name of the field for Drupal Form State API.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   * @param array $parents
   *   Optional. Path to field (including field name) from
   *   $form['organism-' . $organism_index][static::DATA_TYPE].
   *   When empty path to the current field will be used.
   *
   * @return string
   *   Returns field name.
   */
  public static function getStateName($organism_index, array $parents = []) {
    return ':input[name="' . self::getDomName($organism_index, $parents) . '"]';
  }

  /**
   * Gets File Id.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $state
   *   Drupal Form API State array.
   *   validate() works with $form_state.
   *   process() works with $shared_state.
   *
   * @return mixed
   *   Returns File Id or NULL.
   */
  public static function getFileId($organism_index, array $state) {
    $element = $state['saved_values'][TPPS_PAGE_4]
      ['organism-' . $organism_index][static::DATA_TYPE] ?? NULL;
    $id = $element[static::SHARED_STATE_PARENT_FIELDSET][static::FIELD_NAME]
      // Used for validation when shared_state not yet created and $form_state
      // is used instead and field was relocated.
      ?? $element[static::FORM_PARENT_FIELDSET][static::FIELD_NAME]
      ?? NULL;
    return $id;
  }

  /**
   * Gets value of the field.
   *
   * Note: for file fields value is a File Id.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $state
   *   Drupal Form API State array.
   *   validate() works with $form_state.
   *   process() works with $shared_state.
   *
   * @return mixed
   *   Returns field's value or NULL.
   */
  public static function getValue($organism_index, array $state) {
    return self::getFileId($organism_index, $state);
  }

  /**
   * Checks if field's value is empty.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $state
   *   Drupal Form API State array.
   *   validate() works with $form_state.
   *   process() works with $shared_state.
   *
   * @return bool
   *   Returns TRUE if field's value is empty and FALSE otherwise.
   */
  public static function isEmptyValue($organism_index, array $state) {
    return empty(self::getValue($organism_index, $state));
  }

  /**
   * Checks if required field is not empty.
   *
   * Note: If field passed validtion TRUE will be returned and form_get_error()
   * will return empty result (if there wasn't any error message added before).
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State arra.
   * @param mixed $condition
   *   Condition when field is required.
   *
   * @return bool
   *   Returns TRUE if validation passed and FALSE otherwise.
   */
  public static function validateRequired($organism_index, array $form_state, $condition = FALSE) {
    if ($condition && self::isEmptyValue($organism_index, $form_state)) {
      // Field is required but empty. Show error.
      $params = [
        '@field_title' => static::FIELD_TITLE,
        '@organism_index' => $organism_index,
      ];
      $error_message = t('"@field_title" for organism @organism_index: '
        . 'field is required.', $params
      );
      form_set_error(self::getFormErrorName($organism_index), $error_message);
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Gets human readable field name (not localized).
   *
   * @return string
   *   Returns title for form field.
   */
  public static function getFieldTitle() {
    return static::TITLE . ' File';
  }

  /**
   * Gets prefix used for file when stored in $form_state['file_info'] list.
   *
   * E.g., 'Genotype_Assay_Design'.
   *
   * @return string
   *   Returns prefix.
   */
  public static function getFilePrefix() {
    return ucfirst(static::DATA_TYPE) . '_' . str_replace(' ', '_', static::TITLE);
  }

  /**
   * Preserves file if it is valid.
   *
   * 1. Registers file usage (when no one uses this file it could be removed
   *    but if usage is > 0 then file is protected).
   * 2. File will be added to the list of files in shared state array called
   *    'file_info' (and probably could be used for pipeline processing).
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form_state
   *   Drupal Form API State array.
   *
   * @return bool
   *   Returns TRUE in case of success and FALSE otherwise.
   */
  public static function preserveValidFile($organism_index, array &$form_state) {
    if (form_get_errors()) {
      return FALSE;
    }
    tpps_preserve_valid_file(
      $form_state,
      self::getFileId($organism_index, $form_state),
      $organism_index,
      self::getFilePrefix()
    );
    return TRUE;
  }

}
