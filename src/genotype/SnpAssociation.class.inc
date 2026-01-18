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
          $dom_name ?? static::DOM_NAME,
        ];
        break;

      case 2:
      case 3:
      default:
        $parents = [
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
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
    TppsForm::buildFileField([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => static::getParents($organism_index, $form_state),
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
          static::buildCustomStateName($organism_index, [
            static::DATA_TYPE,
            static::SNPS_FIELDSET,
            'upload_snp_association'
          ]) => ['value' => TPPS_YES],
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
      '#title' => t('SNP Association Confidence Value Type: *'),
      '#weight' => -110,
      '#options' => [
        0 => t('- Select -'),
        'P value' => t('P value'),
        'Genomic Inflation Factor (GIF)' => t('Genomic Inflation Factor (GIF)'),
        'P-adjusted (FDR) / Q value' => t('P-adjusted (FDR) / Q value'),
        'P-adjusted (FWE)' => t('P-adjusted (FWE)'),
        'P-adjusted (Bonferroni)' => t('P-adjusted (Bonferroni)'),
      ],
      '#states' => [
        // @todo Use required_when_visible feature.
        'visible' => [
          static::buildCustomStateName($organism_index, [
            static::DATA_TYPE,
            static::SNPS_FIELDSET,
            'upload_snp_association',
          ]) => ['value' => TPPS_YES],
          static::buildCustomStateName($organism_index, [
            static::DATA_TYPE,
            'does_study_include_snp_data',
          ]) => ['value' => 'yes'],
        ],
      ],
    ];

    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    // Field is optional.
    $field_name = 'snps-association-tool';
    $sub_form[static::SNPS_FIELDSET][$field_name] = [
      '#type' => 'select',
      '#title' => t('SNP Association Analysis Tool: *'),
      '#weight' => -130,
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
        // @todo Use required_when_visible feature.
        'visible' => [
          static::buildCustomStateName($organism_index, [
            static::DATA_TYPE,
            static::SNPS_FIELDSET,
            'upload_snp_association',
          ]) => ['value' => TPPS_YES],
          static::buildCustomStateName($organism_index, [
            static::DATA_TYPE,
            'does_study_include_snp_data',
          ]) => ['value' => 'yes'],
        ],
      ],
    ];
  }

  /**
   * Validates file field.
   *
   * Validation rules:
   * 1. ‘SNP Association file’ will always be optional, so it should not be
   * required to process ‘Assay Design’ file. However:
   * the columns in a ‘SNP Association File’ do need to be validated.
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
    $fid = static::getFileId($organism_index, $form_state);
    if (form_get_errors() || empty($fid)) {
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
      && PhenotypeData::hasColumn($organism_index, $form_state, PhenotypeData::DATA_TYPE_YEAR)
    ) {
      $required_groups['Year'] = ['year' => [static::DATA_TYPE_YEAR]];
    }
    // @todo Minor. We exclude 'Year' column data type when it's not
    // a Phenotypestudy or column 'Year' is missing in Phenotype Data file.
    // But also we could show warning message and ask user to upload
    // SNP Association file without 'Year' column.
    // @todo Use FileField::validateColumns().
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Match SNP IDs in 'SNP Association' file and in 'SNP Assay' file.
    //
    // Rule: The column ‘SNP_ID’ should only contain values also present in the
    // ‘SNP Assay file’ in column ‘SNP_ID’. The columns ‘Associated trait’
    // should only contain values also present in the ‘Phenotype’ file.
    // Trait names in a phenotype value are header values.
    $missing_items = static::matchColumn(
      $organism_index,
      $form_state,
      $groups['SNP ID'][static::DATA_TYPE_SNP_ID],
      SnpAssay::getSnpList($organism_index, $form_state)
    );
    // @todo Move error message into matchColumn() method.
    if (!empty($missing_items)) {
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'We detected SNP IDs that were not in your SNP Assay. '
        . 'Please either remove these SNPs from your Association file, '
        . 'or add them to your SNP Assay. '
        . '<br />The SNP Identifiers we found were: @list',
        [
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
          '@list' => implode(', ', $missing_items),
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check that Phenotype names match phenotype metadata section.
    if (static::isPhenotypeStudy($form_state)) {
      // Get missing items.
      if (PhenotypeMeta::getFileId($organism_index, $form_state)) {
        $phenotype_name_list = PhenotypeMeta::getPhenotypeNames(
          $organism_index, $form_state
        );
      }
      else {
        $phenotype_name_list = PhenotypeManual::getPhenotypeNames(
          $organism_index, $form_state
        );
      }

      // Validation rule:
      // The columns ‘Associated trait’ should only contain values also
      // present in the ‘Phenotype’ file. Trait names in a phenotype value
      // are header values.
      $missing_items = static::matchColumn(
        $organism_index,
        $form_state,
        $groups['Associated Trait'][static::DATA_TYPE_ASSOCIATED_TRAIT],
        $phenotype_name_list
      );
      // Report.
      // @todo Move error message into matchColumn() method.
      if (!empty($missing_items)) {
        $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
          . 'We detected Associated Traits that were not specified in the '
          . 'Phenotype Metadata Section. Please either remove these Traits '
          . 'from your Association file, or add them to your Phenotype '
          . 'Metadata section. <br />The Trait names we foud were: @list',
          [
            '@field_title' => t(static::getFieldTitle()),
            '@oidx' => $organism_index,
            '@list' => implode(', ', $missing_items),
          ]
        );
        form_set_error(static::getFormErrorName($organism_index), $error_message);
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check that position values are correctly formatted.
    $position_col = $groups['Position'][static::DATA_TYPE_POSITION];
    $assoc_no_header = $snps[static::DOM_NAME . '-no-header'] ?? FALSE;
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
