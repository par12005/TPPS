<?php

/**
 * @file
 * Contains 'SnpAssociation' class.
 *
 * Submission Version changes:
 * Field was relocated (v.2). ['files'] -> [$snps_fieldset].
 *
 * Note:
 * build() works with $form (and $form_state).
 * validate() works with $form_state.
 * process() works with $shared_state.
 */

/**
 * Manages SNP Association file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SnpAssociation extends FileField {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Common settings.
  //
  const TITLE = 'SNPs Association';
  // Name of the form field.
  const DOM_NAME = 'snps-association';
  // Name of the parent fieldset on form (!) not shared form state.
  // static::SNPS_FIELDSET.
  const SNPS_FIELDSET = 'SNPs';
  // Type of the data.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  const DATA_TYPE = 'genotype';
  // Form step where field is used.
  const FORM_STEP = TPPS_PAGE_4;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Custom settings.
  //
  // Data types for field columns.
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
   * Gets lit of required columns for validation.
   *
   * Note:
   * File class must overwrite this method of column validation is required.
   *
   * @return array
   *   Returns list of groups of required columns.
   *   See AssayDesign::getRequiredColumns() for the structure example.
   */
  public static function getRequiredColumns() {
    return [];
  }

  /**
   * Gets parents of the field.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return array
   *   Returns parents for the checkbox which makes this field required.
   */
  public static function getParents($organism_index, array $form_state) {
    $organism_key = 'organism-' . $organism_index;
    switch (SubmissionVersion::get($form_state)) {
      case 1:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          'files',
          static::DOM_NAME,
        ];
        break;

      case 2:
      case 3:
      default:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::DOM_NAME,
        ];
        break;
    }
    return $parents;
  }

  /**
   * Gets list of column data types.
   *
   * @return array
   *   Returns list of options with column data types.
   */
  public static function getColumnOptions() {
    return [
      static::DATA_TYPE_SNP_ID => 'SNP ID',
      static::DATA_TYPE_SCAFFOLD => 'Scaffold',
      static::DATA_TYPE_POSITION => 'Position',
      static::DATA_TYPE_ALLELE => 'Allele',
      static::DATA_TYPE_ASSOCIATED_TRAIT => 'Associated Trait',
      static::DATA_TYPE_CONFIDENCE_VALUE => 'Confidence Value',
      static::DATA_TYPE_GENE_ID => 'Gene ID',
      static::DATA_TYPE_ANNOTATION => 'Annotation',
      // Option 'year' must be shown only when Phenotype data file has 'year'
      // column. Since phenotype data file could be added/removed at the same
      // page beside 'SNP Association' file field we add this option at
      // server side and will add/remove it on client's side (in browser).
      static::DATA_TYPE_YEAR => 'Year',
    ];
  }

  /**
   * Builds form fields for Phenotype Data file.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($organism_index, array &$form, array $form_state) {
    if (!tpps_is_genotype_data_type($form_state)) {
      return;
    }
    $organism_key = 'organism-' . $organism_index;
    $sub_form = &$form[$organism_key][static::DATA_TYPE];

    $sub_form[static::SNPS_FIELDSET]['upload_snp_association'] = [
      '#type' => 'select',
      '#title' => t('Would you like to upload a SNP association file?'),
      '#options' => [
        0 => t('- Select -'),
        TPPS_YES => t('Yes'),
        TPPS_NO => t('No'),
      ],
      '#default_value' => tpps_get_ajax_value($form_state, [
        $organism_key,
        static::DATA_TYPE,
        static::SNPS_FIELDSET,
        'upload_snp_association',
      ]),
    ];
    // SNP Association File.
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => [
        $organism_key,
        static::DATA_TYPE,
        static::SNPS_FIELDSET,
      ],
      'field_name' => static::DOM_NAME,
      'title' => t(static::getFieldTitle()),
      'organism_name' => $organism_key,
      'type' => static::DATA_TYPE,

      'description' => t('Please upload a spreadsheet file containing '
        . 'SNPs Association data. When your file is uploaded, you will '
        . 'be shown a table with your column header names, several '
        . 'drop-downs, and the first few rows of your file. You will be '
        . 'asked to define the data type for each column, using the '
        . 'drop-downs provided to you. If a column data type does not '
        . 'fit any of the options in the drop-down menu, you may set that '
        . 'drop-down menu to "N/A". Your file must contain columns with '
        . 'the SNP ID, Scaffold, Position (formatted like "start:stop"), '
        . 'Allele (formatted like "major:minor"), Associated Trait Name '
        . '(must match a phenotype from the above section), and '
        . 'Confidence Value. Optionally, you can also specify a Gene ID '
        . '(which should match the gene reference) and '
        . 'a SNP Annotation (non synonymous, coding, etc).'),
      '#tree' => TRUE,
      'optional' => TRUE,
      'states' => [
        'visible' => [
          static::getStateName($organism_index,
            [static::SNPS_FIELDSET, 'upload_snp_association']
          ) => ['value' => TPPS_YES],
        ],
      ],
      'extra_elements' => [
        'empty' => [
          '#default_value' => TppsArray::getValue($form_state,
            [
              'saved_values',
              static::FORM_STEP,
              $organism_key,
              static::DATA_TYPE,
              static::SNPS_FIELDSET,
              static::DOM_NAME,
              'empty',
            ]) ?? 'NA',
        ],
        'columns' => [
          '#description' => t('Please define which columns hold the '
            . 'required data: SNP ID, Scaffold, Position, Allele, '
            . 'Associated Trait, Confidence Value.'),
        ],
        'columns-options' => [
          '#type' => 'hidden',
          '#value' => SnpAssociation::getColumnOptions(),
        ],
        'no-header' => [],
      ],
    ]);
    tpps_add_css_js('page_4_snp_association', $form);
    // Send Option Id of the 'Year' data type to browser to enable/disable this
    // option when Phenotype Data file has/hasn't 'Year' column.
    $form['#attached']['js'][] = [
      'type' => 'setting',
      'data' => [
        'tpps' => [
          'snpAssociation' => ['dataTypeYear' => SnpAssociation::DATA_TYPE_YEAR],
          'phenotypeData' => ['dataTypeYear' => PhenotypeData::DATA_TYPE_YEAR],
        ],
      ],
      'scope' => 'footer',
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    // This field must be optional to not block submission but later will be
    // moved to new page5.
    $field_name = 'snps-association-type';
    $sub_form[static::SNPS_FIELDSET][$field_name] = [
      '#type' => 'select',
      '#title' => t('Confidence Value Type: *'),
      '#options' => [
        0 => t('- Select -'),
        'P value' => t('P value'),
        'Genomic Inflation Factor (GIF)' => t('Genomic Inflation Factor (GIF)'),
        'P-adjusted (FDR) / Q value' => t('P-adjusted (FDR) / Q value'),
        'P-adjusted (FWE)' => t('P-adjusted (FWE)'),
        'P-adjusted (Bonferroni)' => t('P-adjusted (Bonferroni)'),
      ],
      '#states' => [
        'visible' => [
          static::getStateName($organism_index,
            [static::SNPS_FIELDSET, 'upload_snp_association']
          ) => ['value' => TPPS_YES],
          static::getStateName($organism_index,
            ['does_study_include_snp_data']
          ) => ['value' => 'yes'],
        ],
      ],
    ];

    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    // Field is optional.
    $field_name = 'snps-association-tool';
    $sub_form[static::SNPS_FIELDSET][$field_name] = [
      '#type' => 'select',
      '#title' => t('Association Analysis Tool: *'),
      '#options' => [
        0 => t('- Select -'),
        'GEMMA' => t('GEMMA'),
        'EMMAX' => t('EMMAX'),
        'Plink' => t('Plink'),
        'Tassel' => t('Tassel'),
        'Sambada' => t('Sambada'),
        'Bayenv' => t('Bayenv'),
        'BayeScan' => t('BayeScan'),
        'LFMM' => t('LFMM'),
      ],
      '#states' => [
        'visible' => [
          static::getStateName($organism_index,
            [static::SNPS_FIELDSET, 'upload_snp_association']
          ) => ['value' => TPPS_YES],
          static::getStateName($organism_index,
            ['does_study_include_snp_data']
          ) => ['value' => 'yes'],
        ],
      ],
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
    $fid = static::getFileId($organism_index, $form_state);
    $genotype = &$form_state['values'][$organism_key][static::DATA_TYPE] ?? NULL;
    $snps = &$genotype[static::SNPS_FIELDSET] ?? NULL;
    $snps_assay = $snps['snps-assay'] ?? 0;
    $file_type = $snps['file-type'] ?? NULL;

    if (form_get_errors()) {
      // Do not validate if any errors already found.
      return FALSE;
    }
    static::validateRequired($organism_index, $form_state, (
      // Requirement conditions:
      (
        $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
        || $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY
      )
      && !empty($snps['snps-assay'])
      && TppsArray::getValue($snps, ['upload_snp_association']) == TPPS_YES
    ));
    // Do NOT allow empty field validation.
    if (form_get_errors()) {
      return FALSE;
    }

    // @todo Minor. Implement ability to run this code when study was already
    // submitted (on processing step).
    $required_groups = [
      'SNP ID' => ['id' => [static::DATA_TYPE_SNP_ID]],
      'Scaffold' => ['scaffold' => [static::DATA_TYPE_SCAFFOLD]],
      'Position' => ['position' => [static::DATA_TYPE_POSITION]],
      'Allele' => ['allele' => [static::DATA_TYPE_ALLELE]],
      'Associated Trait' => ['trait' => [static::DATA_TYPE_ASSOCIATED_TRAIT]],
      'Confidence Value' => ['confidence' => [static::DATA_TYPE_CONFIDENCE_VALUE]],
      // Optional.
      // 7 => 'Gene ID',
      // 8 => 'Annotation',
      // Conditional.
      // Required when Phenotype data file exists and has 'Year'column.
      // 9 => 'Year'.
    ];

    if (
      static::isPhenotypeStudy($form_state)
      && PhenotypeData::isDataTypeSet($form_state, $organism_index, PhenotypeData::DATA_TYPE_YEAR)
    ) {
      $required_groups['Year'] = ['year' => [static::DATA_TYPE_YEAR]];
    }
    // @todo Minor. We exclude 'Year' column data type when it's not
    // a Phenotypestudy or column 'Year' is missing in Phenotype Data file.
    // But also we could show warning message and ask user to upload
    // SNP Association file without 'Year' column.
    $groups = tpps_file_validate_columns(
      $form_state,
      $required_groups,
      // Note: This is file id stored in $form!
      $form[$organism_key]['genotype'][static::SNPS_FIELDSET][static::DOM_NAME]
    );

    if (form_get_errors()) {
      return;
    }
    // Do not allow the same data type for multiple columns.
    static::allowColumnDataTypeUseOnce($organism_index, $groups);
    if (form_get_errors()) {
      return;
    }

    // Check that SNP IDs match Genotype Assay.
    $snps_id_col = $groups['SNP ID'][static::DATA_TYPE_SNP_ID];
    // For search purpose: 'snps-association-no-header'.
    $assoc_no_header = $snps[static::DOM_NAME . '-no-header'] ?? FALSE;

    $assay_snps = tpps_file_headers($snps_assay);
    unset($assay_snps[key($assay_snps)]);
    $assoc_snps = tpps_parse_file_column($fid, $snps_id_col, $assoc_no_header);
    $missing_snps = array_diff($assoc_snps, $assay_snps);

    // @todo Re-use FileField::matchColumnValues().
    if (!empty($missing_snps)) {
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'We detected SNP IDs that were not in your SNP Assay. '
        . 'Please either remove these SNPs from your Association file, '
        . 'or add them to your SNP Assay. '
        . '<br />The SNP Identifiers we found were: @list',
        [
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
          '@list' => implode(', ', $missing_snps),
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
    }

    // Check that Phenotype names match phenotype metadata section.
    $trait_id_col = $groups['Associated Trait'][static::DATA_TYPE_ASSOCIATED_TRAIT];
    $association_phenotypes = tpps_parse_file_column(
      $fid, $trait_id_col, $assoc_no_header
    );

    if (static::isPhenotypeStudy($form_state)) {
      $phenotype = $form_state['values'][$organism_key]['phenotype'];
      $phenotype_metadata_fid = $phenotype['metadata'];
      $phenotype_number = $phenotype['phenotypes-meta']['number'];
      $phenotype_meta_names = [];
      $phenotype_name_col = PhenotypeData::getPhenotypeNameColumn(
        $organism_index,
        $form_state
      );
      if (isset($phenotype_name_col)) {
        $phenotype_meta_names = tpps_parse_file_column(
          $phenotype_metadata_fid,
          $phenotype_name_col
        );
      }
      for ($i = 1; $i <= $phenotype_number; $i++) {
        $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
      }
      $missing_phenotypes = array_diff($association_phenotypes, $phenotype_meta_names);
      if (!empty($missing_phenotypes)) {
        $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
          . 'We detected Associated Traits that were not specified in the '
          . 'Phenotype Metadata Section. Please either remove these Traits '
          . 'from your Association file, or add them to your Phenotype '
          . 'Metadata section. <br />The Trait names we foud were: @list',
          [
            '@field_title' => t(static::getFieldTitle()),
            '@oidx' => $organism_index,
            '@list' => implode(', ', $missing_phenotypes),
          ]
        );
        form_set_error(static::getFormErrorName($organism_index), $error_message);
      }
    }

    // Check that position values are correctly formatted.
    $position_col = $groups['Position'][static::DATA_TYPE_POSITION];
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
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'We detected SNP positions that do not match the required format: '
        . '@position. <br />The correct format is: "start:stop" or integer.',
        [
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
          '@position' => $position,
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
      break;
    }
    if (form_get_errors()) {
      return;
    }
    $options = [
      'associations_groups' => $groups,
      'organism_index' => $organism_index,
      'column_list' => $snps[static::DOM_NAME . '-columns'],
    ];
    tpps_file_iterator($fid, 'SnpAssociation::validateRow', $options);
    static::preserveValidFile($organism_index, $form_state);
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
    $named_row = static::getNamedRow($row, $options);
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
    $query->condition('ct.cv_id', static::CV_ID_PATO);
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
   *   'organism_index' - int, ordinal number of organism on page. E.g., 1.
   */
  public static function validateRow($row, array &$options = []) {
    static::getNamedRow($row, $options);
    $associations = $options['associations'];
    $column_list = $options['column_list'];
    $groups = $options['associations_groups'];
    $organism_index = $options['organism_index'];
    $params = [
      '@field_title' => t('SNP Association File'),
      '@list' => implode(', ', $row),
    ];

    // Check if required columns has non-empty fields (values).
    $non_empty_column_list['required'] = static::getColumnOptions();
    // Note: optional 'Gene Id' and 'Annotation' columns could have empty
    // values or be missing in file at all so we don't check them.
    unset($non_empty_column_list['required'][static::DATA_TYPE_GENE_ID]);
    unset($non_empty_column_list['required'][static::DATA_TYPE_ANNOTATION]);
    // Conditional.
    $non_empty_column_list['conditional'][static::DATA_TYPE_YEAR]
      = $non_empty_column_list['required'][static::DATA_TYPE_YEAR];
    unset($non_empty_column_list['required'][static::DATA_TYPE_YEAR]);
    foreach ($non_empty_column_list as $type => $item) {
      foreach ($item as $column_key => $column_title) {
        $column_index = array_search($column_key, $column_list);
        if (empty($row[$column_index])) {
          if (
            $type == 'conditional'
            // Do not show error if column wasn't found in the list of required
            // columns which means it's not required not (conditions wasn't met).
            && empty($groups['Year'][static::DATA_TYPE_YEAR])
          ) {
            continue;
          }
          else {
            $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
              . 'Empty "@column_title" column is not allowed.',
              [
                '@field_title' => t(static::getFieldTitle()),
                '@oidx' => $organism_index,
                '@column_title' => t($column_title),
              ]
            );
            form_set_error(static::getFormErrorName($organism_index), $error_message);
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
  public static function getNamedRow(array $row, array &$options = []) {
    $groups = $options['associations_groups'];
    if (empty($groups)) {
      return;
    }

    $id = $row[($groups['SNP ID'][static::DATA_TYPE_SNP_ID] ?? NULL)] ?? NULL;
    $position = $row[$groups['Position'][static::DATA_TYPE_POSITION]];
    $trait = $row[$groups['Associated Trait'][static::DATA_TYPE_ASSOCIATED_TRAIT]];
    $confidence = $row[$groups['Confidence Value'][static::DATA_TYPE_CONFIDENCE_VALUE]];

    $associations = &$options['associations'];
    $associations[$id] = [
      // Required.
      'id' => $id,
      'scaffold' => $row[$groups['Scaffold'][static::DATA_TYPE_SCAFFOLD]],
      'start' => $position,
      'stop' => $position,
      'allele' => $row[$groups['Allele'][static::DATA_TYPE_ALLELE]],
      'trait' => $trait,
      'confidence' => $confidence,
      // Extra.
      //'trait_attr' => $options['phenotype_meta'][strtolower($trait)]['attr_id'],
      //'trait_obs' => $options['phenotype_meta'][strtolower($trait)]['struct_id'] ?? NULL,
      // Optional.
      'gene_id' => $row[($groups['Gene ID'][static::DATA_TYPE_GENE_ID] ?? NULL)] ?? NULL,
      'annotation' => $row[($groups['Annotation'][static::DATA_TYPE_ANNOTATION] ?? NULL)] ?? NULL,
      // Conditional.
      'year' => NULL,
    ];

    // Get 'Year' value.
    $year_column_letter = $groups['Year'][9] ?? NULL;
    if (!empty($year_column_letter) && isset($row[$year_column_letter])) {
      $associations[$id]['year'] = $row[$year_column_letter];
    }

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
   * Note: called from /forms/submit/submit_all.php script.
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
    if (empty($fid = static::getFileId($organism_index, $shared_state))) {
      return;
    }

    tpps_log('Started: Processing "SNP Association" file data.');
    // Note: Full class name must be used (not 'self').
    tpps_file_iterator($fid, 'SnpAssociation::processRow', $options);
    tpps_log('Complete: Processing "SNP Association" file data.');
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
    static::getNamedRow($row, $options);
    static::storeYear($row, $options);
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
  public static function processRowSynonymYear($row, array &$options = []) {
    if (empty($organism_index = ($options['organism_index'] ?? NULL))) {
      return;
    }
    if (
      empty($trait_column = ($options['trait_column'][$organism_index] ?? NULL))
      || empty($year_column = ($options['year_column'][$organism_index] ?? NULL))
    ) {
      return;
    }

    // 'Associated Trait' column in SNP Association file equals to the
    // Phenotype Synonym name.
    if (!empty($year_value = $row[$year_column])) {
      $trait = $row[$trait_column];
      // @todo When it will be confirmed that different organisms couldn't use
      // the same attribute ids and years the $organism_index could be removed.
      $options['snp_association_year'][$trait][$organism_index] = $year_value;
    }
  }

}
