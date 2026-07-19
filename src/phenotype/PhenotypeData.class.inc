<?php

/**
 * @file
 * Contains 'PhenotypeData' class.
 *
 * Phenotype data file is always required. Metadata could be added manually
 * (PhenotypeManual class) or using file (PhenotypeMeta class).
 *
 * Values of column 'year' will be stored in table 'chado.phenotypeprop'.
 * See CvTerm::getId('year') for more details.
 */

/**
 * Manages Phenotype Data file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PhenotypeData extends FileField {

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
  const TITLE = 'Phenotype';
  // Name of the file field.
  // Value is file id.
  const DOM_NAME = 'file';
  // Name of the fieldset on PAGE 4.
  const DATA_TYPE = 'phenotype';
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
      case 2:
      case 3:
      default:
        $parents = [
          static::DATA_TYPE,
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
    $organism_key = 'organism-' . $organism_index;
    return [
      $organism_key,
      'phenotype',
      static::CHECKBOX_NAME,
    ];
  }

  /**
   * Builds form fields for Phenotype Data file.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($organism_index, array &$form, array $form_state) {
    $organism_key = 'organism-' . $organism_index;
    $page4_values = $form_state['saved_values'][TPPS_PAGE_4] ?? NULL;
    $phenotype_dir = variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');
    $normal_check = tpps_get_ajax_value(
      $form_state, [$organism_key, 'phenotype', static::CHECKBOX_NAME],
      TRUE
    );
    if (!empty($normal_check)) {
      $states = [
        ':input[name="' . $organism_key . '[phenotype][phenotypes-meta][number]"]'
        => ['value' => '0'],
        ':input[name="' . $organism_key . '[phenotype][check]"]'
        => ['checked' => FALSE],
      ];
      $form[$organism_key]['phenotype'][static::DOM_NAME] = [
        '#type' => 'managed_file',
        '#title' => t('Phenotype file: Please upload a file containing '
          . 'columns for Plant Identifier, Phenotype Data: *'),
        '#upload_location' => 'public://' . $phenotype_dir,
        '#upload_validators' => ['file_validate_extensions' => ['csv tsv']],
        '#tree' => TRUE,
        '#states' => ['invisible' => $states],
        'empty' => [
          '#default_value' => $page4_values[$organism_key]['phenotype']
            [static::DOM_NAME]['empty'] ?? t('NA'),
        ],
        'columns' => [
          '#description' => t('Please define which columns hold the required '
            . 'data: Plant Identifier, Phenotype name, and Value(s)'
          ),
        ],
      ];
      $form[$organism_key]['phenotype'][static::DOM_NAME]['columns-options'] = [
        '#type' => 'hidden',
        '#value' => PhenotypeData::getColumnOptions(),
      ];
      // Ability to use file without header was disabled to make validation
      // simplier and to avoid problems with processing those files.
      // File without header will not pass validation.
      //$form[$organism_key]['phenotype'][static::DOM_NAME]['no-header'] = [];
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // This will check if there are Time Phenotypes saved from the
    // saved values and use this to re-check the checkboxes on the form.
    if (isset($page4_values[$organism_key]['phenotype']['time']['time_phenotypes'])) {
      $time_phenotypes = $page4_values[$organism_key]['phenotype']['time']['time_phenotypes'] ?? [];
      $count_time_phenotypes = count($time_phenotypes);
      if ($count_time_phenotypes > 0) {
        foreach ($time_phenotypes as $time_phenotype) {
          // If time_phenotype is not 0, meaning the phenotype name
          // and phenotype value would be the same, so not 0.
          if ($time_phenotype != "0") {
            // Adjust the checkbox element on the form to TRUE (checkbox).
            $form[$organism_key]['phenotype']['time']['time_phenotypes']
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
    $organism_key = 'organism-' . $organism_index;
    $phenotype = &$form_state['values'][$organism_key]['phenotype'] ?? NULL;
    $page3_values = $form_state['saved_values'][TPPS_PAGE_3] ?? NULL;
    // The form_state values of the phenotype fieldset for organism $id.
    // @TODO Minor. Rename to $file_form_element.
    $file_element = $form[$organism_key]['phenotype'][static::DOM_NAME];
    $fid = static::getFileId($organism_index, $form_state);
    $metadata_file_id = $phenotype['metadata'];
    $form_field_error_name = $organism_key . '][phenotype][' . static::DOM_NAME;

    $phenotype_number = $phenotype['phenotypes-meta']['number'];
    // File Id of metadata file.
    $phenotype_meta = $phenotype['metadata'];
    $fid = $phenotype[static::DOM_NAME];

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
            form_set_error($organism_key . "][phenotype][phenotypes-meta][$i][synonym_name",
              "Phenotype $i Name: field is required.");
          }
          if ($synonym_description == '') {
            form_set_error($organism_key . "][phenotype][phenotypes-meta][$i][synonym_description",
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
            $organism_key . '][phenotype][phenotypes-meta][' . $i . '][name',
            t(
              'Phenotype @phenotype_id Name: field is required.',
              ['@phenotype_id' => $i]
            )
          );
        }
        if ($description == '') {
          form_set_error(
            $organism_key . '][phenotype][phenotypes-meta][' . $i . '][description',
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
            form_set_error($organism_key . "][phenotype][phenotypes-meta][$i][name", $message);
          }
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Validate 'Attribute'.
        if (!$current_phenotype['attribute']) {
          form_set_error($organism_key . "][phenotype][phenotypes-meta][$i][attribute",
            "Phenotype $i Attribute: field is required.");
        }
        $condition = (
          $current_phenotype['attribute'] == 'other'
          && $current_phenotype['attr-other'] == ''
        );
        if ($condition) {
          form_set_error($organism_key . "][phenotype][phenotypes-meta][$i][attr-other",
            "Phenotype $i Custom Attribute: field is required.");
        }
        // [VS]
        $unit = $current_phenotype['unit'];
        if ($unit == '') {
          form_set_error($organism_key . "][phenotype][phenotypes-meta][$i][unit",
            "Phenotype $i Unit: field is required.");
        }
        elseif ($unit == 'other') {
          if ($current_phenotype['unit-other'] == '') {
            form_set_error($organism_key . "][phenotype][phenotypes-meta][$i][unit-other",
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
          form_set_error($organism_key . "][phenotype][phenotypes-meta][$i][struct-other",
            "Phenotype $i Custom Structure: field is required.");
        }
      }
    }

    if ($phenotype['time']['time-check']) {
      $time = &$form_state['values'][$organism_key]['phenotype']['time'];
      foreach ($phenotype['time']['time_phenotypes'] as $key => $val) {
        if (!$val) {
          unset($time['time_phenotypes'][$key]);
          unset($time['time_values'][$key]);
        }
      }
      if (empty($time['time_phenotypes'])) {
        form_set_error($organism_key . "][phenotype][time][time_phenotypes",
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
    // @todo Use FileField::validateColumns().
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
            $file = TppsFile::load(($phenotype['metadata'] ?? ''));
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if phenotype names in data and metadata files matches.
    if ($is_metadata_file) {
      $phenotype_meta_names = PhenotypeMeta::getPhenotypeNames(
        $organism_index, $form_state
      );
      $message = "Phenotype file: We detected "
        . "Phenotypes that were not in your Phenotype Metadata file. "
        . "Please either remove these phenotypes from your Phenotype file, "
        . "or add them to your Phenotype Metadata file. The phenotypes we "
        . "detected with missing definitions were: @list.";
    }
    else {
      // Manual metadata.
      $phenotype_meta_names = PhenotypeManual::getPhenotypeNames(
        $organism_index, $form_state
      );
      // @todo Update massage for manually added phenotypes.
      $message = "Phenotype file: We detected "
        . "Phenotypes that are missing in your Phenotype data file. "
        . "Please either remove these phenotypes from your Phenotype file, "
        . "or add those Phenotypes manually. The phenotypes we "
        . "detected with missing definitions were: @list.";
    }

    // Note: PhenotypeData::matchColumn() couldn't be used because phenotype
    // names are in file's header.
    // Note: phenotypes in Metadata file are in column.
    // See SnpAssociation class which compares header and column.
    // @todo Reuse existing code.
    //
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

  /**
   * Checks if "Iso" checkbox was selected.
   *
   * Checkbox title:
   * "My phenotypes include results from a mass spectrometry or isotope analysis".
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns TRUE if checkbox was selected and FALSE otherwise.
   *   Returns NULL if field wasn't found.
   */
  public static function isSelected($organism_index, array $form_state) {
    $parents = static::getCheckboxParents($organism_index);
    $key_exists = NULL;
    if (!empty($form_state['values'])) {
      $value = TppsArray::get(
        $form_state['values'],
        $parents,
        $key_exists
      );
      if ($key_exists) {
        return $value;
      }
    }
    if (!empty($form_state['saved_values'][TPPS_PAGE_4])) {
      $value = TppsArray::get(
        $form_state['saved_values'][TPPS_PAGE_4],
        $parents,
        $key_exists
      );
      if ($key_exists) {
        return $value;
      }
    }
    return NULL;
  }

}
