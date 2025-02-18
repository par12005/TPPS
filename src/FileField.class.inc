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
   * Gets lit of required columns for validation.
   *
   * Note:
   * File class must overwrite this method of column validation is required.
   *
   * @return array
   *   Returns list of groups of required columns.
   *   See AssayDesign::getRequiredColumns() for the structure example.
   */
  public static function getRequiredColumns() {
    $message = t("Class '@class' must implement method 'getRequiredColumns'.",
      ['@class' => get_called_class()]
    );
    throw new Exception($message);
    return [];
  }

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
   * Optional fields do NOT need this check.
   * For required fields use form_get_error() to check if validation passed:
   *   self::validateRequired($organism_index, $form_state, $condition);
   *   if (form_get_errors()) {
   *     return;
   *   }
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State arra.
   * @param mixed $condition
   *   Condition when field is required.
   */
  public static function validateRequired($organism_index, array $form_state, $condition = FALSE) {
    if ($condition && empty(self::getValue($organism_index, $form_state))) {
      // Field is required but empty. Show error.
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'field is required.',
        [
          '@field_title' => self::getFieldTitle(),
          '@oidx' => $organism_index,
        ]
      );
      form_set_error(self::getFormErrorName($organism_index), $error_message);
    }
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
      // Example: 'Genotype_Assay_Design'.
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
    $fid = self::getFileId($organism_index, $form_state);
    if (empty($fid) || form_get_errors()) {
      return FALSE;
    }
    tpps_preserve_valid_file(
      $form_state,
      $fid,
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
      $species_fieldset = $page3_values['tree-accession'][$species_index];
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

  /**
   * Checks if field has columns (ability to set column's data type).
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   *
   * @return bool
   *   Returns TRUE if field has columns and FALSE otherwise.
   */
  public static function hasColumns($organism_index, array &$form_state) {

// @TODO Implement real check.

    return TRUE;

  }

  /**
   * Gets form element using list of parent elements.
   *
   * WARNING: Could be used ONLY for validation because uses currently
   * submitted form stored in $form_state['complete form'] element.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $parents
   *   List of parent elements including desired element in the end.
   *   When empty then list of parents will be built automatically using
   *   field's properties.
   *
   * @return mixed
   *   Returns form element or NULL if it wasn't found.
   */
  public static function getFormElement($organism_index, array &$form_state, array $parents = []) {
    if (empty($form_state['complete form'])) {
      $message = t("Form wasn't found under \$form_state['complete form'].");
      throw new Exception($message);
    }
    $element = NULL;
    $is_key_exist = NULL;
    if (empty($parents)) {
      // Build parents for current file field.
      $parents = [
        'organism-' . $organism_index,
        static::DATA_TYPE,
        static::FORM_PARENT_FIELDSET,
        static::FIELD_NAME,
      ];
    }
    $element = drupal_array_get_nested_value(
      $form_state['complete form'],
      $parents,
      $is_key_exist
    );
    if (!$is_key_exist) {
      throw new Exception(t("Form element wasn't found for field @field_name. "
        . "Field parents: @field_parents.",
        [
          '@field_name' => static::FIELD_NAME,
          '@field_parents' => implode(', ', $parents),
        ]
      ));
    }
    return $element;
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
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
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
   * @todo Minor. Replace tpps_file_validate_columns() with this code to allow
   * multiple groups of required columns.
   */
  public static function validateColumns($organism_index, array &$form_state, $silent_mode = FALSE) {
    $debug_mode = FALSE;

    if (!method_exists(static::class, 'getRequiredColumns')) {
      // It's not an error if columns not used for this file field.
      // Watchdog could be used for notices.
      return [];
    }
    $required_cases = static::getRequiredColumns();
    if ($debug_mode) {
      dpm(print_r($required_cases, 1), 'required_cases');
    }
    $file_element = self::getFormElement($organism_index, $form_state);
    // Check if file has zero length (empty).
    if (!($file_element['#file']->filesize ?? NULL)) {
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'File is empty.',
        [
          '@field_title' => self::getFieldTitle(),
          '@oidx' => $organism_index,
        ]
      );
      // We highlight an empty value field because file field not visible
      // when file was uploaded.
      // $file_element['#name'] . '[empty]',
      // There is no 'empty' element for file fields.
      form_set_error($file_element['#name'], $error_message);
      // No need to validate empty file.
      return [];
    }

    //Example of structure:
    //  $selected_columns => ['A' => '1', 'B' => '4',];
    $selected_columns = $file_element['#value']['columns'] ?? NULL;
    if (empty($selected_columns)) {
      // Note: $selected_columns will be empty when user just added file (not clicked 'Upload'
      // button and submitted form (clicked 'Next' button).
      // Probably columns could be detected automatically because they are shown
      // at page and so we could use default values here.
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . "Data types for columns wasn't specified.",
        [
          '@field_title' => t(self::getFieldTitle()),
          '@oidx' => $organism_index,
        ]
      );
      form_set_error(self::getFormErrorName($organism_index), $error_message);
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get some useful values.
    $parents = $file_element['#parents'];
    $new_end_columns = static::FIELD_NAME . "-columns";
    $new_end_no_header = static::FIELD_NAME . "-no-header";
    $new_end_empty = static::FIELD_NAME . "-empty";
    $group_path = static::FIELD_NAME . "-groups";
    // Remove last element from list of parents.
    array_pop($parents);
    // Note: all column related meta-data will be added to to upper element
    // (beside file field value). So to get value just add suffix to the path.
    $values = drupal_array_get_nested_value($form_state['values'], $parents);
    // Initialize form column values in form state.
    $values[$new_end_columns] = [];
    // Hold onto the location of the columns in form state.
    $state_column_values = &$values[$new_end_columns];
    $values[$new_end_no_header] = $file_element['#value']['no-header'] ?? NULL;
    $values[$new_end_empty] = $file_element['#value']['empty'] ?? NULL;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    $columns = [];
    foreach ($required_cases as $case_name => $case_data) {

      // Since file can contain extra columns we can't check if lists are equal
      // but must check if $selected_columns list includes all required columns.
      if (tppsArray::contains($case_data, $selected_columns)) {
        // Values are equal.
        $columns = $case_data;
        break;
      }
    }
    // Report about falied validation of the equired groups of the data types.
    if (empty($columns)) {
      foreach ($required_cases as $case_name => $case_data) {
        $required_print_list[] = implode(', ', static::getDataType($case_data)) . '.';
      }
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . "Not all required data types for columns wasn't set."
        . "<br />Selected data types are: <strong>!selected_columns</strong>"
        . "<br />Independent groups of the required data types are: <strong>!required_cases</strong>",
        [
          '@field_title' => t(self::getFieldTitle()),
          '@oidx' => $organism_index,
          '!selected_columns' => implode(', ', static::getDataType($selected_columns)),
          '!required_cases' => theme('item_list', ['items' => $required_print_list]),
        ]
      );
      form_set_error(self::getFormErrorName($organism_index), $error_message);
    }



    if ($debug_mode) {
      dpm(print_r($columns, 1), 'columns: ' . $case_name);
    }
    // See tpps_file_validate_columns().
    return array_flip($selected_columns);
  }

  /**
   * Gets list of column data types.
   *
   * @param mixed $id_list
   *   Data Type Id or list of ids.
   *
   * @return mixed
   *   Returns list of all localized data types.
   */
  public static function getDataType($id_list = NULL) {
    $list = static::getColumnOptions();
    $output = $list;
    if (!is_null($id_list)) {
      $output = [];
      if (!is_array($id_list)) {
        $id_list = [$id_list];
      }
      foreach ($id_list as $id) {
        $output[$id] = $list[$id] ?? NULL;
      }
    }
    return $output;
  }


  /**
   * Matches values in column with values in header/columns in other file.
   *
   * @param array $data
   *   Metadata for processing. Required keys are:
   *   'organism_index',
   *   'form_state',
   *   'target_form_element'
   *   'no_header'
   *   'use_target_header' bool
   *   'column_letter'
   *
   * @return bool
   *   Returns TRUE if values matches and FALSE otherwise.
   */
  public static function matchColumnValues(array $data) {
    $debug_mode = FALSE;

    // Target File.
    $target_title = trim(str_replace(': *', '', $data['target_form_element']['#title']));
    $target_fid = TppsArray::getValue(
      $data['form_state'],
      // Use 'input' instead of 'saved_values' to get latest value.
      array_merge(['input'], $data['target_form_element']['#parents'], ['fid'])
    );

    if ($data['use_target_header']) {
      // Match with values in header.
      $target_item_list = tpps_file_headers($target_fid);
      unset($target_item_list[key($target_item_list)]);
      if ($debug_mode) {
        dpm($item_list, 'Original file items');
        dpm($target_item_list, 'Target file items');
      }
    }
    else {
      // Match with values in column.
      // @TODO Implement getting column (by letter).
      // Use tpps_file_iterator()
    }

    // Original file.
    $item_list = tpps_parse_file_column(
      self::getFileId($data['organism_index'], $data['form_state']),
      // Column's letter.
      $data['column_letter'],
      $data['no_header']
    );

    // Validation.
    $missing_items = array_diff(
      $target_item_list,
      $item_list
    );
    // Report an error.
    if (!empty($missing_items)) {
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'We detected items that were not in your "<strong>@field_title</strong>" file. '
        . 'Please either remove these items from your "<strong>@target_field_title</strong>" file, or '
        . 'add them to your "<strong>@field_title</strong>" file. '
        . '<br />The missing items we found were: <strong>@list</strong>.',
        [
          '@field_title' => t(static::getFieldTitle()),
          '@target_field_title' => $target_title,
          '@oidx' => $data['organism_index'],
          '@list' => implode(', ', array_unique($missing_items)),
        ]
      );
      form_set_error(
        self::getFormErrorName($data['organism_index']),
        $error_message
      );
      return FALSE;
    }
    return TRUE;
  }

}
