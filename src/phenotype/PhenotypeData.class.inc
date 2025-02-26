<?php

/**
 * @file
 * Contains 'PhenotypeData' class.
 *
 * Phenotype data file is always required. Metadata could be added manually
 * (PhenotypeManual class) or using file (PhenotypeMeta class).
 *
 * @todo File format is important for build() and validate(). Maybe helper
 * methods must get it as an argument?
 *
 * Values of column 'year' will be stored in table 'chado.phenotypeprop'.
 * See method getYearCvTermId() for more details.
 */

/**
 * Manages Phenotype Data file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PhenotypeData {

  // Form step where field is used.
  const FORM_STEP = 4;

  // WARNING: For format = 0.
  // @todo Minor. Maybe use fuction which returns correct value by $format.
  const DATA_TYPE_PHENOTYPE_DATA = 0;
  const DATA_TYPE_PLANT_IDENTIFIER = 1;
  const DATA_TYPE_TIMEPOINT = 2;
  const DATA_TYPE_CLONE_NUMBER = 3;
  const DATA_TYPE_YEAR = 5;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Phenotype file formats.
  //
  // Phenotypes in columns.
  // Default format of the data file where each column has phenotype data.
  // Header: tree_id, phenotype_1, ..., phenotype_N.
  // Example: TPPS_IMAGES_PATH . 'phenotype_format_1.png.
  const FILE_FORMAT_1 = 0;
  // Phenotypes in rows.
  // Phenotype's name in one column and phenotype's value in other.
  // Header: tree_id, phenotype, value.
  // Example: TPPS_IMAGES_PATH . 'phenotype_format_2.png.
  const FILE_FORMAT_2 = 1;

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
    $page4_values = $form_state['saved_values']['4'] ?? NULL;
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
    $phenotype = &$form_state['values']["organism-$org_num"]['phenotype'] ?? NULL;

    // Checkbox (checked by default): "My phenotypes include traits and/or
    // environmental information other than mass spectrometry or isotope analysis.
    $normal_check = $phenotype['normal-check'] ?? NULL;
    // Checkbox: "My phenotypes include results from a mass spectrometry or
    // isotope analysis".
    $iso_check = $phenotype['iso-check'] ?? NULL;

    if ($normal_check) {
      self::validateNormal($org_num, $form, $form_state);
      if (!form_get_errors()) {
        tpps_preserve_valid_file($form_state,
          // Phenotype data file id.
          $phenotype['file'], $org_num, "Phenotype_Data");
      }
    }
    if ($iso_check) {
      self::validateIso($org_num, $form, $form_state);
      if (!form_get_errors()) {
        tpps_preserve_valid_file($form_state,
          // Phenotype iso file id.
          $phenotype['iso'],
          $org_num, "Phenotype_Data");
      }
    }
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
   * Validated normal phenotype data file.
   *
   * @param int $org_num
   *   Ordinal number of organism.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function validateNormal($org_num, array &$form, array &$form_state) {
    $phenotype = &$form_state['values']["organism-$org_num"]['phenotype'] ?? NULL;
    if (empty($phenotype['normal-check']) || empty($phenotype)) {
      return;
    }

    $id = "organism-$org_num";
    $page3_values = $form_state['saved_values'][TPPS_PAGE_3] ?? NULL;
    // The form_state values of the phenotype fieldset for organism $id.
    // @TODO Minor. Rename to $file_form_element.
    $file_element = $form[$id]['phenotype']['file'];
    $file_id = $phenotype['file'] ?? NULL;
    $metadata_file_id = $phenotype['metadata'];
    $form_field_error_name = "$id][phenotype][file";

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if required data file was uploaded.
    if (empty($file_id)) {
      $message = 'Phenotype File is required.';
      form_set_error($form_field_error_name, t($message));
      return;
    }
    $is_metadata_file = (bool) ($phenotype['check'] && !empty($file_id));

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if number of rows in data file matches number of phenotypes in
    // metadata file.
    if ($phenotype['format'] == self::FILE_FORMAT_2) {
      // @todo Implement/add for FILE_FORMAT_2.
    }
    elseif ($phenotype['format'] == self::FILE_FORMAT_1) {
      $file_header = tpps_file_get_header($file_id);
      if (
        is_array($file_header)
        && ($column_number = count($file_header))
      ) {
        if ($is_metadata_file) {
          $phenotype_number = tpps_file_len($metadata_file_id);
        }
        else {
          $phenotype_number = $phenotype['phenotypes-meta']['number'];
        }
        // One column has Plant Id and we need to exclude it.
        $file_phenotypes_count = $column_number - 1;
        // If phenotype file has column 'year' which is not actually
        // phenotype and doesn't have own row in metadata file we need to
        // compare files differently.
        if ($has_year_column) {
          $file_phenotypes_count = $file_phenotypes_count - 1;
        }
        if ($phenotype_number != $file_phenotypes_count) {
          $params = [
            '@count' => $phenotype_number,
            '@file_count' => $file_phenotypes_count,
          ];
          if ($is_metadata_file) {
            $message = 'Number of phenotypes in Phenotype Metadata file '
              . 'NOT matches number of columns in Phenotype file. <br />Number '
              . 'of phenotypes in Metadata File: <strong>@count</strong>.';
          }
          else {
            // Manually added Phenotype Metadata. File wasn't used.
            $message = 'Number of phenotypes NOT matches number of columns '
              . 'in Phenotype File. <br />Number of manually added phenotypes: '
              . '<strong>@count</strong>.';
            // $phenotype['metadata'] could have Phenotype Metadata File Id if user
            // first uploaded file and then decided to manually add phenotypes.
            // We need to remove this file if it exists.
            if (!empty($phenotype['metadata'])) {
              // Remove already uploaded file.
              $file = tpps_file_load(($phenotype['metadata'] ?? ''));
              file_delete($file);
              // Clear metadatafile field.
              unset($phenotype['metadata']);
            }
          }
          $message .= '<br />Number of phenotypes in Phenotype File: <strong>@file_count</strong>.';
          form_set_error($form_field_error_name, t($message, $params));
          return;
        }
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if (form_get_errors()) {
      return;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if phenotype names in data and metadata files matches.
    if ($is_metadata_file) {
      $phenotype_names = [];
      switch ($phenotype['format']) {
        case self::FILE_FORMAT_1:
          // If there is only one column with data then it will be a string
          // but we need an array.
          $phenotype_file_name_cols = is_array($groups['Phenotype Data']['0'])
            ? $groups['Phenotype Data']['0'] : [$groups['Phenotype Data']['0']];
          $headers = tpps_file_headers($file_id, !empty($phenotype['file-no-header']));
          foreach ($phenotype_file_name_cols as $column_index) {
            $phenotype_names[] = $headers[$column_index];
          }
          break;

        case self::FILE_FORMAT_2:
          $phenotype_file_name_col = $groups['Phenotype Name/Identifier']['2'];
          $phenotype_names = tpps_parse_file_column($file_id, $phenotype_file_name_col);
          break;
      }

      // Check if phenotype names in data and metadata files matches.
      $phenotype_meta_names = [];
      $phenotype_name_col = self::getPhenotypeNameColumn($org_num, $form_state);
      if (isset($phenotype_name_col)) {
        $phenotype_meta_names = tpps_parse_file_column(
          $metadata_file_id,
          $phenotype_name_col
        );
      }
      $message = "Phenotype file: We detected "
        . "Phenotypes that were not in your Phenotype Metadata file. "
        . "Please either remove these phenotypes from your Phenotype file, "
        . "or add them to your Phenotype Metadata file. The phenotypes we "
        . "detected with missing definitions were: @list.";
    }
    else {
      // Manual metadata.
      $phenotype_meta_names = [];
      for ($i = 1; $i <= $phenotype_number; $i++) {
        $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
      }
      // @todo Update massage for manually added phenotypes.
      $message = "Phenotype file: We detected "
        . "Phenotypes that were not in your Phenotype Metadata file. "
        . "Please either remove these phenotypes from your Phenotype file, "
        . "or add them to your Phenotype Metadata file. The phenotypes we "
        . "detected with missing definitions were: @list.";
    }

    $missing_phenotypes = array_diff($phenotype_names, $phenotype_meta_names);
    if (!empty($missing_phenotypes)) {
      form_set_error($form_field_error_name,
        t($message, ['@list' => implode(', ', $missing_phenotypes)]
      ));
      return;
    }

    //
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if number of trees in data file matches number of rows in
    // accession file added on Page 3.
    if ($is_metadata_file) {
      $phenotype_file_tree_col = $groups['Tree Identifier']['1'];
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
          $file_id,
          $tree_accession_file,
          $phenotype_file_tree_col,
          $id_col_accession_name,
          $phenotype_no_header,
          $acc_no_header
        );
        if (!empty($missing_trees)) {
          form_set_error($form_field_error_name, t("Phenotype file: We "
            . "detected Plant Identifiers that were not "
            . "in your Plant Accession file. Please either remove these "
            . "plants from your Phenotype file, or add them to your Plant "
            . "Accession file. The Plant Identifiers we found were: @list.",
            ['@list' => implode(', ', $missing_trees)]
          ));
          return;
        }
      }
      else {
        // @TODO What to do if tree id column wasn't selected? Probably this
        // will never happen because earlier we check required columns and
        // tree id is a required column for both file formats.
      }
    }

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
  public static function validateIso($org_num, array &$form, array &$form_state) {
    $phenotype = &$form_state['values']["organism-$org_num"]['phenotype'] ?? NULL;
    if (empty($phenotype['iso-check']) || empty($phenotype)) {
      return;
    }

    $id = "organism-$org_num";
    $page3_values = $form_state['saved_values'][TPPS_PAGE_3] ?? NULL;
    $form_field_error_name = "$id][phenotype][iso";

    if (empty($phenotype['iso'])) {
      form_set_error($form_field_error_name,
        t("Phenotype Isotope/Mass Spectrometry File: field is required.")
      );
      return;
    }

    $headers = tpps_file_headers($phenotype['iso']);
    $id_col_name = key($headers);
    while (($k = array_search(NULL, $headers))) {
      unset($headers[$k]);
    }
    $num_columns = tpps_file_width($phenotype['iso']) - 1;
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
    $species_index = empty($page3_values['tree-accession']['check'])
      ? 'species-1' : "species-$org_num";
    $species = $page3_values['tree-accession'][$species_index];
    $tree_accession_file = $species['file'];
    $id_col_accession_name = $species['file-groups']['Tree Id']['1'];
    $acc_no_header = $species['file-no-header'];

    // Compare files.
    $missing_trees = tpps_compare_files(
      $phenotype['iso'],
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
  }

  /**
   * Gets index of the Phenotype Name column.
   *
   * @param int $id
   *   Organism ordinal number.
   * @param array $form_state
   *   Drupal Form API State array.
   *
   * @return string
   *   Returns index of the column which holds phenotype name. E.g., "A".
   */
  public static function getPhenotypeNameColumn($id, array $form_state) {
    $path = [
      "organism-$id", 'phenotype', 'metadata-groups',
      // 'Phenotype Id' is name of the column (not option's name or index)
      // which created on server side during validation.
      'Phenotype Id',
      // This value doesn't depend from file format and will always be '1'
      // for 'Phenotype Id'.
      '1',
    ];
    return drupal_array_get_nested_value($form_state['input'], $path)
      ?? drupal_array_get_nested_value($form_state['values'], $path)
      ?? drupal_array_get_nested_value($form_state['saved_values'][TPPS_PAGE_4], $path)
      ?? NULL;
  }

  /**
   * Checks if given DataType was set using column's widget.
   *
   * @param array $form_state
   *   Drupal Form API State array.
   * @param int $org_num
   *   Ordinal number of organism on page.
   * @param int $data_type_id
   *   Data type Id. See constants of this class.
   *
   * @return bool
   *   Returns TRUE if data type was set and FALSE otherwise.
   */
  public static function isDataTypeSet(array $form_state, $org_num, $data_type_id) {
    $selected_columns = TppsArray::getValue($form_state,
      ['input', "organism-$org_num", 'phenotype', 'file', 'columns']
    );
    return !empty($selected_columns) && in_array($data_type_id, $selected_columns);
  }

  /**
   * Gets CVTerm Id for phenotype's year.
   *
   * Note: cvterm 54188 (year) doesn't work at dev-server.
   * CVTerm must exist even if phenotype data file has no 'year' column.
   *
   * @return int
   *   Returns CV Term Id for 'year' from phenotype data file.
   */
  public static function getYearCvTermId() {
    try {
      $year_cvterm_id = tpps_load_cvterm('year')->cvterm_id;
    }
    catch (Exception $e) {
      // Note: cvterm 54188 (year) doesn't work at dev-server.
      $year_cvterm_id = variable_get('tpps_submitall_cvterm_phenotype_year', '');
    }
    return $year_cvterm_id ?? NULL;
  }

  /**
   * Builds Phenotype Name.
   *
   * @param string $accession
   *   Study accession. E.g., 'TGDR1234'.
   * @param string $tree_id
   *   Value of the 'self::DATA_TYPE_PLANT_IDENTIFIER' column from the data file.
   * @param mixed $name
   *   Probably phenotype name in the file.
   * @param string $suffix
   *   Suffix?
   * @param string $value_4lettercode
   *   Organism's 4-letter code.
   *
   * @return string
   *   Returns phenotype name to be used in DB.
   *   E.g., 'TGDR1034-ZYMJ10-5-Yellowing, 50%-2747-Potr'
   *   'TGDR1034-ZYMJ10-5-Leaf drop-2747-Potr',
   *
   * @todo Decrease number of arguments. Get data inside those function.
   */
  public static function buildPhenotypeName($accession, $tree_id, $name, $suffix, $value_4lettercode) {
    return "$accession-$tree_id-$name-$suffix-$value_4lettercode";
  }

  /**
   * Gets list of user specified data types for columns.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param object $submission
   *   Study Submission object.
   *
   * @return mixed
   *   Returns acciative array where keys are column letters and values are
   *   data type keys (see current class constants).
   */
  public static function getColumns($organism_index, $submission) {
    $shared_state = $submission->getSharedState();
    return $shared_state['saved_values'][TPPS_PAGE_4]['organism-'
      . $organism_index]['phenotype']['file-columns'] ?? NULL;
  }

  /**
   * Gets data type format.
   *
   * Note: Phenotype data file specific.
   * See class's consatants: FILE_FORMAT_*.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param object $submission
   *   Study Submission object.
   *
   * @return int
   *   Returns value of file format choosen by user at Page 4.
   */
  public static function getFormat($organism_index, $submission) {
    $shared_state = $submission->getSharedState();
    return $shared_state['saved_values'][TPPS_PAGE_4]['organism-'
      . $organism_index]['phenotype']['format'] ?? self::FILE_FORMAT_1;
  }

  /**
   * Checks if given column data type was set by user.
   *
   * Note: Required data types won't pass validation but optional
   * data types like 'Year' requires those check.
   *
   * @param string $column_data_type
   *   Human readable column name. E.g., 'Year'.
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param object $submission
   *   Study Submission object.
   *
   * @return bool
   *   Returns TRUE if given data type was set by user and FALSE otherwise.
   */
  public static function hasColumn($column_data_type, $organism_index, $submission) {
    $columns = self::getColumns($organism_index, $submission);
    // Get format.
    // @todo Minor. Check if method 'getFormat' exists to make this method
    // re-usable and move to parent class.
    $format = self::getFormat($organism_index, $submission);
    // Get column key.
    $options = self::getColumnOptions($format);
    $key = array_search($column_data_type, $options);
    return in_array($key, $columns);
  }

}
