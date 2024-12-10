<?php

/**
 * @file
 * Contains 'PhenotypeData' class.
 *
 * @todo File format is important for build() and validate(). Maybe helper
 * methods must get it as an argument?
 */

/**
 * Manages Phenotype Data file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PhenotypeData {

  // WARNING: For format = 0.
  // @todo Maybe use fuction which returns correct value by $format.
  const DATA_TYPE_PHENOTYPE_DATA = 0;
  const DATA_TYPE_PLANT_IDENTIFIER = 1;
  const DATA_TYPE_TIMEPOINT = 2;
  const DATA_TYPE_CLONE_NUMBER = 3;
  const DATA_TYPE_YEAR = 5;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Phenotype file formats.
  // Default is FILE_FORMAT_1.
  // Each column is a phenotype.
  // Example: TPPS_IMAGES_PATH . 'phenotype_format_1.png.
  const FILE_FORMAT_1 = 0;
  // One of the columns has phenotype name in each row.
  // Example: TPPS_IMAGES_PATH . 'phenotype_format_2.png.
  const FILE_FORMAT_2 = 1;


  /**
   * Contains Drupal managed file id of the phenotype data file.
   *
   * Note: File id not always exists and must be optional.
   *
   * @todo Implement __construct($file_id).
   */
  public fileId;

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
    $page4_values = $form_state['saved_values']['4'];
    $phenotype_dir = variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');
    $normal_check = tpps_get_ajax_value(
      $form_state, ["organism-$id", 'phenotype', 'normal-check'],
      TRUE
    );
    $format = tpps_get_ajax_value(
      $form_state, ["organism-$id", 'phenotype', 'format'], self::FILE_FORMAT_1
    );
    if (!empty($normal_check)) {
      $states = [
        ':input[name="organism-' . $id . '[phenotype][phenotypes-meta][number]"]'
        => ['value' => '0'],
        ':input[name="organism-' . $id . '[phenotype][check]"]'
        => ['checked' => FALSE],
      ];
      $form["organism-$id"]['phenotype']['format'] = [
        '#type' => 'radios',
        '#title' => t('Phenotype file format: *'),
        '#options' => ['Type 1', 'Type 2'],
        '#ajax' => [
          'callback' => 'tpps_phenotype_file_format_callback',
          'wrapper' => "edit-organism-$id-phenotype-file-ajax-wrapper",
        ],
        '#default_value' => $format,
        // @TODO Minor. Show description above values.
        '#description' => t('Please select a file format type from the '
          . 'listed options. Below please see examples of each format type.'
        ),
        '#states' => ['invisible' => $states],
      ];
      $form["organism-$id"]['phenotype']['format'][self::FILE_FORMAT_1]['#prefix'] =
        '<figure><img src="/' . TPPS_IMAGES_PATH . 'phenotype_format_1.png">'
        . '<figcaption></figcaption></figure>';
      $form["organism-$id"]['phenotype']['format'][self::FILE_FORMAT_2]['#prefix'] =
        '<figure><img src="/' . TPPS_IMAGES_PATH . 'phenotype_format_2.png">'
        . '<figcaption></figcaption></figure>';
      $form["organism-$id"]['phenotype']['file'] = [
        '#type' => 'managed_file',
        '#title' => t('Phenotype file: Please upload a file containing '
          . 'columns for Plant Identifier, Phenotype Data: *'),
        '#upload_location' => 'public://' . $phenotype_dir,
        '#upload_validators' => ['file_validate_extensions' => ['csv tsv']],
        '#tree' => TRUE,
        '#states' => ['invisible' => $states],
        'empty' => [
          '#default_value' => $page4_values["organism-$id"]['phenotype']['file']['empty'] ?? t('NA'),
        ],
        'columns' => [
          '#description' => t('Please define which columns hold the required '
            . 'data: Plant Identifier, Phenotype name, and Value(s)'
          ),
        ],
      ];

      // @TODO Use class's constants for keys of those lists.
      if ($format == PhenotypeData::FILE_FORMAT_2) {
        $form["organism-$id"]['phenotype']['file']['#title'] =
          t('Phenotype file: Please upload a file containing columns '
            . 'for Plant Identifier, Phenotype Name, and value for all '
            . 'of your phenotypic data: *'
        );
      }
      $form["organism-$id"]['phenotype']['file']['columns-options'] = [
        '#type' => 'hidden',
        '#value' => PhenotypeData::getColumnOptions($format),
      ];
      // Ability to use file without header was disabled to make validation
      // simplier and to avoid problems with processing those files.
      // File without header will not pass validation.
      //$form["organism-$id"]['phenotype']['file']['no-header'] = [];
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // This will check if there are Time Phenotypes saved from the
    // saved values and use this to re-check the checkboxes on the form.
    if (isset($page4_values["organism-$id"]['phenotype']['time']['time_phenotypes'])) {
      $time_phenotypes = $page4_values["organism-$id"]['phenotype']['time']['time_phenotypes'] ?? [];
      $count_time_phenotypes = count($time_phenotypes);
      if ($count_time_phenotypes > 0) {
        foreach ($time_phenotypes as $time_phenotype) {
          // If time_phenotype is not 0, meaning the phenotype name
          // and phenotype value would be the same, so not 0.
          if ($time_phenotype != "0") {
            // Adjust the checkbox element on the form to TRUE (checkbox).
            $form["organism-$id"]['phenotype']['time']['time_phenotypes'][$time_phenotype]['#default_value'] = TRUE;
          }
        }
      }
    }
  }

  /**
   * Form API validator for Phenotype data file field.
   *
   * @param int $org_num
   *   Ordinal number of organism.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   *
   * @todo Rename $org_num.
   */
  public static function validate($org_num, array &$form, array &$form_state) {
    $id = "organism-$org_num";
    $page3_values = $form_state['saved_values'][TPPS_PAGE_3] ?? NULL;
    // The form_state values of the phenotype fieldset for organism $id.
    $phenotype = &$form_state['values']["organism-$org_num"]['phenotype'] ?? NULL;
    $file_element = $form[$id]['phenotype']['file'];

    // Check if required columns are present in file.
    switch ($phenotype['format']) {
      case self::FILE_FORMAT_1:
        $required_groups = [
          'Tree Identifier' => ['id' => [self::DATA_TYPE_PLANT_IDENTIFIER]],
          'Phenotype Data' => ['phenotype-data' => [self::DATA_TYPE_PHENOTYPE_DATA]],
        ];
        $has_year_column = in_array(
          self::DATA_TYPE_YEAR, array_values($file_element['#value']['columns'])
        );
        break;

      case self::FILE_FORMAT_2:
        $required_groups = [
          'Tree Identifier' => ['id' => [1]],
          'Phenotype Name/Identifier' => ['phenotype-name' => [2]],
          'Phenotype Value(s)' => ['val' => [3]],
        ];
        $has_year_column = in_array(
          // "Magic number" '6' is a key of 'Year' optons for format 2.
          // See getColumnOptions() method.
          6, array_values($file_element['#value']['columns'])
        );
        break;
    }
    $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

    if (form_get_errors()) {
      return;
    }
    // Check if number of phenotypes in Phenotype Metadata file matches number of
    // columns in Phenotype Data file.




    // Check if all phenotypes listed in Phenotype Metadata file are present
    // in Phenotype Data file.
    if (!form_get_errors()) {
      return;
    }
    $phenotype_file_tree_col = $groups['Tree Identifier']['1'];
    $phenotype_names = array();
    switch ($phenotype['format']) {
      case self::FILE_FORMAT_1:
        // If there is only one column with data then it will be a string
        // but we need an array.
        $phenotype_file_name_cols = is_array($groups['Phenotype Data']['0'])
          ? $groups['Phenotype Data']['0'] : [$groups['Phenotype Data']['0']];
        $headers = tpps_file_headers($phenotype_file, !empty($phenotype['file-no-header']));
        foreach ($phenotype_file_name_cols as $column_index) {
          $phenotype_names[] = $headers[$column_index];
        }
        break;

      case self::FILE_FORMAT_2:
        $phenotype_file_name_col = $groups['Phenotype Name/Identifier']['2'];
        $phenotype_names = tpps_parse_file_column($phenotype_file, $phenotype_file_name_col);
        break;
    }

    $phenotype_meta_names = [];
    if (isset($phenotype_name_col)) {
      $phenotype_meta_names = tpps_parse_file_column($phenotype_meta, $phenotype_name_col);
    }

    for ($i = 1; $i <= $phenotype_number; $i++) {
      $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
    }
    $missing_phenotypes = array_diff($phenotype_names, $phenotype_meta_names);
    if (!empty($missing_phenotypes)) {
      form_set_error("$id][phenotype][file", t("Phenotype file: We detected "
        . "Phenotypes that were not in your Phenotype Metadata file. "
        . "Please either remove these phenotypes from your Phenotype file, "
        . "or add them to your Phenotype Metadata file. The phenotypes we "
        . "detected with missing definitions were: @list.",
        ['@list' => implode(', ', $missing_phenotypes)]
      ));
    }

    if (isset($phenotype_file_tree_col)) {
      $species_index = empty($page3_values['tree-accession']['check']) ? 'species-1' : "species-$org_num";
      $tree_accession_file = $page3_values['tree-accession'][$species_index]['file'];
      $column_vals = $page3_values['tree-accession'][$species_index]['file-columns'];

      foreach ($column_vals as $col => $val) {
        if ($val == '1') {
          $id_col_accession_name = $col;
          break;
        }
      }
      $acc_no_header = $page3_values['tree-accession'][$species_index]['file-no-header'];
      $phenotype_no_header = $phenotype['file-no-header'];

      $missing_trees = tpps_compare_files(
        $phenotype['file'],
        $tree_accession_file,
        $phenotype_file_tree_col,
        $id_col_accession_name,
        $phenotype_no_header,
        $acc_no_header
      );
      if (!empty($missing_trees)) {
        form_set_error("$id][phenotype][file", t("Phenotype file: We "
          . "detected Plant Identifiers that were not "
          . "in your Plant Accession file. Please either remove these "
          . "plants from your Phenotype file, or add them to your Plant "
          . "Accession file. The Plant Identifiers we found were: @list.",
          ['@list' => implode(', ', $missing_trees)]
        ));
      }
    }

    // Preserve file if validation passed.
    tpps_preserve_valid_file($form_state, $phenotype['file'], $org_num, "Phenotype_Data");
  }

  /**
   * Gets list of column data types.
   *
   * @param int $format
   *   Phenotype Data file format. Possible values are:
   *   0 - default. Each column is a phenotype.
   *   1 - one of the columns has phenotype name in each row.
   *
   * @return array
   *   Returns list of options to be used in dropdown menu.
   */
  public static function getColumnOptions($format = 0) {
    if ($format == self::FILE_FORMAT_1) {
      // WARNING: Use explicit keys to avoid errors when order changed.
      return [
        self::DATA_TYPE_PHENOTYPE_DATA => 'Phenotype Data',
        self::DATA_TYPE_PLANT_IDENTIFIER => 'Plant Identifier',
        self::DATA_TYPE_TIMEPOINT => 'Timepoint',
        self::DATA_TYPE_CLONE_NUMBER => 'Clone Number',
        '4' => 'N/A',
        self::DATA_TYPE_YEAR => 'Year',
      ];
    }
    else {
      // WARNING: Use explicit keys to avoid errors when order changed.
      // @TODO Use the same keys as $format == 0 does (if possible).
      return [
        '0' => 'N/A',
        '1' => 'Plant Identifier',
        '2' => 'Phenotype Name/Identifier',
        '3' => 'Value(s)',
        '4' => 'Timepoint',
        '5' => 'Clone Number',
        // Note: Added but not tested.
        '6' => 'Year',
      ];
    }
  }

  /**
   * Gets file header (first row).
   *
   * @param int $file_id
   *   Drupal managed file id.
   *
   * @return array
   *   Returns first row of the file.
   *
   * @todo This is very common function and could be moved to parent class.
   */
  public function getFileHeader($file_id = NULL) {
    $file_id = $file_id ?? $this->fileId ?? NULL;
    if (!empty($file_id)) {
      return tpps_file_get_header($file_id);
    }
  }

  /**
   * Returns the number of rows in a file.
   *
   * @param int $file_id
   *   The Drupal managed file identifier of the file.
   *
   * @return int
   *   The number of lines or rows in the file.
   */
  public function getFileRowNumber($file_id) {
    return tpps_file_len($file_id);
  }

}
