<?php

/**
 * @file
 * Contains 'PhenotypeMeta' class.
 *
 * The main reason to have PhenotypeMeta class is that each organism could have
 * multiple number of phenotypes so form fields must be processed in a
 * different way.
 */

/**
 * Manages fields to manually add phenotypes on TPPS Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PhenotypeMeta {

  // Form step where field is used.
  const FORM_STEP = 4;

  // Keys for Phenotype Metadata file columns (data types) options.
  // See PhenotypeMeta::getColumnOptions().
  const DATA_TYPE_IDENTIFIER = 1;
  const DATA_TYPE_ATTRIBUTE = 2;
  const DATA_TYPE_DESCRIPTION = 3;
  const DATA_TYPE_UNIT = 4;
  const DATA_TYPE_STRUCTURE = 5;
  const DATA_TYPE_MINIMUM_VALUE = 6;
  const DATA_TYPE_MAXIMUM_VALUE = 7;
  const DATA_TYPE_IS_ENVIRONMENTAL = 8;

  /**
   * Builds list of options for phenotype file data type selector (columns).
   *
   * @param int $key
   *   Key of the item in the list. When non-empty name of the option could be
   *   obtained.
   *
   * @return mixed
   *   Returns list of options when $key is empty.
   *   Returns name of the option when $key is not empty and exists in the list
   *   of options.
   */
  public static function getColumnOptions($key = NULL) {
    $list = [
      0 => t('N/A'),
      self::DATA_TYPE_IDENTIFIER       => t('Phenotype Name/Identifier'),
      self::DATA_TYPE_ATTRIBUTE        => t('Attribute'),
      self::DATA_TYPE_DESCRIPTION      => t('Description'),
      self::DATA_TYPE_UNIT             => t('Unit'),
      self::DATA_TYPE_STRUCTURE        => t('Structure'),
      self::DATA_TYPE_MINIMUM_VALUE    => t('Minimum Value'),
      self::DATA_TYPE_MAXIMUM_VALUE    => t('Maximum Value'),
      self::DATA_TYPE_IS_ENVIRONMENTAL => t('Environmental Phenotype'),
    ];
    return (!is_null($key) && isset($list[$key])) ? $list[$key] : $list;
  }

  /**
   * Gets list of required columns of phenotype metadata file.
   *
   * This list used to validate file. See tpps_validate_phenotype().
   *
   * @return array
   *   Returns list of required columns in format:
   *   <Group Name> => [<key> => [<data type key in the columns list>]].
   */
  public static function getRequiredColumnList() {
    return [
      'Phenotype Id' => ['id' => [self::DATA_TYPE_IDENTIFIER]],
      'Attribute' => ['attr' => [self::DATA_TYPE_ATTRIBUTE]],
      'Description' => ['desc' => [self::DATA_TYPE_DESCRIPTION]],
      'Unit' => ['unit' => [self::DATA_TYPE_UNIT]],
      'Structure' => ['structure' => [self::DATA_TYPE_STRUCTURE]],
    ];
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
    $organism = &$form_state['values'][$organism_key] ?? NULL;
    $phenotype = &$organism['phenotype'] ?? NULL;

    $phenotype_number = $phenotype['phenotypes-meta']['number'];
    // File Id of metadata file.
    $meta_fid = $phenotype['metadata'];
    $data_fid = $phenotype['file'];
    if (empty($data_fid)) {
      form_set_error($organism_key . "][phenotype][file",
        t('Phenotype File: field is required.')
      );
    }

    // Clear manually added metadata to show correct data at edit page.
    if (!empty($phenotype['phenotypes-meta']['number'])) {
      for ($i = 1; $i <= $phenotype_number; $i++) {
        unset($phenotype['phenotypes-meta'][$i]);
      }
      $phenotype['phenotypes-meta']['number'] = 0;
    }
    if (empty($phenotype['metadata'])) {
      // $phenotype['metadata'] holds File Id of Phenotype Metadata file.
      // @TODO Use:
      // tpps_form_error_required($form_state, [$organism_key, 'phenotype', 'metadata']);
      form_set_error($organism_key . "][phenotype][metadata",
        t('Phenotype Metadata File: field is required.')
      );
    }
    else {
      // Check if all required columns was set.
      $file_element = $form[$organism_key]['phenotype']['metadata'];
      $groups = tpps_file_validate_columns(
        $form_state,
        PhenotypeMeta::getRequiredColumnList(),
        $file_element
      );
      // Not all required columns was set. Can't proceed validation.
      if (form_get_errors()) {
        return;
      }
      // Get phenotype name column.
      $phenotype_name_col = $groups['Phenotype Id']['1'];
      // Preserve file if it is valid.
      tpps_preserve_valid_file(
        $form_state,
        $form_state['values'][$organism_key]['phenotype']['metadata'],
        $organism_index,
        "Phenotype_Metadata"
      );
    }

    $groups = $phenotype['metadata-groups'] ?? NULL;
    if (empty($groups)) {
      form_set_error($organism_key . "][phenotype][metadata",
        t("Phenotype Metadata File: No groups found.")
      );
    }
    else {
      // Empty units in metadata file are not allowed.
      $columns = [
        'name' => $groups['Phenotype Id'][self::DATA_TYPE_IDENTIFIER],
        'attr' => $groups['Attribute'][self::DATA_TYPE_ATTRIBUTE],
        'desc' => $groups['Description'][self::DATA_TYPE_DESCRIPTION],
        'unit' => $groups['Unit'][self::DATA_TYPE_UNIT],
      ];
      $options = [
        'meta_columns' => $columns,
        'id' => $organism_key,
        'selected' => static::getColumnDataTypes($form_state, $organism_index),
      ];
      tpps_file_iterator($meta_fid, 'PhenotypeMeta::validateRow', $options);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Validate if metafile matches phenotype file using columns.
      // Get Phenotype Data header which could have 'year' column.
      $datafile_header = tpps_file_get_header($data_fid);
      if ($datafile_header) {
        // $form_state['values'] has no value but $form_state['input']
        // has is so we will use it.
        $datafile_no_header = $form_state['input'][$organism_key]['phenotype']
          ['file']['no-header'] ?? NULL;
        // Check if checkbox 'My file has no header row' wasn't checked.
        if ($datafile_no_header) {
          // @TODO Check if phenotype file could be without header?
          drupal_set_message(
            t('Phenotype file has no header in 1st row so validation was NOT'
            . ' completed. Please check if Phenotype Names in both files are'
            . ' the same.'),
            'warning'
          );
        }
        else {
          // Check if list of phenotype names from metafile matches column names
          // in phenotype file.
          // $datafile_header contains list of phenotype file column names.
          $options = [
            'columns' => [$columns['name']],
            // List of phenotype names in datafile stored in header.
            'phenotypes' => $datafile_header,
            'organism_name' => $organism_key,
            'column_name' => $columns['name'],
          ];
          tpps_file_iterator(
            $meta_fid,
            'tpps_validate_metafile_phenotype_names',
            $options
          );
        }
      }
    }
  }

  /**
   * Validates single row of the metadata file.
   *
   * @param mixed $row
   *   The item yielded by the TPPS file generator.
   * @param array $options
   *   Additional options set when calling tpps_file_iterator().
   */
  public static function validateRow($row, array &$options = []) {
    $form_error_name = $options['id'] . '][phenotype][metadata';
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if 'unit' column has empty values.
    $columns = $options['meta_columns'];
    if (empty($row[$columns['unit']])) {
      $message = t('Phenotype Metadata File: Empty unit not allowed for '
        . '"<strong>@phenotype_name</strong>".<br />Row: @row',
        [
          '@row' => implode(', ', $row),
          '@phenotype_name' => $phenotype_name,
        ]
      );
      form_set_error($form_error_name, $message);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Validate synonyms.
    if (
      variable_get('tpps_page_4_phenotype_metadata_validate_synonyms')
    ) {
      // See also Submission->getPhenotypeSynonymList().
      $attribute = $row[array_search(PhenotypeMeta::DATA_TYPE_ATTRIBUTE,
        $options['selected']
      )];
      $phenotype_name = $row[array_search(PhenotypeMeta::DATA_TYPE_IDENTIFIER,
        $options['selected']
      )];
      $synonym_id = array_search($attribute, PhenotypeSynonym::getAll());
      if (empty($synonym_id)) {
        $message = t('Phenotype Metadata File: Phenotype synonym is missing for '
          . '"<strong>@phenotype_name</strong>".<br />Row: @row',
          [
            '@row' => implode(', ', $row),
            '@phenotype_name' => $phenotype_name,
          ]
        );
        form_set_error($form_error_name, $message);
      }
    }
  }

  /**
   * Processes single row from file.
   *
   * Must be used with tpps_file_iterator().
   *
   * @param mixed $row
   *   The item yielded by the TPPS file generator.
   * @param array $options
   *   Additional options set when calling tpps_file_iterator().
   */
  public static function processRow($row, array &$options = []) {
    $column_list = $options['meta_columns'];
    $meta = &$options['meta'];

    if (empty($name = strtolower($row[$column_list['name']]))) {
      tpps_log('Empty phenotype "@name".', ['@name' => $name], TRIPAL_ERROR);
      return;
    }
    // Note: $meta[$name] is not empty and filled with correct data when
    // submit_all.php script runs.
    // @todo Why it should be overwritten here?
    $meta[$name]['attr'] = 'other';
    $meta[$name]['attr-other'] = $row[$column_list['attr']];
    $meta[$name]['desc'] = $row[$column_list['desc']];
    $meta[$name]['unit'] = 'other';
    $meta[$name]['unit-other'] = $row[$column_list['unit']];
    $meta[$name]['env'] = $row[$column_list['env']];
    // $name is in lower case but we need original phenotype name.
    $meta[$name]['name'] = $row[$column_list['name']];

    if (
      !empty($column_list['struct'])
      && isset($row[$column_list['struct']])
      && $row[$column_list['struct']] != ''
    ) {
      $meta[$name]['struct'] = 'other';
      $meta[$name]['struct-other'] = $row[$column_list['struct']];
    }

    // Search for synonym.
    $table = 'chado.phenotype_synonyms';
    $synonym_id = db_select($table, 't')
      ->fields('t', ['phenotype_synonyms_id'])
      ->condition('synonym', strtolower($row[$column_list['attr']]))
      ->execute()->fetchField();
    if (empty($synonym_id)) {
      tpps_log('No synonym found for phenotype "@name".', ['@name' => $name], TRIPAL_WARNING);
      return;
    }
    tpps_log('Found synonym #@synonym_id for phenotype "@name".',
      ['@name' => $name, '@synonym_id' => $synonym_id], TRIPAL_DEBUG
    );
    $meta[$name]['synonym_id'] = $synonym_id;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Process 'is environmental phenotype' column.
    // Note: synonym_id is required.
    if (!isset($column_list['env'])) {
      tpps_log("No 'Environmental Phenotype' in Phenotype Metadata.", [], TRIPAL_DEBUG);
      return;
    }
    // Store 'Environmental Phenotype' value.
    db_update($table)
      ->fields([
        'is_environmental_phenotype' => (empty($row[$column_list['env']]) ? 0 : 1),
      ])
      ->condition('phenotype_synonyms_id', $synonym_id)
      ->execute();
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
   * Gets list of the selected column's data types.
   *
   * @param array $form_state
   *   Drupal Form State array.
   * @param int $organism_index
   *   Ordinal number of organism at page.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function getColumnDataTypes(array $form_state, $organism_index) {
    $parents = [
      'saved_values',
      TPPS_PAGE_4,
      'organism-' . $organism_index,
      'phenotype',
      'metadata-columns',
    ];
    return tpps_array_get_value($form_state, $parents);
  }

  /**
   * Gets value of the 'no-header' sub-field.
   *
   * @param array $form_state
   *   Drupal Form State array.
   * @param int $organism_index
   *   Ordinal number of organism at page.
   *
   * @return mixed
   *   Returns value or NULL.
   */
  public static function getNoHeader(array $form_state, $organism_index) {
    $parents = [
      'saved_values',
      TPPS_PAGE_4,
      'organism-' . $organism_index,
      'phenotype',
      'metadata-no-header',
    ];
    return tpps_array_get_value($form_state, $parents);
  }

  /**
   * Get column from file by column key.
   *
   * @param array $form_state
   *   Drupal Form State array.
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param int $column_key
   *   Optional. Column integer key (see class constants DATA_TYPE_*).
   *
   * @return array
   *   Returns values from the column.
   */
  public static function getColumn(array $form_state, $organism_index, $column_key = NULL) {
    $column_key = $column_key ?? static::DATA_TYPE_IDENTIFIER;
    $organism_key = 'organism-' . $organism_index;
    // @todo Replace with PhenotypeMeta::getFileId() when branch
    // VS/assay_design_column_data_type will be merged.
    $fid = tpps_array_get_value($form_state,
      ['saved_values', TPPS_PAGE_4, $organism_key, 'phenotype', 'metadata']
    );
    $selected = static::getColumnDataTypes($form_state, $organism_index);
    $no_header = static::getNoHeader($form_state, $organism_index);
    // Alphabetical key of the columns with phenotype names.
    $column_letter = array_search($column_key, $selected);
    $column = tpps_parse_file_column($fid, $column_letter, $no_header);
    if ($no_header) {
      // @todo use TppsArray::removeFirst().
      reset($column);
      array_shift($column);
    }
    return $column;
  }

}
