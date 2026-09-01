<?php

/**
 * @file
 * Contains 'FileField' class.
 */

/**
 * Common methods for file field form elemnts.
 *
 * Terms:
 * 'Column Data Type Index' - integer. See 'DATA_TYPE_*' constants.
 * 'Column Header Title' - string. Value of the column header (1st row).
 * $column_key could be a 'Column Data Type Index' or 'Column Header Title'.
 *
 * Column indexes starts from 1 (not from 0).
 * Column letter starts from 'A'.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
abstract class FileField {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Required Constants.
  //
  // File field title.
  // Example: 'SNP Assay'.
  // WARNING: DO NOT use ' File' suffix.
  const TITLE = '';
  // HTML name of the form field used in DOM.
  // Example: 'snps-assay'.
  const DOM_NAME = '';
  // Type of the data.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  // Example: 'genotype'.
  const DATA_TYPE = '';
  // Form step where field is used.
  const FORM_STEP = '';

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Extra fields.
  // DOM name of the checkbox with title 'Does your file have a header row?'.
  // Note: 'saved values' will have new name static::DOM_NAME . '-no-header'.
  const NO_HEADER = 'no-header';

  /**
   * Builds set of form fields for 'Assay Design Citation' feature.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($organism_index, array &$form, array $form_state) {}

  /**
   * Validates file field.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form
   *   Drupal Form API array.
   * @param array $form_state
   *   Drupal Form API State array.
   */
  public static function validate($organism_index, array &$form, array &$form_state) {}

  /**
   * Processes a file.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $shared_state
   *   Submission's Shared State.
   * @param array $options
   *   Options for processing. Could be used to return processing results.
   */
  public static function process($organism_index, array $shared_state, array &$options = []) {}

  /**
   * Processes a single row of the file.
   *
   * Must be used with tpps_file_iterator().
   * Processes genotype SNP design assay file.
   * Initially done to get position data from the assay design file.
   *
   * @param mixed $row
   *   The item yielded by the TPPS file generator.
   *   Keys are column letters and value is a cell value. Example of structure:
   *   [
   *     [A] => SNP ID
   *     [B] => Scaffold
   *     [C] => Position
   *     [D] => Flank sequence
   *   ].
   * @param array $options
   *   Additional options set when calling tpps_file_iterator().
   */
  public static function processRow($row, array &$options = []) {}

  /**
   * Gets list of column data types.
   *
   * WARNING:
   * Do not change keys of the options because all previously created
   * submissions became broken. Use new or create versioning system.
   *
   * Note:
   * To get name by key: (static::getColumnOptions()[$data_type_key] ?? NULL).
   *
   * @return array
   *   Returns list of options with column data types.
   */
  public static function getColumnOptions() {
    return [];
  }

  /**
   * Checks if file field has column data type selector.
   *
   * @return bool
   *   Return TRUE for file field which has column data type selector and
   *   FALSE otherwise.
   */
  public static function hasColumnDataTypeSelector() {
    return !empty(static::getColumnOptions());
  }

  /**
   * Gets parents of the field.
   *
   * See AssayDesign::getParents() for example.
   *
   * WARNING: List of parents has no page number to allow use it under
   * $form_state['input'].
   *
   * Usage:
   * To get parents for $form skip $organism_index.
   *   $parents = AssayDesign::getParents();
   * To get latest version or skip version detection - skip $form_state array.
   *   $parents = AssayDesign::getParents($organism_index);
   * To detect current version of the $form_state array:
   *   $parents = AssayDesign::getParents($organism_index, $form_state);
   *
   * @param int $organism_index
   *   Ordinal number on form.
   *   When empty list of parents will not include leading organism key element.
   *   E.g., 'organism-N'.
   * @param array $form_state
   *   Drupal Form State array.
   *   When empty latest version of the SubmissionForm will be used. When NOT
   *   empty then $form_state will be searched for SubmissionForm version.
   * @param string $dom_name
   *   When empty then static::DOM_NAME will be used.
   *   When non-empty it's a DOM-name of the subfield. For example:
   *   static::EXISTING_STUDY_FIELD_NAME.
   *
   * @return array
   *   Returns parents for the checkbox which makes this field required.
   */
  abstract public static function getParents($organism_index, array $form_state, $dom_name = NULL);

  /**
   * Gets name of the field for form_set_error().
   *
   * WARNING: no trailing ']' but has first ']'.
   *
   * Difference:
   * getFormErrorName(): "organism-1][genotype][SNPs][snps-association".
   *                                ^                                 ^
   * buildDomName(): "organism-1[genotype][SNPs][snps-association]".
   *                           ^                                 ^
   * See https://api.drupal.org/api/drupal/includes%21form.inc/function/form_set_error/7.x
   *
   * Note: it's not DOM name of the element but path. Drupal uses '#parents'.
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
      $parents = [static::SNPS_FIELDSET, static::DOM_NAME];
    }
    $organism_key = 'organism-' . $organism_index;
    $parents = array_merge([$organism_key, static::DATA_TYPE], $parents);
    return implode('][', $parents);
  }

  /**
   * Builds DOM field name.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   * @param array $form_state
   *   Drupal Form State array.
   * @param string $dom_name
   *   When empty then list of the parents will have static::DOM_NAME
   *   as a last element and given DOM-name otherwise.
   *
   * @return string
   *   Returns field name used in DOM.
   *   Example: 'organism-1[genotype][SNPs][assay-design]'.
   */
  public static function buildDomName($organism_index, array $form_state, $dom_name = NULL) {
    switch (static::FORM_STEP) {
      case TPPS_PAGE_4:
        $organism_key = 'organism-' . $organism_index;
        $parents = static::getParents($organism_index, $form_state, $dom_name);
        TppsArray::removeFirst($parents);
        if (empty($parents)) {
          $message = 'List of the parents is empty for field "@field".';
          $params = ['@dom_name' => $dom_name];
          throw new Exception(t($message, $params));
        }
        $name = $organism_key . '[' . implode('][', $parents) . ']';
        break;

      default:
        $message = 'DOM name was NOT defined for @step".';
        $params = ['@step' => static::FORM_STEP];
        throw new Exception($message);
    }
    return $name;
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
    $dom_name = static::buildDomName($organism_index, $parents);
    return ':input[name="' . $dom_name . '"]';
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Currently submitted form (validation).
    //
    // To always get most recent and up-to-date value we check first recenlty
    // submitted data stored in $form_state['input'] instead of using value
    // stored under $form_state['saved_values'].
    //
    // Any class which extends 'FileField' is a file field so we could get file
    // id for validation (when form not yet submitted) from $form_state['input']
    // using 'fid' hidden sub-element of 'managed_file' element.
    $input_parents = array_merge($parents, ['fid']);
    $value = TppsArray::get($form_state['input'], $input_parents, $key_exists);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if ($key_exists === FALSE) {
      $value = TppsArray::get($form_state['values'], $parents, $key_exists);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Saved Values.
    if ($key_exists === FALSE) {
      $value = TppsArray::get(
        $form_state['saved_values'][static::FORM_STEP],
        $parents,
        $key_exists
      );
    }
    // Nothing was found.
    // It's not an error and there is no sence to throw an exception.
    if ($key_exists === FALSE) {
      $message = 'Organism #@oidx / @field_title: Value was NOT found for @sub_field. !stack';
      if ($debug_mode ?? FALSE) {
        $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 10);
      }
      $params = [
        '@field_title' => static::getFieldTitle(),
        '@oidx' => $organism_index,
        '@sub_field' => implode('/', $parents),
        '!stack' => empty($stack) ? NULL : TppsArray::print($stack),
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
    if (static::DOM_NAME == 'snps-assay') {
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
        static::getSubFieldValue($organism_index, $form_state, static::NO_HEADER),
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
   * See FileField::getValue() and TppsArray::get().
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
    $debug_mode = FALSE;

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
        static::SNPS_FIELDSET,
        static::DOM_NAME,
        $name,
      ],
      // @todo MInor. Probably it's useless step and could be removed.
      'values' => [
        'values',
        $organism_key,
        static::DATA_TYPE,
        static::SNPS_FIELDSET,
        static::DOM_NAME,
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
        static::SNPS_FIELDSET,
        static::DOM_NAME . '-' . $name,
      ],
      'shared_state_saved_values' => [
        'saved_values',
        static::FORM_STEP,
        $organism_key,
        static::DATA_TYPE,
        // Parent fieldset could be different for Shared state.

// @todo UPDATE! Use getParents().


        'files',
        static::DOM_NAME . '-' . $name,
      ],
    ];
    foreach ($parent_list as $name => $parents) {
      $value = TppsArray::get($form_state, $parents, $key_exists);
      if ($key_exists !== FALSE) {
        break;
      }
    }

    // Nothing was found.
    if ($key_exists === FALSE) {
      // It's not an error and there is no sence to throw an exception.
      $message = 'Organism #@oidx / @field_title: Value was NOT found for @sub_field. !stack';
      if ($debug_mode ?? FALSE) {
        $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 10);
      }
      $params = [
        '@field_title' => static::getFieldTitle(),
        '@oidx' => $organism_index,
        '@sub_field' => implode('/', $parents),
        '!stack' => empty($stack) ? NULL : TppsArray::print($stack),
      ];
      watchdog('tpps', $message, $params, WATCHDOG_WARNING);
    }
    return $value;
  }

  /**
   * Checks if file has column with given column key.
   *
   * This check is required for optional 'Column Data Types' like 'Year'.
   * Required data types even won't pass validation.
   * For example column 'Year' in PhenotypeData, SnpAssociation.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form_state
   *   Drupal Form API state array.
   * @param mixed $column_key
   *   Column name (string), ordinal index (int) or data type index (int).
   *
   * @return bool
   *   Returns TRUE if given data type (by index) was selected by user.
   *   Returns FALSE otherwise.
   */
  public static function hasColumn($organism_index, array $form_state, $column_key) {
    return !empty(static::getColumnLetter($organism_index, $form_state, $column_key));
  }

  /**
   * Gets form element using list of parent elements.
   *
   * Order of processing:
   * 1. $form,
   * 2. $form_state['complete_form'],
   *
   * WARNING: Could be used ONLY for validation because uses currently
   * submitted form stored in $form_state['complete form'] element.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   * @param array $form
   *   Optional Drupal Form array.
   *
   * @return mixed
   *   Returns form element or NULL if it wasn't found.
   */
  public static function getFormElement($organism_index, array &$form_state, array $form = NULL) {
    $organism_key = 'organism-' . $organism_index;
    $element = $key_exists = NULL;
    $parents = static::getParents($organism_index, $form_state);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // 1. Use $form first.
    if (!empty($form)) {
      $element = TppsArray::get($form, $parents, $key_exists);
      if ($key_exists) {
        return $element;
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // 2. Use $form_state['complete form'].
    if (empty($form_state['complete form'])) {
      $message = t("Form wasn't found under \$form_state['complete form'].");
      throw new Exception($message);
    }
    // Check if current field belongs to recently submitted form and we could
    // get form element from $form_state['complete form'].
    $complete_form_step = TppsArray::get(
      $form_state['complete form'],
      ['stage', '#value']
    );
    if (static::FORM_STEP != $complete_form_step) {
      $message = "Step number of the submitted form (@complete_form_step) "
        . "differs from step (@field_step) of the field '@field_name'.";
      $params = [
        '@field_name' => static::DOM_NAME,
        '@field_step' => static::FORM_STEP,
        '@complete_form_step' => $complete_form_step,
      ];
      throw new Exception(t($message, $params));
    }
    // Get form element.
    $element = TppsArray::get($form_state['complete form'], $parents, $key_exists);
    if ($key_exists === FALSE) {
      $message = "Form element wasn't found for field @field_name. "
        . "Field parents: @field_parents.";
      $params = [
        '@field_name' => static::DOM_NAME,
        '@field_parents' => implode(', ', $parents),
      ];
      throw new Exception(t($message, $params));
    }
    return $element;
  }

  /**
   * This function validates that the file contains all of the required groups.
   *
   * WARNING: NOT YET IMPLEMENTED.
   *
   * Based on code from function tpps_file_validate_columns() but allows
   * multiple sets of required columns.
   * To use this method class must implement 'getRequiredColumns' method and
   * allow multiple sets of required columns.
   *
   * WARNING:
   * - Not compatible with tpps_file_validate_columns().
   * - Allows multiple required groups.
   * - Uses different structure in getColumnOptions().
   * - Uses diferent output structure (1 level instead of 2).
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form State array.
   * @param array $form
   *   Drupal Form array.
   *
   * @return mixed
   *   The groups and the column option types that the file was valid for.
   *
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
   * @todo Use format 'A1B2': ['A' => 1, 'B' => 2] which differs from used in
   * the function tpps_file_validate_columns().
   */
  public static function validateColumns($organism_index, array &$form_state, array $form = NULL) {
    if (static::isMultipleRequiredColumns()) {
      // @todo see tpps_file_validate_columns().
    }
    else {
      // @todo Implement validation for file fields which has multiple
      // required groups of columns. See AssayDesign::validateColumns().
    }
    $message = t(
      'Method @method_name not yet implemented.',
      ['@method_name' => __METHOD__]
    );
    throw new Exception($message);
  }

  /**
   * Gets list of user selected data types for columns.
   *
   * To get column letter: static::getColumnLetter();
   *
   * Static caching note:
   * No need to use static cache because file_load() already uses caching.
   * See https://api.drupal.org/api/drupal/includes%21file.inc/function/file_load/7.x
   * First call takes about 2.3 msec (cold cache); next 0.05 msec (warm cache).
   *
   * Note:
   * Result could have extra (not required) columns because it uses user's
   * selection. Be sure to call static::validateColumns() first.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   * @param bool $reset
   *   Flag to reset cache.
   *
   * @return array
   *   Returns list of column's data types selected by user.
   *   Key is column letter and value - data type key.
   *   E.g., ['A' => 1, 'B' => 4, 'C' => 2].
   *   This format allows multiple columns to have the same data type and
   *   more descriptive and easy to read then flipped version.
   *
   *   To get column letter:  FileField::getColumnLetter().
   *
   *   To get data type keys: see 'DATA_TYPE_*' class's constants.
   *   Column letter is an uppercase string. Starts from 'A' and contains only
   *   letters. E.g., 'A', 'AB', 'AZ'.
   *   To get column integer column number by column letter use:
   *   FileField::columnLetterToNumber($letters).
   */
  public static function getSelectedColumns($organism_index, array &$form_state, $reset = FALSE) {
    $organism_key = 'organism-' . $organism_index;
    $debug_mode = FALSE;
    // Check if current field belongs to recently submitted form and we could
    // get form element from $form_state['complete form'].
    $complete_form_step = TppsArray::get($form_state['complete form'],
      ['stage', '#value']
    );
    $input_step = TppsArray::get($form_state['input'], ['stage']);
    if (static::FORM_STEP == $complete_form_step) {
      $file_element = static::getFormElement($organism_index, $form_state);
      $selected_columns = $file_element['#value']['columns'] ?? [];
    }
    elseif (static::FORM_STEP == $input_step) {
      // For form validation (DB-record wasn't yet updated).
      // Note: there is no data yet under $form_state['saved_values'][$current_step].
      $parents = static::getParents($organism_index, $form_state, static::DOM_NAME);
      // For searching purposes: 'assay-design-columns'.
      $selected_columns = TppsArray::get(
        $form_state['input'],
        array_merge($parents, ['columns'])
        );
      if ($debug_mode) {
        tpps_log_line();
        tpps_log($selected_columns, 'selected_columns');
      }
    }
    else {
      // For processing (already submitted form/step).
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // WARNING: List of the selected by user data types for the columns has
      // new simple structure.
      $parents = static::getParents($organism_index, $form_state, static::DOM_NAME . '-columns');
      // For searching purposes: 'assay-design-columns'.
      $selected_columns = TppsArray::get($form_state['saved_values'][static::FORM_STEP], $parents);
      if ($debug_mode) {
        tpps_log_line();
        tpps_log($selected_columns, 'selected_columns');
      }
    }
    return $selected_columns;
  }

  /**
   * Gets column letter.
   *
   * Supports fields with and without 'Column Data Type Selector': yes.
   *
   * Checks only existing columns. To get letter for non-existing column:
   *   static::columnNumberToLetter($column_index)
   *
   * For file fields which has 'Column Data Type Selector' gets column letter
   *   by 'Data Type Index' (see class's constants 'DATA_TYPE_*').
   * For file fields which does NOT have'Column Data Type Selector' searches
   *   column letter for header's label (value of the header's sell).
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   * @param mixed $column_key
   *   1. Positive integer: column data type index or column index.
   *   2. Case insensitive (!) string: column header title.
   *
   *   When file field has 'Column Data Type Selector':
   *   - positive integer value treated as 'Data Type Index'.
   *     See class constants DATA_TYPE_*.
   *   When file field has NO 'Column Data Type Selector':
   *   - string treated as column header title.
   *   - positive integer treated first as column header title and then
   *   (if nothing found) as column index (ordinal number).
   *
   * @return string
   *   Returns column letter.
   *   Note: First column has letter 'A'.
   *   Returns FALSE when:
   *   1. $column_key wasn't specified.
   *   2. $column_key wasn't found.
   *
   * @todo Case: field has no header and $column_key is string (not integer).
   * Currently it gives FALSE but we could get first row and search there.
   */
  public static function getColumnLetter($organism_index, array &$form_state, $column_key = NULL) {
    if (is_null($column_key)) {
      return FALSE;
    }
    $letter = FALSE;
    if (static::hasColumnDataTypeSelector($column_key)) {
      if (is_numeric($column_key) && $column_key > 0) {
        // $column_key is an integer 'Data Type Index'.
        $selected = static::getSelectedColumns($organism_index, $form_state);
        if (!empty($selected)) {
          $letter = array_search($column_key, $selected);
        }
      }
      if (empty($letter)) {
        if (static::hasHeader($organism_index, $form_state)) {
          // Check header for string column name.
          // Note: $header will be empty if file has no header.
          $header = static::getFileHeader($organism_index, $form_state);
          if (!empty($header)) {
            // $header format: ['A' => 'Phenotype name', ...].
            $letter = array_search($column_key, $header);
          }
        }
      }
    }
    else {
      if (static::hasHeader($organism_index, $form_state)) {
        // File field has no 'Column Data Type Selector'.
        // $column_key is a string header title.
        $header = static::getFileHeader($organism_index, $form_state);
        if (!empty($header)) {
          // $header format: ['A' => 'Phenotype name', ...].
          // Allow case insensitive search.
          $header = array_map('strtolower', $header);
          $letter = array_search(strtolower($column_key), $header);
        }
      }
    }
    return $letter;
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
    $target_fid = TppsArray::get(
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
    $study_type = strtolower(TppsArray::get($form_state,
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
   * Checks if multiple groups (sets) of required column data types is in use.
   *
   * Note:
   * - Uses static caching.
   * - See method 'getRequiredColumns'.
   * - Newer format it to use multiple groups of the column's data types.
   *   This also means different format of the selected columns in method
   *   'validateColumns'.
   *
   * @param bool $reset
   *   Flag to reset cache.
   *
   * @return mixed
   *   Returns TRUE when field used multiple groups of required columns.
   *   Returns FALSE when field uses single group of required columns.
   *   Returns NULL when field doesn't use columns.
   */
  public static function isMultipleRequiredColumns($reset = FALSE) {
    $key = __METHOD__;
    $static_cache = &drupal_static($key);
    if ($reset || !isset($static_cache[$key])) {
      // Get value.
      if (!method_exists(static::class, 'getRequiredColumns')) {
        // Class has no 'getRequiredColumns' method.
        $value = NULL;
      }
      $required_cases = static::getRequiredColumns();
      $value = (bool) (is_array(reset($required_cases)));
      // Update static cache.
      $static_cache[$key] = $value;
    }
    return $static_cache[$key];
  }

  /**
   * Checks if multiple groups (sets) of required column data types is in use.
   *
   * @return bool
   *   Returns TRUE if class uses newest format of the required columns.
   *   Returns FALSE otherwise and if class doesn't have 'getRequiredColumns'
   *   method (and doesn't use this feature and doesn't validate column's
   *   data types).
   */
  public static function countRequiredColumns() {
    if (!method_exists(static::class, 'getRequiredColumns')) {
      $number = 0;
    }
    else {
      $number = 1;
      if (static::isMultipleRequiredColumns()) {
        $required_cases = static::getRequiredColumns();
        $number = count($required_cases);
      }
    }
    return $number;
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
   *   WARNING: For 'Assay Design' field format is:
   *   'A1B2': ['A' => 1, 'B' => 2].
   *
   * @return bool
   *   Returns FALSE when at least one data type was set to multiple columns.
   *   Returns TRUE when all column data types was selected only once.
   */
  public static function allowColumnDataTypeUseOnce($organism_index, array $selected_columns) {
    if (static::DOM_NAME == 'assay-design') {
      // For Assay Design file.
      $duplicates = TppsArray::getDuplicates($selected_columns);
      if (empty($duplicates)) {
        return TRUE;
      }
      foreach ($duplicates as $column_letter => $data_type_index) {
        $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
          . 'The same data type "@data_type_name" was used twice for different '
          . 'columns. Original column: @original_column_letter. '
          . 'Please either remove the column "@duplicate_column_letter" '
          . 'from file or use different data type.',
          [
            '@field_title' => t(static::getFieldTitle()),
            '@oidx' => $organism_index,
            '@data_type_name' => (static::getColumnOptions()[$data_type_index] ?? NULL),
            // Note: list of the duplicates doesn't include original element
            // but only duplcates.
            '@original_column_letter' => array_search($data_type_index, $selected_columns),
            '@duplicate_column_letter' => $column_letter,
          ]
        );
        form_set_error(static::getFormErrorName($organism_index), $error_message);
      }
      return FALSE;
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
    $project_id = TppsArray::get($shared_state, ['ids', 'project_id']);
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
      TppsArray::set($form_state['values'], $parents, $value = 0);
    }
  }

  /**
   * Builds field's name for Drupal State rule for custom field.
   *
   * @param mixed $organism_index
   *   Ordinal organism number at page.
   * @param array $parents
   *   List of the parents including field name.
   *
   * @return sting
   *   Returns name to be used in Drupal State rules.
   */
  public static function buildCustomStateName($organism_index, array $parents) {
    $organism_key = 'organism-' . $organism_index;
    return ':input[name="' . $organism_key . '['
      . implode('][', $parents) . ']"]';
  }

  /**
   * This function processes a single row of a genotype spreadsheet.
   *
   * Former: tpps_process_genotype_spreadsheet().
   *
   * This function is used for SNP assay files, SSR spreadsheets, and other
   * marker type spreadsheets. The functionality is slightly different based on
   * the type of marker being processed (this is set in the options array). This
   * function is meant to be used with tpps_file_iterator().
   *
   * @param mixed $row
   *   The item yielded by the TPPS file generator.
   * @param array $options
   *   Additional options set when calling tpps_file_iterator().
   *   Keys are: 'shared_state' and etc.
   */
  public static function processSpreadSheet($row, array &$options = []) {
    global $tpps_job;
    $job = $tpps_job;
    $type = $options['type'];
    $organism_index = $options['organism_index'];
    $records = &$options['records'];
    // Hybrid / copy such as genotype_call records.
    $records2 = &$options['records2'];
    $headers = $options['headers'];

    $tree_info = &$options['tree_info'];
    $species_codes = $options['species_codes'];
    $genotype_count = &$options['genotype_count'];
    $project_id = $options['project_id'];
    $marker = $options['marker'];
    if ($marker == 'SSRs') {
      $marker = 'SSR';
    }
    elseif ($marker == 'cpSSRs') {
      $marker = 'cpSSR';
    }

    $type_cvterm = $options['type_cvterm'];
    $seq_var_cvterm = $options['seq_var_cvterm'];
    $multi_insert_options = $options['multi_insert'];
    $associations = $options['associations'] ?? [];
    $vcf_processing_completed = $options['vcf_processing_completed'];
    // $analysis_id = $options['analysis_id'];
    // echo "Analysis ID: $analysis_id\n";

    $record_group = variable_get('tpps_record_group', 10000);
    $stock_id = NULL;

    if (!empty($options['tree_id'])) {
      $val = $row[$options['tree_id']];
      $stock_id = $tree_info[trim($val)]['stock_id'];
      $current_id = $tree_info[trim($val)]['organism_id'];
      $species_code = $species_codes[$current_id];
    }

    $keys = array_keys($row);
    $key_index = -1;
    $tree_id = NULL;
    foreach ($row as $key => $val) {
      $key_index++;
      // echo "ROW key:$key, val:$val\n";
      // tpps_log('ROW KEY $key and ROW VAL $val', [], TRIPAL_INFO);
      if (empty($headers[$key])) {
        continue;
      }
      // This $val is different from the $val later on so order is important.
      if (!isset($stock_id)) {
        // Set the tree_id
        $tree_id = trim($val);
        $study_accession = $options['study_accession'];
        $tree_id = $study_accession . '-' . $tree_id;
        $stock_id = $tree_info[trim($val)]['stock_id'];
        $current_id = $tree_info[trim($val)]['organism_id'];
        $species_code = $species_codes[$current_id];
        continue;
      }
      $genotype_count++;
      echo "Stock ID: $stock_id, Current ID: $current_id, Genotype_count: $genotype_count\n";

      echo "Header before alterations:" . $headers[$key] . "\n";

      $header_length = strlen($headers[$key]);
      // Cater for Diploids [Rish: 8/3/2023].
      if (
        ($options['ploidy'] ?? NULL) == 'Diploid'
        && substr($headers[$key], $header_length - 2, 2) == "_A"
      ) {
        // Remove the _A from the first diploid header
        // and allow the below code to continue to be processed so the SSR can be imported in
        $headers[$key] = substr($headers[$key], 0, $header_length - 2);
        $options['diploid_header'] = $headers[$key];
        $options['diploid_val'] = $val;
        // Save this header for use in a later iteration when _B gets called.
        // This reason for this is we want _A and _B values recorded.
        echo "Diploid first header reset to: " . $headers[$key] . "\n";
        // This will skip processing iteration by ONE iteration if _A (SSR diploid detected).
        continue;
      }

      // [RISH] This is a minor adjustment for diploid done on 8/3/2023
      if (
        ($options['ploidy'] ?? NULL) == 'Diploid'
        && substr($headers[$key], $header_length - 2, 2) == "_B"
      ) {
        $options['diploid_val'] .= ',' . $val;

        // Reset to these new values for insertion into the database later on.
        $headers[$key] = $options['diploid_header'];
        $val = $options['diploid_val'];
        echo "Diploid val: $val\n";
      }
      // End of cater for diploids.

      // Cater for Polyploids [RISH: 8/7/2023]
      // Get header without the trailing _X (_1,_2,_3 etc)
      $header_parts = explode("_", $headers[$key]);
      $header_parts_length = count($header_parts);
      $header_without_polyploid_index = "";
      for ($j = 0; $j < $header_parts_length - 1; $j++) {
        if ($j > 0) {
          $header_without_polyploid_index .= "_";
        }
        $header_without_polyploid_index .= $header_parts[$j];
      }

      if (
        ($options['ploidy'] ?? NULL) == 'Polyploid'
        && $options['polyploid_header'] != $header_without_polyploid_index
      ) {
        // Remove the _1 from the first diploid header and allow the below
        // code to continue to be processed so the SSR can be imported in.
        $headers[$key] = $header_without_polyploid_index;
        $options['polyploid_header'] = $headers[$key];
        $options['polyploid_val'] = $val;
        // Save this header for use in a later iteration when _B gets called.
        // This reason for this is we want _A and _B values recorded.
        echo "Polyploid first header reset to: " . $headers[$key] . "\n";
        // This will skip processing iteration by ONE iteration if _1
        // (SSR diploid detected).
        continue;
      }

      // [RISH] This is a minor adjustment for polyploid done on 8/7/2023
      // Look forward to see if the next headers_key
      $header_next = @$headers[$keys[$key_index + 1]];
      // Get next header without the trailing _X (_1,_2,_3 etc)
      $header_next_parts = explode("_", $header_next);
      $header_next_parts_length = count($header_next_parts);
      $header_next_without_polyploid_index = "";
      for ($j = 0; $j < $header_next_parts_length - 1; $j++) {
        if ($j > 0) {
          $header_next_without_polyploid_index .= "_";
        }
        $header_next_without_polyploid_index .= $header_next_parts[$j];
      }

      if (
        ($options['ploidy'] ?? NULL) == 'Polyploid'
        && $options['polyploid_header'] == $header_without_polyploid_index
      ) {
        // Append the new value to what was already there.
        $options['polyploid_val'] .= ',' . $val;

        // Check if the next header does not match current header (this would
        // mean next header starts a new SSR polyploid) OR if the next header
        // is NULL (this means end of headers of the file) so we need to allow
        // the rest of code below to happen to insert this current SRR polyploid.
        if (
          ($header_without_polyploid_index != $header_next_without_polyploid_index)
          // NULL happens if end of all headers.
          || $header_next == NULL
        ) {
          // We have found that $headers[$key] is the last polyploid column
          // for the current SSR so reset to these new values for insertion
          // into the database later on.
          $headers[$key] = $options['polyploid_header'];
          $val = $options['polyploid_val'];
          echo "Polyploid val: $val for insertion using SSR marker" . $headers[$key] . "\n";
        }
        else {
          // This will skip insertion (below code) until all values for the
          // current SSR polyploid is found.
          echo "Skipping insert\n";
          continue;
        }
      }
      // End of catering for polyploids.
      echo "Processing the insert\n";

      if ($type == 'ssrs' and !empty($options['empty']) and $val == $options['empty']) {
        continue;
      }

      if ($type == 'ssrs' and ($val === 0 or $val === "0")) {
        $val = "NA";
      }

      // RISH NOTES: This addition uses the organism_id based on the organism
      // order of the fourth page (we likely have to pass the i from previous
      // function here)
      // THIS TECHNICALLY OVERRIDES PETER'S LOGIC ABOVE. TO BE DETERMINED IF
      // RISH'S WAY IS CORRECT OR NOT [6/22/2023]
      // THIS WAS AN ISSUE BROUGHT UP BY EMILY REGARDING SNPS NOT BEING
      // ASSOCIATED WITH POP TRICH (665 STUDY)
      $species_code = NULL;
      $organism_id = NULL;
      $count_tmp = 0;
      foreach ($species_codes as $organism_id_tmp => $species_code_tmp) {
        $count_tmp = $count_tmp + 1;
        // Check if count_tmp matches $organism_index.
        if ($count_tmp == $organism_index) {
          $species_code = $species_code_tmp;
          $organism_id = $organism_id_tmp;
          break;
        }
      }

      // $headers[$key] is accessing the header value for the current column being processed in the genotype spreadsheet. In this context, it is used to retrieve the name of the variant (e.g., a specific SNP or SSR marker) that corresponds to the current column in the row of data being processed. This variant name is then used to construct unique identifiers for features and genotypes in the database, ensuring that each marker and its associated genotype data are correctly linked to the appropriate organism and study.
      $variant_name = $headers[$key];
      $marker_name = $variant_name . $marker;
      $genotype_name_without_call = "$marker-$variant_name-$species_code";
      $genotype_name = "$marker-$variant_name-$species_code-$val";

      // Check if feature exists, if not insert.
      $sql = 'SELECT count(*) as c1 FROM chado.feature WHERE uniquename = :marker_name';
      $feature_check_results = chado_query($sql, [':marker_name' => $marker_name]);
      $feature_check_count = $feature_check_results->fetchObject()->c1;
      if ($feature_check_count <= 0) {
        chado_insert_record('feature', [
          'name' => $marker_name,
          'organism_id' => $organism_id,
          'uniquename' => $marker_name,
          'type_id' => $seq_var_cvterm,
        ]);
      }
      // Lookup the marker_name_id.
      $sql = "SELECT feature_id FROM chado.feature WHERE uniquename = :uniquename";
      $results = chado_query($sql, [':uniquename' => $marker_name]);
      $marker_name_id = NULL;
      foreach ($results as $row) {
        $marker_name_id = $row->feature_id;
      }
      // Check if feature exists, if not insert.
      $feature_check_results = chado_query(
        'SELECT count(*) as c1 FROM chado.feature WHERE uniquename = :variant_name',
        [':variant_name' => $variant_name]
      );
      $feature_check_count = $feature_check_results->fetchObject()->c1;

      if ($feature_check_count <= 0) {
        chado_insert_record('feature', [
          'name' => $variant_name,
          'organism_id' => $organism_id,
          'uniquename' => $variant_name,
          'type_id' => $seq_var_cvterm,
        ]);
      }

      // Lookup the variant_name_id.
      $results = chado_query("SELECT feature_id FROM chado.feature
        WHERE uniquename = :uniquename", [':uniquename' => $variant_name]
      );
      $variant_name_id = NULL;
      foreach ($results as $row) {
        $variant_name_id = $row->feature_id;
      }


      if (!empty($associations) and !empty($associations[$variant_name])) {
        $association = $associations[$variant_name];
        $assoc_feature_name = "{$variant_name}-{$options['associations_type']}-{$association['trait']}";
        echo "Association data for this row is being processed\n";
        $records['feature'][$association['scaffold']] = [
          'organism_id' => $current_id,
          'uniquename' => $association['scaffold'],
          'type_id' => $options['scaffold_cvterm'],
        ];
        $records['feature'][$assoc_feature_name] = [
          'organism_id' => $current_id,
          'uniquename' => $assoc_feature_name,
          'type_id' => $seq_var_cvterm,
        ];
        if (!empty($association['trait_attr'])) {
          $records['feature_cvterm'][$assoc_feature_name] = [
            'cvterm_id' => $association['trait_attr'],
            'pub_id' => $options['pub_id'],
            '#fk' => ['feature' => $assoc_feature_name],
          ];
          if (!empty($association['trait_obs'])) {
            $records['feature_cvtermprop'][$assoc_feature_name] = [
              'type_id' => $association['trait_obs'],
              '#fk' => ['feature_cvterm' => $assoc_feature_name],
            ];
          }
        }
        $records['featureprop'][$assoc_feature_name] = [
          'type_id' => $options['associations_type'],
          '#fk' => ['feature' => $assoc_feature_name],
        ];
        // PETER's code - which doesn't connect to analysis.
        $records['featureloc'][$variant_name] = [
          'fmin' => $association['start'],
          'fmax' => $association['stop'],
          'residue_info' => $association['allele'],
          '#fk' => [
            'feature' => $variant_name,
            'srcfeature' => $association['scaffold'],
          ],
        ];
        $records['feature_relationship'][$assoc_feature_name] = [
          'type_id' => $options['associations_type'],
          'value' => $association['confidence'],
          '#fk' => [
            'subject' => $variant_name,
            'object' => $assoc_feature_name,
          ],
        ];
      }
      $process_genotype_assay_new_naming = TRUE;
      if ($process_genotype_assay_new_naming) {
        $genotype_name = $variant_name . '-' . $val;
        tpps_safe_chado_insert_record('genotype', [
          'name' => $genotype_name,
          'uniquename' => $genotype_name,
          'description' => $val,
          'type_id' => $type_cvterm,
        ]);
        // @TODO Use data returned by chado_insert_record() to get genotype_id.
        // https://tripal.readthedocs.io/en/latest/dev_guide/chado.html
        // On success this function returns the inserted record with the new primary
        // keys added to the returned array. On failure, it returns FALSE.
        $genotype_id = tpps_submitall_get_genotype_id($genotype_name);
        $options = array_merge($options, [
          'genotype_id' => $genotype_id,
          'genotype_name' => $genotype_name,
        ]);
        SnpAssociation::process($organism_index, $options['shared_state'], $options);
        tpps_safe_chado_insert_record('feature_genotype', [
          'feature_id' => $variant_name_id,
          'genotype_id' => $genotype_id,
          'chromosome_id' => NULL,
          'rank' => 0,
          'cgroup' => 0,
          'cvterm_id' => $type_cvterm,
        ]);
        tpps_log("Genotype_call key: @key", ['@key' => $stock_id . '-' . $genotype_name]);
        if (isset($records2['genotype_call']["$stock_id-$genotype_name"])) {
          tpps_log("This genotype_call key is already set (so uniqueness is maybe broken?");
        }
        $records['stock_genotype']["$stock_id-$genotype_name"] = [
          'stock_id' => $stock_id,
          'genotype_id' => $genotype_id,
        ];
      }

      if ($genotype_count >= $record_group) {
        if ($vcf_processing_completed == TRUE && $type == 'snp') {
          tpps_log('Skipped genotype and genotype_call SNPs since VCF already loaded...', [], TRIPAL_INFO);
        }
        tpps_log('Inserting data into database using insert_multi...', [], TRIPAL_INFO);
        tpps_chado_insert_multi($records, $multi_insert_options);
        tpps_log('Inserting data into database using insert_hybrid...', [], TRIPAL_INFO);
        tpps_chado_insert_hybrid($records2, $multi_insert_options);
        tpps_log('Done.', [], TRIPAL_INFO);
        $records = [
          'feature' => [],
          'genotype' => [],
          'genotype_call' => [],
          'stock_genotype' => [],
        ];
        // Do this for the hybrid (COPY) command.
        $records2 = [
          'genotype_call' => [],
        ];
        if (!empty($associations)) {
          $records['featureloc'] = [];
          $records['featureprop'] = [];
        }
        $options['genotype_total'] += $genotype_count;
        tpps_log('Genotypes inserted:' . $options['genotype_total'], [], TRIPAL_INFO);
        $genotype_count = 0;
      }
    }
  }

  /**
   * Gets file's header (1st row).
   *
   * To get 1st row (which is not header):
   *   static::getFileFirstRow($organism_index, $form_state, $reset);
   * Static caching note:
   *   Using static cache is useful even when file_load() already uses caching.
   *   First call takes about 7 msec (cold cache) and next 2.5 msec (warm cache).
   *   Static cache: 7 msec / 0.4 msec.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form_state
   *   Drupal Form API State array.
   * @param bool $reset
   *   Flag to reset cache.
   *
   * @return array
   *   Returns the header of the file.
   *   Returns empty array when:
   *   1. File has no header.
   *      Checkbox 'Does your file have a header row?' set to 'No'.
   *   2. File wasn't found.
   *   Format: Key is column letter and value is header's name.
   *   E.g., ['A' => 'SNP_ID', 'B' => 'SNP1', 'C' => 'SNP2', 'D' => 'SNP3'].
   *
   * @todo Return NULL instead of empty array.
   */
  public static function getFileHeader($organism_index, array $form_state, $reset = FALSE) {
    $key = static::generateUniqueKey($organism_index, $form_state);
    $static_cache = &drupal_static(__METHOD__);
    if ($reset || !isset($static_cache[$key])) {
      $output = [];
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Get data.
      if (static::hasHeader($organism_index, $form_state)) {
        $output = static::getFileFirstRow($organism_index, $form_state, $reset);
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      $static_cache[$key] = $output;
    }
    return $static_cache[$key];
  }

  /**
   * Gets the 1st row of the file.
   *
   * Note: Doesn't respect 'no-header' field because it's no a header.
   *
   * Static caching note:
   *   Using static cache is useful even when file_load() already uses caching.
   *   First call takes about 7 msec (cold cache) and next 2.5 msec (warm cache).
   *   Static cache: 7 msec / 0.4 msec.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form_state
   *   Drupal Form API State array.
   * @param bool $reset
   *   Flag to reset cache.
   *
   * @return array
   *   Returns the 1st row of the file.
   *   Returns empty array when file wasn't found.
   *   Format: Key is column letter and value is header's name.
   *   E.g., ['A' => 'SNP_ID', 'B' => 'SNP1', 'C' => 'SNP2', 'D' => 'SNP3'].
   */
  public static function getFileFirstRow($organism_index, array $form_state, $reset = FALSE) {
    $key = static::generateUniqueKey($organism_index, $form_state);
    $static_cache = &drupal_static(__METHOD__);
    if ($reset || !isset($static_cache[$key])) {
      $output = [];
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Get data.
      if ($fid = static::getFileId($organism_index, $form_state)) {
        // WARNING: tpps_file_headers() uses $no_header in a different way.
        $output = tpps_file_headers($fid, $no_header = FALSE);
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      $static_cache[$key] = $output;
    }
    return $static_cache[$key];
  }

  /**
   * Matches column with given list of values.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State.
   * @param string $column_letter
   *   Letter of the column which will be matched.
   *   'A' for 1st column, 'B' for second and so on.
   * @param array $list
   *   List of values with given $column must match.
   *
   * @return array
   *   Returns list of the missing items in current file which present in
   *   given $list.
   */
  public static function matchColumn($organism_index, array $form_state, $column_letter, array $list) {
    // Match lists of SNPs from those 2 files.
    $column_values = tpps_parse_file_column(
      static::getFileId($organism_index, $form_state),
      $column_letter,
      static::getNoHeader($organism_index, $form_state)
    );
    $missing_items = array_diff($column_values, $list);
    return $missing_items;
  }

  /**
   * Gets value of the 'no-header' field.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State.
   *
   * @return mixed
   *   Returns raw value of the field.
   *   Checkbox field has title 'Does your file have a header row?'
   *   so values are iverted:
   *     '0' means file field has header.
   *     '1' means no header.
   */
  public static function getNoHeader($organism_index, array $form_state) {
    // For search purpose: 'snps-association-no-header'.
    if (
      // For validation we use just submitted value (not saved before).
      static::FORM_STEP == TppsArray::get($form_state['input'], ['stage'])
    ) {
      // For just submitted form/step.
      // Example: ['organism-1', 'phenotype', 'file', 'no-header'].
      $parents = static::getParents($organism_index, $form_state);
      $parents[] = 'no-header';
      // $form_state['values'] has no value but $form_state['input']
      // has is so we will use it.
      $value = TppsArray::get($form_state['input'], $parents);
    }
    else {
      // For processing only.
      // Example: ['organism-1', 'phenotype', 'file-no-header'].
      // @TODO Use the same structure under 'saved_values' to avoid extra code.
      $parents = static::getParents($organism_index, $form_state,
        static::DOM_NAME . '-' . static::NO_HEADER
      );
      $value = TppsArray::get($form_state['saved_values'], $parents);
    }
    return $value;
  }

  /**
   * Checks if file has header.
   *
   * Note: We can't check if th first row is actually a header
   * but only study submitter could specify it using dropdown menu with title
   * 'Does your file have a header row?'.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State.
   *
   * @return bool
   *   Returns TRUE when file has header and FALSE otherwise.
   */
  public static function hasHeader($organism_index, array $form_state) {
    return !static::getNoHeader($organism_index, $form_state);
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
   * Gets list of file field classes.
   *
   * @return array
   *   Returns list of class names.
   */
  public static function getClassList() {
    // @todo Scan folder and dynamically build list of classes.
    // @todo Add all file fields to this list.
    return [
      // Genotype.
      'AssayDesign',
      'OtherMarker',
      'SnpAssay',
      'SnpAssociation',
      'SnpsKinship',
      'SnpsPopulationStructure',
      // Phenotype.
      'PhenotypeData',
      'PhenotypeIso',
      'PhenotypeMeta',
    ];
  }

  /**
   * Gets file path.
   *
   * Uses static caching.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   * @param bool $reset
   *   Flag to reset cache.
   *
   * @return string
   *   Returns absolute path to file.
   */
  public static function getFilePath($organism_index, array $form_state, $reset = FALSE) {
    $unique = [$form_state['accession'], static::DOM_NAME, $organism_index];
    $key = implode('_', $unique);
    $static_cache = &drupal_static(__METHOD__);
    if ($reset || !isset($static_cache[$key])) {
      $output = NULL;

      $fid = static::getFileId($organism_index, $form_state);
      if (!empty($fid) && $file = tpps_file_load($fid)) {
        $output = tpps_get_location($file->uri);
      }

      $static_cache[$key] = $output;
    }
    return $static_cache[$key];
  }

  /**
   * Gets header column index by column name.
   *
   * Supports fields with/without 'Column Data Type Selector': yes.
   *
   * WARNING:
   * 1. Indexes starts from 0.
   *    See FileField::columnLetterToNumber() which count from 1.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   * @param mixed $column_key
   *   Column key could be:
   *   1. Integer column's data type key.
   *   2. String column name.
   *
   * @return mixed
   *   Returns integer column index (0 means 1st column).
   *   Returns NULL when column wasn't found.
   *   Returns FALSE when:
   *   1. $column_key wasn't specified
   *   2. File has no header.
   *
   * @todo Merge with FileField::columnLetterToNumber(). Note: This function
   * used by submit_all.php and uses NULL and FALSE values differently.
   */
  public static function getHeaderIndex($organism_index, array $form_state, $column_key = NULL) {
    if (is_null($column_key)) {
      return FALSE;
    }
    $letter = static::getColumnLetter($organism_index, $form_state, $column_key);
    if (!empty($letter)) {
      return (static::columnLetterToNumber($letter) - 1);
    }
    return NULL;
  }

  /**
   * Converts letters into number.
   *
   * Used for file field column letter like 'AA', 'AAX' and etc.
   * Inspiration: https://stackoverflow.com/a/55226643
   *
   * @param string $letters
   *   Letters.
   *
   * @return int
   *   Returns number. For example: A = 1, B = 2 and etc.
   */
  public static function columnLetterToNumber($letters) {
    if (empty($letters)) {
      return FALSE;
    }
    $alphabet = range('A', 'Z');
    $number = 0;
    foreach (str_split(strrev($letters)) as $key => $char) {
      $number = $number + (array_search($char, $alphabet) + 1) * pow(count($alphabet), $key);
    }
    return $number;
  }

  /**
   * Converts a column index to an Excel-style column letter.
   *
   * @param int $column_index
   *   Column index.
   *   Note: 1st column has index '1'.
   *
   * @return string
   *   Returns column letter.
   *   E.g., 1 to A, 26 to Z, 27 to AA, etc.
   *   Returns FALSE if $column_index is not positive integer.
   */
  public static function columnNumberToLetter($column_index) {
    $column_letter = FALSE;
    while ($column_index > 0) {
      $modulo = ($column_index - 1) % 26;
      $column_letter = chr(65 + $modulo) . $column_letter;
      $column_index = floor(($column_index - $modulo) / 26);
    }
    return $column_letter;
  }

  /**
   * Generates unique key for static caching.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return string
   *   Returns key which is unique per organism.
   */
  public static function generateUniqueKey($organism_index, array $form_state) {
    $unique = [
      ($form_state['accession'] ?? NULL),
      static::DOM_NAME,
      $organism_index,
    ];
    return implode('_', $unique);
  }

  /**
   * Get column values from file by column key.
   *
   * Supports fields:
   * 1. with/without 'Column Data Type Selector': yes.
   * 2. with/without header: yes.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   * @param mixed $column_key
   *   1. Positive integer: column data type index or column index.
   *   2. Case sensitive string: column header title.
   *   See FileField::getColumnLetter().
   *
   * @return mixed
   *   Returns values of the column.
   *   Returns NULL when $column_key is empty.
   *   Note:
   *   When field has no header but uses 'Column Data Type Selector'
   *   and $column_key is a data type index (not column's ordinal number or
   *   column's header) then column values will be returned.
   */
  public static function getColumnValues($organism_index, array $form_state, $column_key) {
    $organism_key = 'organism-' . $organism_index;
    if (empty($column_key)) {
      return NULL;
    }
    return tpps_parse_file_column(
      static::getFileId($organism_index, $form_state),
      static::getColumnLetter($organism_index, $form_state, $column_key),
      // WARNING; '0' means file has header, '1' means doesn't have.
      static::getNoHeader($organism_index, $form_state)
    );
  }

  /**
   * Checks if give 'Column Key' is a Column Data Type Index.
   *
   * Note: Column could be addressed by 'Column Letter' so it's safe enough
   * to do this validation.
   *
   * @param mixed $column_key
   *   Column key could be:
   *   1. Integer column's data type key (see class's constants 'DATA_TYPE_')
   *      for file field which has column data type selector.
   *      E.g., 'Assay Design', 'SNP Association' and etc.
   *      Note: usually '0' not used for data type index but means not
   *      selected default value.
   *   2. String column name from file's header for file fields which doesn't
   *      allow to select column data type. E.g., 'SNP Assay'.
   *
   * @return bool
   *   Returns TRUE when $column_key has column data type index.
   *   Returns FALSE otherwise.
   */
  public static function isColumnDataTypeIndex($column_key) {
    return (bool) (
     // File field has 'Column Data Type Selector'.
      static::hasColumnDataTypeSelector()
      // Value of the $column_key is positive integer.
      && is_numeric($column_key)
      // Note: Data Type indexes start from 1.
      && $column_key > 0
    );
  }

}
