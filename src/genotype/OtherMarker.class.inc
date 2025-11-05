<?php

/**
 * @file
 * Contains 'SNPs Kinship' class.
 */

/**
 * Manages 'Other Marker' file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class OtherMarker extends FileField {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Common settings.
  //
  const TITLE = 'Other spreadsheet';
  // Name of the form field.
  const DOM_NAME = 'other_marker';
  // Name of the parent fieldset on form (!) not shared form state.
  // static::SNPS_FIELDSET.
  const SNPS_FIELDSET = 'SNPs';
  const OTHER_FIELDSET = 'other';
  // Type of the data.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  const DATA_TYPE = 'genotype';
  // Form step where field is used.
  const FORM_STEP = 4;

  /**
   * Gets parents of the field.
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
  public static function getParents($organism_index = NULL, array $form_state = NULL, $dom_name = NULL) {
    // WARNING: Be sure to use 'default' key in switch block
    // or explicitly set latest version number here.
    $version = NULL;
    if (!is_null($organism_index) && !is_null($form_state)) {
      $version = SubmissionVersion::get($form_state);
    }
    $organism_key = NULL;

    switch ($version) {
      case 1:
        $parents = [
          static::DATA_TYPE,
          'files',
          $dom_name ?? 'other',
        ];
        break;

      case 2:
        $parents = [
          static::DATA_TYPE,
          static::OTHER_FIELDSET,
          $dom_name ?? 'other',
        ];
        break;

      case 3:
      default:
        $parents = [
          static::DATA_TYPE,
          static::OTHER_FIELDSET,
          $dom_name ?? static::DOM_NAME,
        ];
        break;
    }

    if (!is_null($organism_index)) {
      $organism_key = 'organism-' . $organism_index;
      $parents = array_merge([$organism_key], $parents);
    }
    return $parents;
  }

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
  public static function validate($organism_index, array &$form, array &$form_state) {
    $organism_key = 'organism-' . $organism_index;
    $genotype = &$form_state['values'][$organism_key][static::DATA_TYPE] ?? NULL;
    if (($genotype['does_study_include_other_genotypic_data'] ?? NULL) == 'no') {
      return;
    }
    $parents = static::getParents($organism_index, $form_state);
    if (tpps_is_required_field_empty($form_state, $parents)) {
      // Validation failed and no need to do extra checks.
      return;
    }
    $fid = static::getFileId($organism_index, $form_state);

    // @todo Move to 'Accession' class.
    $page3_values = $form_state['saved_values'][TPPS_PAGE_3];
    $species_index = empty($page3_values['tree-accession']['check'])
      ? 'species-1' : 'species-' . $organism_index;
    $tree_accession_file = $page3_values['tree-accession'][$species_index]['file'];
    $organism_key_col_accession_name = $page3_values['tree-accession'][$species_index]
      ['file-groups']['Tree Id']['1'];


    // ? [VS] Should $form_state be used instead of $form here?
    $form_element = static::getFormElement($organism_index, $form_state, $form);
    if (array_key_exists('columns', $form_element)) {
      $required_groups = [
        'Tree Id' => ['id' => [1]],
        'Genotype Data' => ['data' => [0]],
      ];
      $file_element = $form[$organism_key]['genotype'][static::OTHER_FIELDSET]['other_marker'];
      // @todo Use FileField::validateColumns().
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
      // Get Plant Id column name.
      if (!form_get_errors()) {
        $organism_key_col_genotype_name = $groups['Tree Id']['1'];
      }
    }
    else {
      $headers = tpps_file_headers($fid);
      if (!form_get_errors()) {
        $organism_key_col_genotype_name = key($headers);
      }
    }

    if (!form_get_errors()) {
      // @TODO Reuse SnpAssay::validateTrees().
      $acc_no_header = TppsArray::getValue(
        $form_state['saved_values'],
        [TPPS_PAGE_3, 'tree-accession', $species_index, 'file-no-header']
      );
      $missing_trees = tpps_compare_files(
        $fid,
        $tree_accession_file,
        $organism_key_col_genotype_name,
        $organism_key_col_accession_name,
        // Note: Sub-field 'no-header' was removed.
        FALSE,
        $acc_no_header
      );
      if (!empty($missing_trees)) {
        $message = t("Other Marker Genotype Spreadsheet: "
          . "We detected Plant Identifiers that were not in your "
          . "Plant Accession file. Please either remove these plants "
          . "from your Genotype file, or add them to your Plant Accession "
          . "file. The Plant Identifiers we found were: @tree_list",
          ['@tree_list' => implode(', ', $missing_trees)]
        );
        form_set_error(
          static::getFormErrorName($organism_index, $parents),
          $message
        );
      }
    }
    // Preserve file if it is valid.
    tpps_preserve_valid_file(
      $form_state,
      $fid,
      $organism_index,
      'Genotype_Other_Marker_Spreadsheet'
    );
  }

  /**
   * Builds file field.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($organism_index, array &$form, array $form_state) {
    $organism_key = 'organism-' . $organism_index;
    // $title = t('Other spreadsheet: '
    // . '<br />please provide a spreadsheet with columns for the Plant ID '
    // . 'of genotypes used in this study');
    $description = t('Please upload a spreadsheet file containing '
      . 'Genotype data. When your file is uploaded, you will be shown '
      . 'a table with your column header names, several drop-downs, '
      . 'and the first few rows of your file. You will be asked to define '
      . 'the data type for each column, using the drop-downs provided to you. '
      . 'If a column data type does not fit any of the options in the '
      . 'drop-down menu, you may set that drop-down menu to "N/A". '
      . 'Your file must contain one column with the Plant Identifier.');
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => static::getParents($organism_index, $form_state),
      'field_name' => static::DOM_NAME,
      'title' => t(static::TITLE),
      // 'title' => $title,
      'organism_name' => $organism_key,
      'type' => static::DATA_TYPE,
      'description' => $description,
    ]);
  }

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
  public static function process($organism_index, array $shared_state, array &$options = []) {
    $organism_key = 'organism-' . $organism_index;

    $debug_mode = TRUE;
    $params = ['@title' => static::TITLE];
    if ($debug_mode) {
      $message = t('"@title": Started file processing.', $params);
      tpps_log_header($message);
    }
    // Check if file was already processed.
    // WARNING: This extra check was missing in submit_all.php script.
    if (static::wasProcessed($organism_index, $shared_state)) {
      $message = '"@title" file was already processed.';
      tpps_log($message, $params, TRIPAL_DEBUG);
      return;
    }
    if (empty($fid = static::getFileId($organism_index, $shared_state))) {
      if ($debug_mode ?? FALSE) {
        $message = '"@title": File Id is empty.';
        tpps_log($message, $params, TRIPAL_DEBUG);
      }
      return;
    }
    tpps_add_project_file($shared_state, $fid);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Collect necessary data into $options array.
    $options['organism_index'] = $organism_index;
    $options['headers'] = tpps_file_headers($fid);
    $options['type'] = 'other';
    // Value of the select-box 'Other Marker Type'.
    $options['marker'] = TppsArray::getValue($shared_state, [
      'saved_values',
      static::FORM_STEP,
      $organism_key,
      static::DATA_TYPE,
      'files',
      'other-marker',
    ]);
    $options['type_cvterm'] = tpps_load_cvterm('genetic_marker')->cvterm_id;
    $options['shared_state'] = $shared_state;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    tpps_log('"@title": Processing file data...', $params);
    tpps_file_iterator($fid, 'OtherMarker::processRow', $options);
    tpps_log('"@title": Done.', $params);

    tpps_log('"@title": Inserting data into database using insert_multi...', $params);
    tpps_chado_insert_multi($options['records'], $multi_insert_options);
    tpps_log('"@title": Done.', $params);

    tpps_log('"@title": Inserting data into database using insert_hybrid...', $params);
    tpps_chado_insert_hybrid($options['records2'], $multi_insert_options);
    tpps_log('"@title": Done.', $params);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // CREATE INDEXES FROM GENOTYPE_CALL TABLE.
    // tpps_create_genotype_call_indexes();
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    // @todo Check if this is really necessary and remove if possible.
    $options['records'] = [
      'feature' => [],
      'genotype' => [],
      'genotype_call' => [],
      'stock_genotype' => [],
    ];
    if ($debug_mode) {
      $message = t('"@title": Complete file processing.', $params);
      tpps_log_header($message);
    }
  }

  /**
   * Processes a single row of the file.
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
  public static function processRow($row, array &$options = []) {
    static::processSpreadSheet($row, $options);
  }

}
