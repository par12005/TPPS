<?php

/**
 * @file
 * Contains 'SnpAssociation' class.
 */

/**
 * Manages SNP Association file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SnpAssociation {

  // Data types for 'SNP Association' field columns.
  const DATA_TYPE_SNP_ID = 1;
  const DATA_TYPE_SCAFFOLD = 2;
  const DATA_TYPE_POSITION = 3;
  const DATA_TYPE_ALLELE = 4;
  const DATA_TYPE_ASSOCIATED_TRAIT = 5;
  const DATA_TYPE_CONFIDENCE_VALUE = 6;
  const DATA_TYPE_GENE_ID = 7;
  const DATA_TYPE_ANNOTATION = 8;
  const DATA_TYPE_YEAR = 9;

  // CV if of the 'PATO'.
  // @todo Get it dynamically.
  const CV_ID_PATO = 105;

  /**
   * Gets list of column data types.
   *
   * @return array
   *   Returns list of options with column data types.
   */
  public static function getColumnOptions() {
    return [
      self::DATA_TYPE_SNP_ID => 'SNP ID',
      self::DATA_TYPE_SCAFFOLD => 'Scaffold',
      self::DATA_TYPE_POSITION => 'Position',
      self::DATA_TYPE_ALLELE => 'Allele',
      self::DATA_TYPE_ASSOCIATED_TRAIT => 'Associated Trait',
      self::DATA_TYPE_CONFIDENCE_VALUE => 'Confidence Value',
      self::DATA_TYPE_GENE_ID => 'Gene ID',
      self::DATA_TYPE_ANNOTATION => 'Annotation',
      // Option 'year' must be shown only when Phenotype data file has 'year'
      // column. Since phenotype data file could be added/removed at the same
      // page beside 'SNP Association' file field we add this option at
      // server side and will add/remove it on client's side (in browser).
      self::DATA_TYPE_YEAR => 'Year',
    ];
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
    $page4_values = $form_state['saved_values']['4'];
  }

  /**
   * Validates file field.
   *
   * @param int $org_num
   *   Ordinal organism number on page.
   * @param array $form
   *   Drupal Form API array.
   * @param array $form_state
   *   Drupal Form API State array.
   */
  public static function validate($org_num, array &$form, array &$form_state) {
    $genotype = &$form_state['values']["organism-$org_num"]['genotype'] ?? NULL;
    $snps_fieldset = 'SNPs';
    $snps = &$genotype[$snps_fieldset] ?? NULL;
    $snps_assay = $snps['snps-assay'] ?? 0;
    $fid = $snps['snps-association'] ?? 0;
    $file_type = $snps['file-type'] ?? NULL;
    $id = "organism-$org_num";

    if (
      form_get_errors()
      || $file_type != TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
      // No file uploaded.
      || empty($fid)
    ) {
      return;
    }

    // @todo Minor. Implement ability to run this code when study was already
    // submitted (on processing step).
    $required_groups = [
      'SNP ID' => ['id' => [self::DATA_TYPE_SNP_ID]],
      'Scaffold' => ['scaffold' => [self::DATA_TYPE_SCAFFOLD]],
      'Position' => ['position' => [self::DATA_TYPE_POSITION]],
      'Allele' => ['allele' => [self::DATA_TYPE_ALLELE]],
      'Associated Trait' => ['trait' => [self::DATA_TYPE_ASSOCIATED_TRAIT]],
      'Confidence Value' => ['confidence' => [self::DATA_TYPE_CONFIDENCE_VALUE]],
      // Optional.
      // 7 => 'Gene ID',
      // 8 => 'Annotation',
      // Conditional. Required when Phenotype data file has 'Year'column.
      // 9 => 'Year'.
    ];

    if (PhenotypeData::isDataTypeSet($form_state, $org_num, PhenotypeData::DATA_TYPE_YEAR)) {
      $required_groups['Year'] = ['year' => [self::DATA_TYPE_YEAR]];
    }

    $file_element = $form[$id]['genotype'][$snps_fieldset]['snps-association'];
    $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

    if (form_get_errors()) {
      return;
    }

    // Check that SNP IDs match Genotype Assay.
    $snps_id_col = $groups['SNP ID'][self::DATA_TYPE_SNP_ID];
    $assoc_no_header = $snps['snps-association-no-header'] ?? FALSE;

    $assay_snps = tpps_file_headers($snps_assay);
    unset($assay_snps[key($assay_snps)]);
    $assoc_snps = tpps_parse_file_column($fid, $snps_id_col, $assoc_no_header);
    $missing_snps = array_diff($assoc_snps, $assay_snps);

    if (!empty($missing_snps)) {
      form_set_error("$id][genotype][$snps_fieldset][snps-association",
        t("SNP Association File: We detected SNP IDs that were not in "
        . "your Genotype Assay. Please either remove these SNPs from "
        . "your Association file, or add them to your Genotype Assay. "
        . "The SNP Identifiers we found were: @list",
        ['@list' => implode(', ', $missing_snps)]));
    }

    // Check that Phenotype names match phenotype metadata section.
    $trait_id_col = $groups['Associated Trait'][self::DATA_TYPE_ASSOCIATED_TRAIT];
    $association_phenotypes = tpps_parse_file_column(
      $fid, $trait_id_col, $assoc_no_header
    );

    $phenotype = $form_state['values'][$id]['phenotype'];
    $phenotype_meta = $phenotype['metadata'];
    $phenotype_number = $phenotype['phenotypes-meta']['number'];

    $phenotype_meta_names = [];
    $phenotype_name_col = PhenotypeData::getPhenotypeNameColumn($org_num, $form_state);
    if (isset($phenotype_name_col)) {
      $phenotype_meta_names = tpps_parse_file_column($phenotype_meta, $phenotype_name_col);
    }

    for ($i = 1; $i <= $phenotype_number; $i++) {
      $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
    }

    $missing_phenotypes = array_diff($association_phenotypes, $phenotype_meta_names);
    if (!empty($missing_phenotypes)) {
      form_set_error("$id][genotype][$snps_fieldset][snps-association",
        t("SNP Association File: We detected Associated Traits that were "
        . "not specified in the Phenotype Metadata Section. Please "
        . "either remove these Traits from your Association file, "
        . "or add them to your Phenotype Metadata section. The Trait "
        . "names we foud were: @list",
        ['@list' => implode(', ', $missing_phenotypes)])
      );
    }

    // Check that position values are correctly formatted.
    $position_col = $groups['Position'][self::DATA_TYPE_POSITION];
    $positions = tpps_parse_file_column($fid, $position_col, $assoc_no_header);
    foreach ($positions as $position) {
      if (
        // Numeric (integer).
        is_numeric($position)
        // Format: 'start:stop'.
        || preg_match('/^(\d+):(\d+)$/', $position)
      ) {
        continue;
      }
      form_set_error("$id][genotype][$snps_fieldset][snps-association",
        t('SNP Association File: We detected SNP positions that do '
          . 'not match the required format: @position.'
          . '<br />The correct format is: "start:stop" or integer.',
          ['@position' => $position]
        )
      );
      break;
    }
    if (form_get_errors()) {
      return;
    }
    $options = [
      'associations_groups' => $groups,
      'id' => $id,
      'column_list' => $snps['snps-association-columns'],
    ];
    tpps_file_iterator($fid, 'SnpAssociation::validateRow', $options);

    // Preserve file if it is valid.
    tpps_preserve_valid_file($form_state, $fid, $org_num, "SNPs_Association");
  }

  /**
   * Stores single value of the 'Year' column.
   */
  public static function storeYear($row, array &$options = []) {
    $groups = $options['associations_groups'];
    if (empty($groups)) {
      return;
    }
    $debug_mode = FALSE;
    // Get genotype variant name.
    // Format: $genotype_name = "$marker-$variant_name-$species_code-$val";
    $name = explode('-', $options['genotype_name']);
    // 2nd item (index is '1') called 'variant_name'.
    if (!is_array($name) || count($name) != 4 || empty($name[1])) {
      return;
    }

    // While $row has values, $named_row has keys for each value.
    $named_row = self::getNamedRow($row, $options);
    if ($named_row['id'] != $name[1]) {
      // This is another genotype.
      return;
    }

    // Extra check just to be sure that we have non-empty 'year' field value.
    $year = $named_row['year'];
    $confidence = $named_row['confidence'];
    $associated_trait = $named_row['trait'];

    if ($debug_mode) {
      tpps_log($named_row, 'named row');
      tpps_log(print_r(array_keys($options, 1)), __METHOD__ . ' options');
      tpps_log($row, __METHOD__ . ', row:');
      tpps_log($groups, __METHOD__ . ' groups');
      //tpps_log($options, __METHOD__ . ' options');
      tpps_log('Year: ' . $year . '; Confidencs:' . $confidence);
      tpps_log($options['genotype_id'], 'genotype_id');
      tpps_log($options['genotype_name'], 'genotype_name');
    }

    // Note: Other way to get genotype id.
    //
    // The information on the SNP and genotype should be used to get the
    // correct genotype id
    // 1. Get the marker name from feature.uniquename
    // 2. Using marker_id get the genotype ids from that using feature_genotype.
    // 3. Use genotype.description to match allele read)
    // which will then be stored in the table I made.


    // Get phenotype synonym id.
    // I wrote down (at Scrum Meeting) to
    // use 'Associated trait' (from SNP Association file)
    // to get Phenotype name
    // (from Phenotype Metadata file) and then use it's 'Attribute'
    // to get 'Phenotype Synonym Id'. Should I use new way you've described above or this one?

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get synonym id.
    // Note: Other way is to parse Phenotype Meta file.
    $query = db_select('chado.phenotype_synonyms', 'ps')
      ->fields('ps', ['phenotype_synonyms_id']);
    $query->join('chado.cvterm', 'ct', 'ct.cvterm_id = ps.attribute_id');
    // 'Associated trait' column from SNP Association file).
    $query->condition('ct.name', $named_row['trait']);
    $query->condition('ct.cv_id', self::CV_ID_PATO);
    $phenotype_synonyms_id = $query->range(0, 1)->execute()->fetchField();
    if (empty($phenotype_synonyms_id)) {
      // Synonym wasn't found.
      return;
    }

    $record = [
      'genotype_id' => $options['genotype_id'],
      'phenotype_synonyms_id' => $phenotype_synonyms_id,
      'year' => $year,
      'confidence' => $confidence,
    ];
    // @TODO Minor. Remove records when study removed.
    $table = 'genotype_phenotype_snp_association';
    if (variable_get('tpps_snp_association_use_chado', FALSE)) {
      tpps_chado_insert_record($table, $record);
    }
    else {
      db_insert('chado.' . $table)->fields($record)->execute();
    }
    tpps_log($record, 'Inserted');
  }

  /**
   * Validates single row of file.
   *
   * @param mixed $row
   *   The item yielded by the TPPS file generator.
   * @param array $options
   *   Additional options set when calling tpps_file_iterator().
   *   Required keys:
   *   'groups' - list of required columns.
   *   'id' - string e.g., 'organism-1'.
   */
  public static function validateRow($row, array &$options = []) {
    $name = $options['id'] . '][genotype][snps-association';
    self::getNamedRow($row, $options);
    $associations = $options['associations'];
    $column_list = $options['column_list'];
    $groups = $options['associations_groups'];
    $params = [
      '@field_title' => t('SNP Association File'),
      '@list' => implode(', ', $row),
    ];

    // Check if required columns has non-empty fields (values).
    $non_empty_column_list['required'] = self::getColumnOptions();
    // Note: optional 'Gene Id' and 'Annotation' columns could have empty
    // values or be missing in file at all so we don't check them.
    unset($non_empty_column_list['required'][self::DATA_TYPE_GENE_ID]);
    unset($non_empty_column_list['required'][self::DATA_TYPE_ANNOTATION]);
    // Conditional.
    $non_empty_column_list['conditional'][self::DATA_TYPE_YEAR]
      = $non_empty_column_list['required'][self::DATA_TYPE_YEAR];
    unset($non_empty_column_list['required'][self::DATA_TYPE_YEAR]);
    foreach ($non_empty_column_list as $type => $item) {
      foreach ($item as $column_key => $column_title) {
        $column_index = array_search($column_key, $column_list);
        if (empty($row[$column_index])) {
          if (
            $type == 'conditional'
            // Do not show error if column wasn't found in the list of required
            // columns which means it's not required not (conditions wasn't met).
            && empty($groups['Year'][self::DATA_TYPE_YEAR])
          ) {
            continue;
          }
          else {
            $params['@column_title'] = t($column_title);
            form_set_error($name, t('@field_title: Empty "@column_title" '
              . 'column is not allowed.<br />Row: @list.', $params));
          }
        }
      }
    }
  }

  /**
   * Replaces column index with column data type in $row.
   *
   * @param array $row
   *   Values of the single row. E.g.,
   *   Array
   *    (
   *        [A] => SNP3
   *        [B] => scaffold_3
   *        [C] => 2038
   *        [D] => T
   *        [E] => daylight hours
   *        [F] => 2021
   *        [G] => 2.13E-07
   *        [H] => GENEID3
   *        [I] => intergenic_region
   *   )
   * @param array $options
   *   Data for processing.
   *
   * @return array
   *   Returns associated array where keys are column name and values are
   *   values from the $row. Note: $options['associations'] also updated.
   */
  public static function getNamedRow($row, array &$options = []) {
    $groups = $options['associations_groups'];
    if (empty($groups)) {
      return;
    }

    $id = $row[($groups['SNP ID'][self::DATA_TYPE_SNP_ID] ?? NULL)] ?? NULL;
    $position = $row[$groups['Position'][3]];
    $trait = $row[$groups['Associated Trait'][5]];

    $confidence = $row[$groups['Confidence Value'][6]];
    $associations = &$options['associations'];
    $associations[$id] = [
      // Required.
      'id' => $id,
      'scaffold' => $row[$groups['Scaffold'][2]],
      'start' => $position,
      'stop' => $position,
      'allele' => $row[$groups['Allele'][4]],
      'trait' => $trait,
      'confidence' => $confidence,
      // Extra.
      //'trait_attr' => $options['phenotype_meta'][strtolower($trait)]['attr_id'],
      //'trait_obs' => $options['phenotype_meta'][strtolower($trait)]['struct_id'] ?? NULL,
      // Optional.
      'gene_id' => $row[($groups['Gene ID'][self::DATA_TYPE_GENE_ID] ?? NULL)] ?? NULL,
      'annotation' => $row[($groups['Annotation'][self::DATA_TYPE_ANNOTATION] ?? NULL)] ?? NULL,
      // Conditional.
      'year' => $row[$groups['Year'][9]] ?? NULL,
    ];

    // Process 'start:stop' format of the 'Position' column.
    preg_match('/^(\d+):(\d+)$/', $position, $matches);
    if (!empty($matches)) {
      // Format: start:stop:
      $start = $matches[1];
      $stop = $matches[2];
      // Swap.
      if ($start > $stop) {
        $temp = $start;
        $start = $stop;
        $stop = $temp;
      }
      $associations[$id]['start'] = $start;
      $associations[$id]['stop'] = $stop;
    }
    return $associations[$id];
  }

  /**
   * Processes a file.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $shared_state
   *   Submission's Shared State.
   * @param array $options
   *   Necessary data.
   *   WARNING: 'genotype_id' is required to store 'year' column on file.
   */
  public static function process($organism_index, array $shared_state, array &$options = []) {
    $fid = $shared_state['saved_values'][TPPS_PAGE_4]["organism-$organism_index"]
      ['genotype']['files']['snps-association'] ?? NULL;
    if (empty($fid)) {
      return;
    }

    tpps_log('Processing "SNP Association" file data...', [], TRIPAL_INFO);
    // Note: Full class name must be used (not 'self').
    tpps_file_iterator($fid, 'SnpAssociation::processRow', $options);
    tpps_log('Done.', [], TRIPAL_INFO);
  }

  /**
   * Processes a single row of the file.
   *
   * Must be used with tpps_file_iterator().
   *
   * @param mixed $row
   *   The item yielded by the TPPS file generator.
   * @param array $options
   *   Additional options set when calling tpps_file_iterator().
   */
  public static function processRow($row, array &$options = []) {
    if (empty($options['genotype_name'])) {
      return;
    }
    self::getNamedRow($row, $options);
    self::storeYear($row, $options);
  }

}
