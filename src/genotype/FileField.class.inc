<?php

/**
 * @file
 * Contains 'FileField' class.
 *
 * Validation / Build:
 *   getFileId() == getValue().
 * Processing / AJAX / Build:
 *   getSavedFileId() == getSavedValue().
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
  // File field title.
  // Example: 'SNP Assay'.
  const TITLE = '';
  // Name of the form field.
  // Example: 'snps-assay'.
  const FIELD_NAME = '';
  // Name of the parent fieldset on form (!) not shared form state.
  // Use in classes: self::FORM_PARENT_FIELDSET.
  // Use here: static::FORM_PARENT_FIELDSET.
  // Example: 'SNPs'.
  const FORM_PARENT_FIELDSET = '';
  // Name of the parent element in shared state array.
  // Note: Differs if field was relocated. See versions in function
  // tpps_submission_form_version_changes().
  // Example: 'files'.
  const SHARED_STATE_PARENT_FIELDSET = '';
  // Type of the data.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  // Example: 'genotype'.
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Value for processing.

  /**
   * Gets File Id stored in $state['saved_values'].
   *
   * To get value for validation use 'getFileId()' method.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $state
   *   Drupal Form API State array.
   *   process() works with $shared_state.
   *   Both $shared_state and $form_state could be passed.
   *
   * @return mixed
   *   Returns File Id or NULL.
   */
  public static function getSavedFileId($organism_index, array $state) {
    return self::getSavedValue($organism_index, $state);
  }

  /**
   * Gets value of the field stored in 'saved_values'.
   *
   * To get value for validation use 'getValue()' method.
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
  public static function getSavedValue($organism_index, array $state) {
    $element = $state['saved_values'][TPPS_PAGE_4]
      ['organism-' . $organism_index][static::DATA_TYPE] ?? NULL;
    $value = $element[static::SHARED_STATE_PARENT_FIELDSET][static::FIELD_NAME]
      // Used for validation when shared_state not yet created and $form_state
      // is used instead and field was relocated.
      ?? $element[static::FORM_PARENT_FIELDSET][static::FIELD_NAME]
      ?? NULL;
    return $value;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Value for validation.

  /**
   * Gets File Id for validation stored under $form_state['input'].
   *
   * To get value for processing use 'getSavedFileId()' method.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form_state
   *   Drupal Form API State array.
   *   validate() works with $form_state.
   *   process() works with $shared_state.
   *
   * @return mixed
   *   Returns File Id or NULL.
   */
  public static function getFileId($organism_index, array $form_state) {
    return self::getValue($organism_index, $form_state);
  }

  /**
   * Gets value of the field for validation stored in $form_state['input'].
   *
   * Note: $form_state['input'] holds values of the currently submitted form
   * and should be used for validation instead of values stored under
   * $form_state['saved_values'] which holds values already passed validation.
   *
   * To get value for processing use 'getSavedValue()' method.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form_state
   *   Drupal Form API State array.
   *   validate() works with $form_state.
   *   process() works with $shared_state.
   *
   * @return mixed
   *   Returns field's value or NULL.
   */
  public static function getValue($organism_index, array $form_state) {
    return drupal_array_get_nested_value($form_state, [
      'input',
      'organism-' . $organism_index,
      static::DATA_TYPE,
      static::FORM_PARENT_FIELDSET,
      static::FIELD_NAME,
      'fid',
    ]);
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

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
    if ($condition && empty(self::getValue($organism_index, $form_state))) {
      // Field is required but empty. Show error.
      $params = [
        '@field_title' => self::getFieldTitle(),
        '@organism_index' => $organism_index,
      ];
      $error_message = t('Organism #@organism_index / "<strong>@field_title</strong>": '
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
    if (static::FIELD_NAME == 'snps-assay') {
      // Note: field's title is 'SNP Assay' but prefix has extra 's' after 'SNP'.
      return 'Genotype_SNPs_Assay';
    }
    else {
      return ucfirst(static::DATA_TYPE) . '_' . str_replace(' ', '_', static::TITLE);
    }
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

  /**
   * Checks if all trees from accession file are present.
   *
   * When validation failes error message was created.
   *   To check if validation failed: if (form_get_errors()) { // Falied }.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function validateTrees($organism_index, array &$form_state) {
    if (form_get_errors()) {
      return;
    }
    if (empty($fid = self::getFileId($organism_index, $form_state))) {
      return;
    }

    $headers = tpps_file_headers($fid);
    $page3_values = $form_state['saved_values'][TPPS_PAGE_3];

    // Note: Form element 'existing_trees' is a checkbox with title:
    // 'These plants may have been studied in the past'.
    // 'If this box is checked, TPPS will try to find plants with matching
    // ids around the same location as the ones you are providing. If it finds
    // them successfully, it will mark them as the same plant in the database.
    if (empty($page3_values['existing_trees'])) {
      $species_index = 'species-1';
      if (empty($page3_values['tree-accession']['check'])) {
        $species_index = 'species-' . $organism_index;
      }
      $species_fieldset = page3_values['tree-accession'][$species_index];
      if (empty($species_fieldset['file'])) {
        // @TODO Show error message.
        return;
      }
      $missing_trees = tpps_compare_files(
        $fid,
        // Accession File id.
        $species_fieldset['file'],
        key($headers),
        // Get column which holds Tree Ids.
        $species_fieldset['file-groups']['Tree Id']['1'],
        self::getNoHeaderValue($organism_index, $form_state),
        // Note: Accession file uses non-standard ['file-no-header'] field name
        // instead of nested ['file']['no-header'].
        $species_fieldset['file-no-header']
      );
      if (!empty($missing_trees)) {
        $error_message = t('Organism #@oidx / "<strong>@field_title</strong>":'
          . 'We detected Plant Identifiers that were not in your Plant '
          . 'Accession file. Please either remove these plants from your '
          . 'Genotype file, or add them to your Plant Accession file. '
          . 'The Plant Identifiers we found were: @tree_id_str',
          [
            '@field_title' => t(self::getFieldTitle()),
            '@oidx' => $organism_index,
            '@tree_id_str' => implode(', ', $missing_trees),
          ]
        );
        form_set_error(self::getFormErrorName($organism_index), $error_message);
      }
    }
  }

  /**
   * Gets value of the 'No header' sub-field.
   *
   * 'No header' is a dropdown menu with 'Yes/No' options:
   *   0 => 'Yes',
   *   1 => 'No'.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   *
   * @return int
   *   Returns '0' when 'Yes' was selected and '1' when 'No' was selected.
   */
  public static function getNoHeaderValue($organism_index, array &$form_state) {
    // Example: organism-1[genotype][SNPs][snps-association][no-header].
    return $form_state['values']['organism-' . $organism_index][static::DATA_TYPE]
      [static::FORM_PARENT_FIELDSET][static::FIELD_NAME]['no-header'] ?? FALSE;
  }


}
