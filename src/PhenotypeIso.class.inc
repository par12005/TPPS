<?php

/**
 * @file
 * Contains 'PhenotypeIso' class.
 */

/**
 * Manages 'Phenotype Isotope/Mass Spectrometry' file field.
 *
 * File field appears when a user selects 'My phenotypes include results from
 * a mass spectrometry or isotope analysis' checkbox.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PhenotypeIso extends TppsFile {

  // Data Type's Id.
  const DATA_TYPE_PLANT_IDENTIFIER = 1;
  const DATA_TYPE_CLONE_NUMBER = 2;
  const DATA_TYPE_METABOLOMIC_DATA = 3;


  // Name of the checkbox which specifies that phenotype "isotope" data file
  // must be used. When This checkbox is checked the normal phenotype data
  // file is required.
  // Note: there is also 'normal-check' checkbox.
  const CHECKBOX_NAME = 'iso-check';

  // Name of the file field.
  // Value is file id.
  const FIELD_NAME = 'iso';

  /**
   * Gets parents for checkbox which makes field required.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   *
   * @return array
   *   Returns parents for the checkbox which makes this field required.
   */
  public static function getCheckboxParents($organism_index) {
    return ['organism-' . $organism_index, 'phenotype', static::CHECKBOX_NAME];
  }

  /**
   * Gets parents of the field.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   *
   * @return array
   *   Returns parents for the checkbox which makes this field required.
   */
  public static function getParents($organism_index) {
    return ['organism-' . $organism_index, 'phenotype', static::FIELD_NAME];
  }

  /**
   * Builds form fields for Phenotype Data file.
   *
   * @param int $organism_key
   *   The HTML id of the organism fieldset being populated.
   *   E.g., 'organism-1'.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($organism_key, array &$form, array $form_state) {
    $allow_column_data_types = FALSE;
    $iso_check = tpps_get_ajax_value(
      $form_state,
      [$organism_key, 'phenotype', static::CHECKBOX_NAME],
      NULL
    );
    if (empty($iso_check)) {
      return;
    }
    $form[$organism_key]['phenotype']['iso'] = [
      '#type' => 'managed_file',
      '#title' => t('Phenotype Isotope/Mass Spectrometry file: *'),
      '#upload_location' => 'public://'
        . variable_get('tpps_phenotype_files_dir', 'tpps_phenotype'),
      '#upload_validators' => ['file_validate_extensions' => ['csv tsv']],
      '#description' => t('Please upload a file containing all of your '
        . 'isotope/mass spectrometry data. The format of this file is very '
        . 'important! The first column of your file should contain plant '
        . 'identifiers which match the plant identifiers you provided in '
        . 'your plant accession file, and all of the remaining columns '
        . 'should contain isotope or mass spectrometry data.'),
    ];
    if ($allow_column_data_types) {
      $form[$organism_key]['phenotype']['iso']['columns'] = [
        '#description' => t('Please define which columns hold the required '
          . 'data: Plant Identifier, Clone Number, and Metabolomic Data.'
        ),
      ];
      $form[$organism_key]['phenotype']['iso']['columns-options'] = [
        '#type' => 'hidden',
        '#value' => static::getColumnOptions(),
      ];
    }
  }

  /**
   * Validates Phenotype ISO file.
   *
   * This file used when checkbox "My phenotypes include results from a
   * mass spectrometry or isotope analysis" on Page 4 was checked.
   *
   * @param int $organism_index
   *   Ordinal number of organism on page.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function validate($organism_index, array &$form, array &$form_state) {
    $debug_mode = FALSE;
    $allow_column_data_types = FALSE;

    if (!static::isSelected($organism_index, $form_state)) {
      return;
    }

    $organism_key = 'organism-' . $organism_index;
    $form_field_error_name = $organism_key . '][phenotype][' . static::FIELD_NAME;
    $file_element = $form[$organism_key]['phenotype'][static::FIELD_NAME];

    if (empty($fid = static::getFileId($organism_index, $form_state))) {
      $message = t("Phenotype Isotope/Mass Spectrometry File: field is required.");
      form_set_error($form_field_error_name, $message);
      return;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Validate columns.
    if ($allow_column_data_types) {
      $required_groups = [
        'Plant Identifier' => ['id' => [static::DATA_TYPE_PLANT_IDENTIFIER]],
        'Metabolomic Data' => ['metabolomic-data' => [static::DATA_TYPE_METABOLOMIC_DATA]],
      ];
      // @TODO Check if Accession file has column 'Clone Number'.
      $has = FALSE;
      if ($has) {
        $required_groups['Clone Number'] = [
          'clone_number' => [static::DATA_TYPE_CLONE_NUMBER]
        ];
      }
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
      if (form_get_errors()) {
        return;
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Validate number of 'Metabolomic data' columns.
    $headers = static::getHeader($organism_index, $form_state);


    // Will be 'A' for 1st column.
    $id_col_name = key($headers);
    while (($k = array_search(NULL, $headers))) {
      unset($headers[$k]);
    }
    // 'Tree Id' is required column and we should decrease number of columns
    // to get number of Metabolomic data columns.
    $num_columns = tpps_file_width($fid) - 1;
    $num_unique_columns = count(array_unique($headers)) - 1;

    if ($num_unique_columns != $num_columns) {
      form_set_error($form_field_error_name, t("Mass spectrometry/Isotope "
        . "file: some columns in the file you provided are missing or have "
        . "duplicate header values. Please either enter valid header values "
        . "for those columns or remove those columns, then reupload your file.")
      );
      return;
    }

    // Get correct accession file and it's metadata for current organism.
    $page3_values = $form_state['saved_values'][TPPS_PAGE_3] ?? NULL;
    $species_index = empty($page3_values['tree-accession']['check'])
      ? 'species-1' : 'species-' . $organism_index;
    $species = $page3_values['tree-accession'][$species_index];
    $tree_accession_file = $species['file'];
    $id_col_accession_name = $species['file-groups']['Tree Id']['1'];
    $acc_no_header = $species['file-no-header'];

    // Compare files.
    $missing_trees = tpps_compare_files(
      $fid,
      $tree_accession_file,
      $id_col_name,
      $id_col_accession_name,
      // @TODO Use actual 'no-header' value of the iso phenotype file.
      FALSE,
      $acc_no_header
    );
    // Note: original code checked if $missing_tree is empty to show error
    // message which is wrong and was fixed but could block submission.
    if (!empty($missing_trees)) {
      $params = ['@list' => implode(', ', $missing_trees)];
      $message = "Mass spectrometry/Isotope file: We detected Plant "
        . "Identifiers that were not in your Plant Accession file. Please "
        . "either remove these plants from your file, or add them to your "
        . "Plant Accession file. The Plant Identifiers we found were: @list.";
      form_set_error($form_field_error_name, t($message, $params));
    }
    if ($debug_mode ?? FALSE) {
      form_set_error('debug', 'DEBUG');
      return;
    }
    if (!form_get_errors()) {
      // Preserve file if it is valid.
      tpps_preserve_valid_file($form_state, $fid, $organism_index, "Phenotype_Data");
    }
  }

  /**
   * Gets list of column data types.
   *
   * @return array
   *   Returns list of options to be used in dropdown menu.
   */
  public static function getColumnOptions() {
    return [
      // 1st item will be selected by default so if there is a lot columns it's
      // easier to change order than manually set it.
      // @TODO Implement pre-selection or suggestions for the columns.
      self::DATA_TYPE_METABOLOMIC_DATA => 'Metabolomic Data',
      self::DATA_TYPE_PLANT_IDENTIFIER => 'Plant Identifier',
      self::DATA_TYPE_CLONE_NUMBER => 'Clone Number',
      '0' => 'N/A',
    ];
  }

  /**
   * Processes a file.
   *
   * Note: called from /forms/submit/submit_all.php script.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $shared_state
   *   Submission's Shared State.
   * @param array $options
   *   Necessary data.
   */
  public static function process($organism_index, array $shared_state, array &$options = []) {
    if (!static::isSelected($organism_index, $shared_state)) {
      return;
    }
    $fid = static::getFileId($organism_index, $shared_state);
    tpps_add_project_file($shared_state, $fid);

    $options['iso'] = TRUE;
    $options['file_headers'] = static::getHeader($organism_index, $shared_state);
    $options['meta'] = [
      'desc' => "Mass Spectrometry",
      // Unit name replaced with Unit Id (cvterm_id for 'chemical substance').
      // Outdated: 'unit' => "intensity (arbitrary nits)".
      'unit' => 139527,
      'unit_id' => 139527,
      'attr_id' => tpps_load_cvterm('intensity')->cvterm_id,
      // Manual term for MASS Spec.
      'struct_id' => tpps_load_cvterm('whole plant')->cvterm_id,
    ];

    tpps_log('ISO_FID: ' . $fid, [], TRIPAL_DEBUG);
    tpps_log('Processing phenotype_data file data...', [], TRIPAL_INFO);
    tpps_file_iterator($fid, 'tpps_process_phenotype_data', $options);
    tpps_log('Inserting phenotype_data into database using insert_multi...', [], TRIPAL_INFO);
    // [VS]
    $id_list = tpps_chado_insert_multi($options['records'], ['fks' => 'phenotype']);
    tpps_log('Done.', [], TRIPAL_INFO);
  }

}
