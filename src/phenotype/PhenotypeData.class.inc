<?php

/**
 * @file
 * Contains 'PhenotypeData' class.
 *
 * Phenotype data file is always required. Metadata could be added manually
 * (PhenotypeManual class) or using file (PhenotypeMeta class).
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
class PhenotypeData extends TppsFile {

  const DATA_TYPE_PHENOTYPE_DATA = 0;
  const DATA_TYPE_PLANT_IDENTIFIER = 1;
  const DATA_TYPE_TIMEPOINT = 2;
  const DATA_TYPE_CLONE_NUMBER = 3;
  const DATA_TYPE_YEAR = 5;

  // Name of the checkbox which specifies that phenotype "normal" data file
  // must be used. When This checkbox is checked the normal phenotype data
  // file is required.
  // Note: there is also 'iso-check' checkbox.
  const CHECKBOX_NAME = 'normal-check';

  // Name of the file field.
  // Value is file id.
  const FIELD_NAME = 'file';
  // Name of the fieldset on PAGE 4.
  const DATA_TYPE = 'phenotype';
  // Form step where field is used.
  const FORM_STEP = 4;

  /**
   * Gets parents of the field.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return array
   *   Returns list of paretns.
   *
   * @throws Exception
   *   When child class doesn't overwrite this method.
   */
  public static function getParents($organism_index, array $form_state) {
    $organism_key = 'organism-' . $organism_index;
    switch (SubmissionVersion::get($form_state)) {
      case 1:
      case 2:
      case 3:
      default:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          static::FIELD_NAME,
        ];
        break;
    }
    return $parents;
  }

  /**
   * Gets parents for checkbox which makes field required.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   *
   * @return array
   *   Returns list of paretns.
   *
   * @throws Exception
   *   When child class doesn't overwrite this method.
   */
  public static function getCheckboxParents($organism_index) {
    return ['organism-' . $organism_index, 'phenotype', static::CHECKBOX_NAME];
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
    $page4_values = $form_state['saved_values'][TPPS_PAGE_4] ?? NULL;
    $phenotype_dir = variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');
    $normal_check = tpps_get_ajax_value(
      $form_state, ["organism-$id", 'phenotype', static::CHECKBOX_NAME],
      TRUE
    );
    if (!empty($normal_check)) {
      $states = [
        ':input[name="organism-' . $id . '[phenotype][phenotypes-meta][number]"]'
        => ['value' => '0'],
        ':input[name="organism-' . $id . '[phenotype][check]"]'
        => ['checked' => FALSE],
      ];
      $form["organism-$id"]['phenotype'][static::FIELD_NAME] = [
        '#type' => 'managed_file',
        '#title' => t('Phenotype file: Please upload a file containing '
          . 'columns for Plant Identifier, Phenotype Data: *'),
        '#upload_location' => 'public://' . $phenotype_dir,
        '#upload_validators' => ['file_validate_extensions' => ['csv tsv']],
        '#tree' => TRUE,
        '#states' => ['invisible' => $states],
        'empty' => [
          '#default_value' => $page4_values["organism-$id"]['phenotype']
            [static::FIELD_NAME]['empty'] ?? t('NA'),
        ],
        'columns' => [
          '#description' => t('Please define which columns hold the required '
            . 'data: Plant Identifier, Phenotype name, and Value(s)'
          ),
        ],
      ];
      $form["organism-$id"]['phenotype'][static::FIELD_NAME]['columns-options'] = [
        '#type' => 'hidden',
        '#value' => PhenotypeData::getColumnOptions(),
      ];
      // Ability to use file without header was disabled to make validation
      // simplier and to avoid problems with processing those files.
      // File without header will not pass validation.
      //$form["organism-$id"]['phenotype'][static::FIELD_NAME]['no-header'] = [];
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
            $form["organism-$id"]['phenotype']['time']['time_phenotypes']
              [$time_phenotype]['#default_value'] = TRUE;
          }
        }
      }
    }
  }

  /**
   * Form API validator for Phenotype data file field.
   *
   * @param int $organism_index
   *   Ordinal number of organism.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function validate($organism_index, array &$form, array &$form_state) {
    if (empty(static::isSelected($organism_index, $form_state))) {
      return;
    }
    $id = 'organism-' .$organism_index;
    $phenotype = &$form_state['values'][$id]['phenotype'] ?? NULL;
    $page3_values = $form_state['saved_values'][TPPS_PAGE_3] ?? NULL;
    // The form_state values of the phenotype fieldset for organism $id.
    // @TODO Minor. Rename to $file_form_element.
    $file_element = $form[$id]['phenotype'][static::FIELD_NAME];
    $fid = static::getFileId($organism_index, $form_state);
    $metadata_file_id = $phenotype['metadata'];
    $form_field_error_name = $id . '][phenotype][' . static::FIELD_NAME;

    $phenotype_number = $phenotype['phenotypes-meta']['number'];
    // File Id of metadata file.
    $phenotype_meta = $phenotype['metadata'];
    $fid = $phenotype[static::FIELD_NAME];

    if (empty($fid)) {
      form_set_error($form_field_error_name,
        t('Phenotype File: field is required.')
      );
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Phenotype Metafile was used.
    $is_metadata_file = (bool) ($phenotype['check'] && !empty($fid));
    if ($is_metadata_file) {
      module_load_include('inc', 'tpps', 'src/PhenotypeMeta.class');
      PhenotypeMeta::validate($organism_index, $form, $form_state);
    }
    // Manually added Phenotype Metadata. File wasn't used.
    else {
      // @TODO Move to PhenotypeManual::validate().
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      for ($i = 1; $i <= $phenotype_number; $i++) {
        $current_phenotype = &$phenotype['phenotypes-meta']["$i"];
        // [VS] Synonym form.
        if (!empty($current_phenotype['synonym_id'])) {
          $synonym_name = $current_phenotype['synonym_name'];
          $synonym_description = $current_phenotype['synonym_description'];
          if ($synonym_name == '') {
            form_set_error("$id][phenotype][phenotypes-meta][$i][synonym_name",
              "Phenotype $i Name: field is required.");
          }
          if ($synonym_description == '') {
            form_set_error("$id][phenotype][phenotypes-meta][$i][synonym_description",
              "Phenotype $i Description: field is required.");
          }
          if (!empty($current_phenotype['synonym_id'])) {
            // Restore only if there is Synonym Id.
            PhenotypeSynonym::restoreValues($current_phenotype);
          }
        }

        // [/VS]
        // Main form.
        $name = $current_phenotype['name'];
        $description = $current_phenotype['description'];
        if ($name == '') {
          form_set_error(
            $id . '][phenotype][phenotypes-meta][' . $i . '][name',
            t(
              'Phenotype @phenotype_id Name: field is required.',
              ['@phenotype_id' => $i]
            )
          );
        }
        if ($description == '') {
          form_set_error(
            $id . '][phenotype][phenotypes-meta][' . $i . '][description',
            t('Phenotype @phenotype_id Description: field is required.',
              ['@phenotype_id' => $i]
            )
          );
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Check if phenotype name matches column names in Phenotype File.
        if (!form_get_errors() && !empty($fid)) {
          $file_header = tpps_file_get_header($fid);
          if (!empty($file_header) && !in_array(($name ?? NULL), $file_header)) {
            $message = t('Phenotype @phenotype_id Name: Name '
              . '"<strong>@phenotype_name</strong>" do not match any column name '
              . 'in Phenotype File.<br />Columns in file are: @column_list.',
              [
                '@phenotype_id' => $i,
                '@phenotype_name' => $name,
                '@column_list' => implode(', ', $file_header),
              ]
            );
            form_set_error("$id][phenotype][phenotypes-meta][$i][name", $message);
          }
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Validate 'Attribute'.
        if (!$current_phenotype['attribute']) {
          form_set_error("$id][phenotype][phenotypes-meta][$i][attribute",
            "Phenotype $i Attribute: field is required.");
        }
        $condition = (
          $current_phenotype['attribute'] == 'other'
          && $current_phenotype['attr-other'] == ''
        );
        if ($condition) {
          form_set_error("$id][phenotype][phenotypes-meta][$i][attr-other",
            "Phenotype $i Custom Attribute: field is required.");
        }
        // [VS]
        $unit = $current_phenotype['unit'];
        if ($unit == '') {
          form_set_error("$id][phenotype][phenotypes-meta][$i][unit",
            "Phenotype $i Unit: field is required.");
        }
        elseif ($unit == 'other') {
          if ($current_phenotype['unit-other'] == '') {
            form_set_error("$id][phenotype][phenotypes-meta][$i][unit-other",
              "Phenotype $i Custom Unit: field is required.");
          }
          else {
            // Create a record in 'Unit Warning' table for Custom Unit.
            db_merge('tpps_phenotype_unit_warning')
              ->key(['study_name' => $form_state['accession']])
              ->fields(['study_name' => $form_state['accession']])
              ->execute();
          }
        }
        // [/VS]

        $condition = (
          $current_phenotype['structure'] == 'other'
          && $current_phenotype['struct-other'] == ''
        );
        if ($condition) {
          form_set_error("$id][phenotype][phenotypes-meta][$i][struct-other",
            "Phenotype $i Custom Structure: field is required.");
        }
      }
    }

    if ($phenotype['time']['time-check']) {
      $time = &$form_state['values'][$id]['phenotype']['time'];
      foreach ($phenotype['time']['time_phenotypes'] as $key => $val) {
        if (!$val) {
          unset($time['time_phenotypes'][$key]);
          unset($time['time_values'][$key]);
        }
      }
      if (empty($time['time_phenotypes'])) {
        form_set_error("$id][phenotype][time][time_phenotypes",
          t("Time-based Phenotypes: field is required.")
        );
      }
    }


    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if required data file was uploaded.
    if (empty($fid)) {
      $message = 'Phenotype File: field is required.';
      form_set_error($form_field_error_name, t($message));
      return;
    }
    $is_metadata_file = (bool) ($phenotype['check'] && !empty($fid));

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if required columns are present in file.
    $required_groups = [
      'Tree Identifier' => ['id' => [self::DATA_TYPE_PLANT_IDENTIFIER]],
      'Phenotype Data' => ['phenotype-data' => [self::DATA_TYPE_PHENOTYPE_DATA]],
    ];
    $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
    if (form_get_errors()) {
      return;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if number of rows in data file matches number of phenotypes in
    // metadata file.
    $file_header = tpps_file_get_header($fid);
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
      // Note:
      // self::DATA_TYPE_PHENOTYPE_DATA is zero so array_filter() could be used.
      $file_phenotypes_count = count($file_element['#value']['columns'])
        - count(array_filter($file_element['#value']['columns']));

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
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if (form_get_errors()) {
      return;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if phenotype names in data and metadata files matches.
    $phenotype_names = [];
    // If there is only one column with data then it will be a string
    // but we need an array.
    $phenotype_file_name_cols = is_array($groups['Phenotype Data']['0'])
      ? $groups['Phenotype Data']['0'] : [$groups['Phenotype Data']['0']];
    $headers = tpps_file_headers($fid, !empty($phenotype['file-no-header']));
    foreach ($phenotype_file_name_cols as $column_index) {
      $phenotype_names[] = $headers[$column_index];
    }
    $phenotype_meta_names = [];

    if ($is_metadata_file) {
      // Check if phenotype names in data and metadata files matches.
      $phenotype_name_col = self::getPhenotypeNameColumn($organism_index, $form_state);
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
      for ($i = 1; $i <= $phenotype_number; $i++) {
        $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
      }
      // @todo Update massage for manually added phenotypes.
      $message = "Phenotype file: We detected "
        . "Phenotypes that are missing in your Phenotype data file. "
        . "Please either remove these phenotypes from your Phenotype file, "
        . "or add those Phenotypes manually. The phenotypes we "
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
        $species_index = empty($page3_values['tree-accession']['check'])
          ? 'species-1' : 'species-' . $organism_index;
        // Note: 'file' is the name of Accession file field.
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
          $fid,
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
        // tree id is a required column.
      }
    }

    if (!form_get_errors()) {
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
    // WARNING: Use explicit keys to avoid errors when order changed.
    return [
      self::DATA_TYPE_PHENOTYPE_DATA => 'Phenotype Data',
      self::DATA_TYPE_PLANT_IDENTIFIER => 'Plant Identifier',
      self::DATA_TYPE_TIMEPOINT => 'Timepoint',
      self::DATA_TYPE_CLONE_NUMBER => 'Clone Number',
      self::DATA_TYPE_YEAR => 'Year',
      '4' => 'N/A',
    ];
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
  public static function getPhenotypeNameColumn($organism_index, array $form_state) {
    $parents = [
      'organism-' . $organism_index, 'phenotype', 'metadata-groups',
      // 'Phenotype Id' is name of the column (not option's name or index)
      // which created on server side during validation.
      'Phenotype Id',
      // This value will always be '1' for 'Phenotype Id'.
      '1',
    ];
    return drupal_array_get_nested_value($form_state['input'], $parents)
      ?? drupal_array_get_nested_value($form_state['values'], $parents)
      ?? drupal_array_get_nested_value(
        $form_state['saved_values'][TPPS_PAGE_4],
        $parents
      ) ?? NULL;
  }

  /**
   * Checks if given DataType was set using column's widget.
   *
   * WARNING: only for validation because uses $form_state['input'].
   *
   * @param array $form_state
   *   Drupal Form API State array.
   * @param int $organism_index
   *   Ordinal number of organism on page.
   * @param int $data_type_id
   *   Data type Id. See constants of this class.
   *
   * @return bool
   *   Returns TRUE if data type was set and FALSE otherwise.
   */
  public static function isDataTypeSet(array $form_state, $organism_index, $data_type_id) {
    $selected_columns = TppsArray::getValue($form_state['input'],
      ['organism-' . $organism_index, 'phenotype', static::FIELD_NAME, 'columns']
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
    return $submission->sharedState['saved_values'][TPPS_PAGE_4]['organism-'
      . $organism_index]['phenotype']['file-columns'] ?? NULL;
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
    $options = self::getColumnOptions();
    $key = array_search($column_data_type, $options);
    return in_array($key, $columns);
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
  }

}
