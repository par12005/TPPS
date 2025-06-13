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
class PhenotypeIso {

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
   * Gets file id.
   *
   * @param int $org_num
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns File Id or NULL if file wasn't uploaded.
   */
  public static function getFileId($org_num, array $form_state) {
    $parents = ['organism-' . $org_num, 'phenotype', static::FIELD_NAME];
    $value = drupal_array_get_nested_value(
      $form_state['values'], $parents, NULL
    ) ?? drupal_array_get_nested_value(
      $form_state['saved_values'][TPPS_PAGE_4], $parents, NULL
    ) ?? NULL;

    // Other way to get value:
    $normal_check = tpps_get_ajax_value(
      $form_state, ["organism-$org_num", 'phenotype', static::FIELD_NAME],
      TRUE
    );
    return $value;
  }

  /**
   * Checks if "Iso" checkbox was selected.
   *
   * Checkbox title:
   * "My phenotypes include results from a mass spectrometry or isotope analysis".
   *
   * @param int $org_num
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return bool
   *   Returns TRUE if checkbox was selected and FALSE otherwise.
   */
  public static function isSelected($org_num, array $form_state) {
    $parents = ["organism-$org_num", 'phenotype', static::CHECKBOX_NAME];
    $value = drupal_array_get_nested_value(
      $form_state['values'], $parents, NULL
    ) ?? drupal_array_get_nested_value(
      $form_state['saved_values'][TPPS_PAGE_4], $parents, NULL
    ) ?? NULL;
    return $value;
  }

  /**
   * Builds form fields for Phenotype Data file.
   *
   * @param int $id
   *   Organism Id.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($id, array &$form, array $form_state) {
    $iso_check = tpps_get_ajax_value(
      $form_state,
      [$id, 'phenotype', 'iso-check'],
      NULL
    );
    if (empty($iso_check)) {
      return;
    }
    $form[$id]['phenotype']['iso'] = [
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
      'columns' => [
        '#description' => t('Please define which columns hold the required '
          . 'data: Plant Identifier, Clone Number, and Metabolomic Data.'
        ),
      ],
      'columns-options' => [
        '#type' => 'hidden',
        '#value' => static::getColumnOptions(),
      ],
    ];
  }

  /**
   * Validates Phenotype ISO file.
   *
   * This file used when checkbox "My phenotypes include results from a
   * mass spectrometry or isotope analysis" on Page 4 was checked.
   *
   * @param int $org_num
   *   Ordinal number of organism.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function validate($org_num, array &$form, array &$form_state) {
    if (!static::isSelected($org_num, $form_state)) {
      return;
    }

    $id = "organism-$org_num";
    $form_field_error_name = "$id][phenotype][iso";
    $file_element = $form[$id]['phenotype']['iso'];

    if (empty($fid = static::getFileId($org_num, $form_state))) {
      $message = t("Phenotype Isotope/Mass Spectrometry File: field is required.");
      form_set_error($form_field_error_name, $message);
      return;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Validate columns.
    $required_groups = [
      'Tree Identifier' => ['id' => [self::DATA_TYPE_PLANT_IDENTIFIER]],
      'Metabolomic Data' => ['metabolomic-data' => [self::DATA_TYPE_METABOLOMIC_DATA]],
    ];
    $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
    if (form_get_errors()) {
      return;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Validate number of 'Metabolomic data' columns.
    $headers = tpps_file_headers($fid);
    $id_col_name = key($headers);
    while (($k = array_search(NULL, $headers))) {
      unset($headers[$k]);
    }
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
      ? 'species-1' : "species-$org_num";
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
      FALSE,
      $acc_no_header
    );

    if (empty($missing_trees)) {
      $params = ['@list' => implode(', ', $missing_trees)];
      $message = "Mass spectrometry/Isotope file: We detected Plant "
        . "Identifiers that were not in your Plant Accession file. Please "
        . "either remove these plants from your file, or add them to your "
        . "Plant Accession file. The Plant Identifiers we found were: @list.";
      form_set_error($form_field_error_name, t($message, $params));
    }

    if (!form_get_errors()) {
      // Preserve file if it is valid.
      tpps_preserve_valid_file($form_state, $fid, $org_num, "Phenotype_Data");
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
      '0' => 'N/A',
      self::DATA_TYPE_METABOLOMIC_DATA => 'Metabolomic Data',
      self::DATA_TYPE_PLANT_IDENTIFIER => 'Plant Identifier',
      self::DATA_TYPE_CLONE_NUMBER => 'Clone Number',
    ];
  }

}
