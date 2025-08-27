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
  // File field title.
  // Example: 'SNP Assay'.
  const TITLE = '';
  // Name of the form field.
  // Example: 'snps-assay'.
  const FIELD_NAME = '';
  // Name of the parent fieldset on form (!) not shared form state.
  // Use in classes: static::FORM_PARENT_FIELDSET.
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
  // Form step where field is used.
  const FORM_STEP = '';

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
  abstract public static function getRequiredColumns();

  /**
   * Gets parents of the field.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return array
   *   Returns parents for the checkbox which makes this field required.
   */
  abstract public static function getParents($organism_index, array $form_state);

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
   *   $form[$organism_key][static::DATA_TYPE].
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
    $organism_key = 'organism-' . $organism_index;
    $parents = array_merge([$organism_key, static::DATA_TYPE], $parents);
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
   *   $form[$organism_key][static::DATA_TYPE].
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
    $organism_key = 'organism-' . $organism_index;
    return $organism_key . '[' . implode('][', $parents) . ']';
  }

  /**
   * Gets name of the field for Drupal Form State API.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   * @param array $parents
   *   Optional. Path to field (including field name) from
   *   $form[$organism_key][static::DATA_TYPE].
   *   When empty path to the current field will be used.
   *
   * @return string
   *   Returns field name.
   */
  public static function getStateName($organism_index, array $parents = []) {
    return ':input[name="' . static::getDomName($organism_index, $parents) . '"]';
  }

  /**
   * Gets File Id for validation stored under $form_state['input'].
   *
   * Note: First tries to get value for validation ('input' sub-element),
   * then for processing ('saved_values') using State and Shared State.
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
    return static::getValue($organism_index, $form_state);
  }

  /**
   * Gets value of the field for validation stored in $form_state['input'].
   *
   * Note: $form_state['input'] holds values of the currently submitted form
   * and should be used for validation instead of values stored under
   * $form_state['saved_values'] which holds values already passed validation.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form_state
   *   Drupal Form API State array.
   *   validate() works with $form_state.
   *   process() works with $shared_state.
   *
   * @return mixed
   *   Returns integer File Id.
   *   Returns NULL if field wasn't found.
   *   Returns string '0' if field wasn't set (file wasn't uploaded).
   *
   * @todo Minor. Detect validation/processing and if $form_state contains
   * State or Shared State.
   */
  public static function getValue($organism_index, array $form_state) {
    // For validation.
    $value = $key_exists = NULL;
    // Note: List of parents depends from current version of $form_state.
    $parents = static::getParents($organism_index, $form_state);

    // To always get most recent and up-to-date value we check first recenlty
    // submitted data stored in $form_state['input'] instead of using value
    // stored under $form_state['saved_values'].
    //
    // Any class which extends 'FileField' is a file field so we could get file
    // id for validation (when form not yet submitted) from $form_state['input']
    // using 'fid' hidden sub-element of 'managed_file' element.
    $input_parents = array_merge($parents, ['fid']);
    $value = TppsArray::getValue($form_state['input'], $input_parents, $key_exists);
    if ($key_exists === FALSE) {
      $value = TppsArray::getValue($form_state['saved_values'], $parents, $key_exists);
    }
    // Nothing was found.
    // It's not an error and there is no sence to throw an exception.
    if ($key_exists === FALSE) {
      $message = 'Organism #@oidx / @field_title: Value not found.';
      $params = [
          '@field_title' => static::getFieldTitle(),
          '@oidx' => $organism_index,
        ];
      watchdog('tpps', $message, $params, WATCHDOG_WARNING);
    }
    return $value;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Checks if required field is not empty.
   *
   * Optional fields do NOT need this check.
   * For required fields use form_get_error() to check if validation passed:
   *   static::validateRequired($organism_index, $form_state, $condition);
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
    if ($condition && empty(static::getValue($organism_index, $form_state))) {
      // Field is required but empty. Show error.
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'field is required.',
        [
          '@field_title' => static::getFieldTitle(),
          '@oidx' => $organism_index,
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
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
    $fid = static::getFileId($organism_index, $form_state);
    if (empty($fid) || form_get_errors()) {
      return FALSE;
    }
    tpps_preserve_valid_file(
      $form_state,
      $fid,
      $organism_index,
      static::getFilePrefix()
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
    if (empty($fid = static::getFileId($organism_index, $form_state))) {
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
        static::getSubFieldValue($organism_index, $form_state, 'no-header'),
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
            '@field_title' => t(static::getFieldTitle()),
            '@oidx' => $organism_index,
            '@tree_id_str' => implode(', ', $missing_trees),
          ]
        );
        form_set_error(static::getFormErrorName($organism_index), $error_message);
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
   *   Testing:
   *   $s = new Submission('TGDR1033');
   *   $organism_index = 1;
   *   dpm(SnpAssociation::getSubFieldValue(
   *     $organism_index,
   *     $s->getState(),
   *     'no-header'
   *   ));
   *
   * See FileField::getValue() and TppsArray::getValue().
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   * @param string $name
   *   Sub-field machine name. Possible values are:
   *   'no-header', 'columns', 'empty'.
   *
   * @return mixed
   *   Returns '0' when 'Yes' was selected and '1' when 'No' was selected.
   *   Returns NULL when value wasn't found.
   *
   * @todo Minor. Detect validation/processing and if $form_state contains
   * State or Shared State.
   */
  public static function getSubFieldValue($organism_index, array &$form_state, $name) {
    $value = $key_exists = NULL;
    $organism_key = 'organism-' . $organism_index;
    $parent_list = [
      // For validation.
      // Note: 'saved_values' doesn't have value until validation passed. Why?
      // Because 'no-header' is a part of complex file field.
      'input' => [
        'input',
        $organism_key,
        static::DATA_TYPE,
        static::FORM_PARENT_FIELDSET,
        static::FIELD_NAME,
        $name,
      ],
      // @todo MInor. Probably it's useless step and could be removed.
      'values' => [
        'values',
        $organism_key,
        static::DATA_TYPE,
        static::FORM_PARENT_FIELDSET,
        static::FIELD_NAME,
        $name,
      ],
      // Processing.
      // Use 'saved_values'
      // Note: structure differs from 'input'.
      $parents = [
        'saved_values',
        static::FORM_STEP,
        $organism_key,
        static::DATA_TYPE,
        static::FORM_PARENT_FIELDSET,
        static::FIELD_NAME . '-' . $name,
      ],
      'shared_state_saved_values' => [
        'saved_values',
        static::FORM_STEP,
        $organism_key,
        static::DATA_TYPE,
        // Parent fieldset could be different for Shared state.
        static::SHARED_STATE_PARENT_FIELDSET,
        static::FIELD_NAME . '-' . $name,
      ],
    ];
    foreach ($parent_list as $name => $parents) {
      $value = TppsArray::getValue($form_state, $parents, $key_exists);
      if ($key_exists !== FALSE) {
        break;
      }
    }
    // Nothing was found.
    if ($key_exists === FALSE) {
      // It's not an error and there is no sence to throw an exception.
      watchdog('tpps', 'Organism #@oidx / @field_title: Value was NOT found for @sub_field.',
        [
          '@field_title' => static::getFieldTitle(),
          '@oidx' => $organism_index,
          '@sub_field' => $name,
        ],
        WATCHDOG_WARNING
      );
    }
    return $value;
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
    $form_element = static::getFormElement($organism_index, $form_state);
    return !empty($form_element['columns']);
  }

  /**
   * Gets form element using list of parent elements.
   *
   * WARNING: Could be used ONLY for validation because uses currently
   * submitted form stored in $form_state['complete form'] element.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
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
    $organism_key = 'organism-' . $organism_index;
    $element = $key_exists = NULL;
    if (empty($parents)) {
      // Build parents for current file field.
      $parents = [
        'complete form',
        $organism_key,
        static::DATA_TYPE,
        static::FORM_PARENT_FIELDSET,
        static::FIELD_NAME,
      ];
    }
    $element = TppsArray::getValue($form_state, $parents, $key_exists);
    if ($key_exists === FALSE) {
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
   * WARNING:
   * Allows multiple required groups.
   * Uses different structure in getColumnOptions().
   * Uses diferent output structure (1 level instead of 2).
   *
   * Probably outdated: @TODO Review text below.
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
  public static function validateColumns($organism_index, array &$form_state) {
    if (!method_exists(static::class, 'getRequiredColumns')) {
      // It's not an error if columns not used for this file field.
      // Watchdog could be used for notices.
      return [];
    }
    $required_cases = static::getRequiredColumns();
    $file_element = static::getFormElement($organism_index, $form_state);
    // Check if file has zero length (empty).
    if (empty($file_element['#file']->filesize)) {
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'File is empty.',
        [
          '@field_title' => static::getFieldTitle(),
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get user selected data types for columns.
    $selected_columns = $file_element['#value']['columns'] ?? NULL;
    // Example of structure: ['A' => '1', 'B' => '4',];
    // Note: there is no sense to use FileField::getSelectedColumns() here
    // because it uses different structure of output array.
    if (empty($selected_columns)) {
      // Note: $selected_columns will be empty when user just added file (not clicked 'Upload'
      // button and submitted form (clicked 'Next' button).
      // Probably columns could be detected automatically because they are shown
      // at page and so we could use default values here.
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . "Data types for columns wasn't specified.",
        [
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
      return [];
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if the same data type not used twice.
    static::allowColumnDataTypeUseOnce($organism_index, $selected_columns);
    if (form_get_errors()) {
      return [];
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
    $values = TppsArray::getValue($form_state['values'], $parents);
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
      if (TppsArray::contains($case_data, $selected_columns)) {
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
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
          '!selected_columns' => implode(', ', static::getDataType($selected_columns)),
          '!required_cases' => theme('item_list', ['items' => $required_print_list]),
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
    }

    // See tpps_file_validate_columns().
    // Format: ['1' => 'A', '4' => 'B'].
    return array_flip($selected_columns);
  }

  /**
   * Gets list of user selected data types for columns.
   *
   * Note:
   * User selection is stored in format: ['A' => '1', 'B' => '4'];
   * But returned value uses array_flip().
   *
   * Note:
   * Result could have extra (not required) columns because it uses user's
   * selection. Be sure to call validateColumns() first.

   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   *
   * @return array
   *   Returns list of column's data types selected by user in flipped format.
   *   E.g., ['1' => 'A', '4' => 'B'].
   */
  public static function getSelectedColumns($organism_index, array &$form_state) {
    $file_element = static::getFormElement($organism_index, $form_state);
    return array_flip($file_element['#value']['columns'] ?? []);
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
   *      Value of 'No header' checkbox of the current file.
   *   'use_target_header' bool
   *     When TRUE then header of the target file will be used.
   *     When FALSE then column (not yet implemented).
   *   'column_letter' string
   *     Letter of the column starting form A to match in current file.
   *
   * @return bool
   *   Returns TRUE if values matches and FALSE otherwise.
   */
  public static function matchColumnValues(array $data) {
    // Current file.
    $item_list = tpps_parse_file_column(
      static::getFileId($data['organism_index'], $data['form_state']),
      // Column's letter.
      $data['column_letter'],
      $data['no_header']
    );
    // Target File.
    $target_title = trim(str_replace(': *', '', $data['target_form_element']['#title']));
    $target_fid = TppsArray::getValue(
      $data['form_state'],
      // Use 'input' instead of 'saved_values' to get latest value.
      array_merge(['input'], $data['target_form_element']['#parents'], ['fid'])
    );
    if (empty($data['target_column_letter'])) {
      // Match with values in header.
      $target_item_list = tpps_file_headers($target_fid);
      unset($target_item_list[key($target_item_list)]);
    }
    else {
      // Match with values in column.
      $target_item_list = tpps_parse_file_column(
        $target_fid,
        $data['target_column_letter'],
        $data['target_no_header'] ?? FALSE
      );
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Match.
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
        static::getFormErrorName($data['organism_index']),
        $error_message
      );
      return FALSE;
    }
    return TRUE;
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
   * static::class    // MyNamespace\ClassA when run in A
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

  /**
   * Matches study's type to given type.
   *
   * Note: it checks data type set on Step 2 (not actually added
   * phenotype file because they could not yet be loaded).
   * See Submission::hasGenotypeData() which checks phenotype files
   * loadede on Step 4.
   *
   * @param array $form_state
   *   Drupal Form API State array.
   * @param string $type
   *   Study type. Case independent.
   *   Possible values are: 'Phenotype', 'Genotype' and 'Environment'.
   *
   * @return bool
   *   Returns TRUE when study has given $type and FALSE otherwise.
   *
   * @todo Implement custom method for 'Environment' but this value
   * could be changed in future according to mockup in Figma.
   */
  public static function matchStudyDataType(array $form_state, $type) {
    $study_type = strtolower(TppsArray::getValue($form_state,
      ['saved_values', TPPS_PAGE_2, 'data_type']
    ));
    return (bool) (strpos($study_type, strtolower($type)) !== FALSE);
  }

  /**
   * Checks if study data type contains 'Phenotype'.
   *
   * @param array $form_state
   *   Drupal Form API State array.
   *
   * @return bool
   *   Returns TRUE for phenotype study and FALSE otherwise.
   */
  public static function isPhenotypeStudy(array $form_state) {
    return static::matchStudyDataType($form_state, 'Phenotype');
  }

  /**
   * Checks if study data type contains 'Genotype'.
   *
   * @param array $form_state
   *   Drupal Form API State array.
   *
   * @return bool
   *   Returns TRUE for genotypestudy and FALSE otherwise.
   */
  public static function isGenotypeStudy(array $form_state) {
    return static::matchStudyDataType($form_state, 'Genotype');
  }

  /**
   * Checks if column data type was used only once.
   *
   * WARNING: Designed for validation because uses form_set_error().
   * Column data type could be used only for single column.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $selected_columns
   *   List of data types selected by user for file columns.
   *   List is generated by tpps_file_validate_columns().
   *
   * @return bool
   *   Returns FALSE when at least one data type was set to multiple columns.
   *   Returns TRUE
   */
  public static function allowColumnDataTypeUseOnce($organism_index, array $selected_columns) {
    if (static::FIELD_NAME == 'assay-design') {
      // For Assay Design file.
      $diff = array_unique(array_diff_assoc(
        $selected_columns,
        array_unique($selected_columns)
      ));
      if (!empty($diff)) {
        foreach ($diff as $column_letter => $data_type_key) {
          $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
            . 'The same data type "@data_type" was used twice for different '
            . 'columns. Please either remove the column "@column" '
            . 'from file or use different data type.',
            [
              '@field_title' => t(static::getFieldTitle()),
              '@oidx' => $organism_index,
              '@data_type' => (static::getColumnOptions()[$data_type_key] ?? NULL),
              '@column' => $column_letter,
              // @todo Minor. Show not duplicate only but original letter.
            ]
          );
          form_set_error(static::getFormErrorName($organism_index), $error_message);
        }
        return FALSE;
      }
      return TRUE;
    }
    else {
      // For SNP Association file.
      foreach ($selected_columns as $name => $data) {
        foreach ($data as $key => $value) {
          if ($key == '#type' || !is_numeric($key)) {
            continue;
          }
          if (is_array($value) && count($value) > 1) {
            // Error.
            $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
              . 'The same data type "@data_type" was used twice for different '
              . 'columns. Please either remove one of the columns (@column_list) '
              . 'from file or use different data type.',
              [
                '@field_title' => t(static::getFieldTitle()),
                '@oidx' => $organism_index,
                '@data_type' => check_plain($name),
                '@column_list' => implode(', ', $value),
              ]
            );
            form_set_error(static::getFormErrorName($organism_index), $error_message);
            return FALSE;
          }
        }
      }
    }
    return TRUE;
  }

  /**
   * Stores sub-fields to be used in 'saved_values'.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function storeSubFields($organism_index, array &$form_state) {
    $file_element = static::getFormElement($organism_index, $form_state);
    $parents = $file_element['#parents'];
    array_pop($parents);
    $values = TppsArray::getValue($form_state['values'], $parents);
    $values[static::FIELD_NAME . '-columns'] = $file_element['#value']['columns'] ?? NULL;
    $values[static::FIELD_NAME . '-no-header'] = $file_element['#value']['no-header'] ?? NULL;
    $values[static::FIELD_NAME . '-empty'] = $file_element['#value']['empty'] ?? NULL;
    // There is no need to store required groups because of different structure
    // and this class's method could be used to get data.
    //$values[static::FIELD_NAME . '-groups']] = [];
    TppsArray::setValue($form_state['values'], $parents, $values);
  }

  /**
   * Checks if file was already processed.
   *
   * Allows to avoid calling ::process() method more then once because it's
   * hard to figure out what could cause this exception:
   * 'duplicate key value violates nique constraint
   * "tpps_project_file_managed_tpps_project_file_managed_c1_key"'.
   *
   * DB table 'tpps_project_file_managed' has unique index for 'project_id'
   * and 'fid' columns so we could check if file was already processed by
   * checking if record exists.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $shared_state
   *   Drupal Form API State Array.
   *
   * @return bool
   *   Returns TRUE when file already was processed and record in DB table
   *   'tpps_project_file_managed' exists and FALSE otherwise.
   */
  public static function wasProcessed($organism_index, array $shared_state) {
    $project_id = TppsArray::getValue($shared_state, ['ids', 'project_id']);
    $result = db_select('tpps_project_file_managed', 't')
      ->fields('t', ['project_id'])
      ->condition('project_id', $project_id)
      ->condition('fid', static::getFileId($organism_index, $shared_state))
      ->range(0, 1)->execute()->fetchField();
    return (bool) !empty($result);
  }

  /**
   * Removes field's file by File Id.
   *
   * WARNING: for validation only.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function removeFile($organism_index, array &$form_state) {
    $fid = static::getFileId($organism_index, $form_state);
    if (tpps_file_remove($fid)) {
      $parents = static::getParents($organism_index, $form_state);
      TppsArray::setValue($form_state['values'], $parents, $value = 0);
    }
  }

}
