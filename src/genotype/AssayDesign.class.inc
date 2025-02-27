<?php

/**
 * @file
 * Contains 'AssayDesign' class.
 */

/**
 * Manages 'Assay Design' file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class AssayDesign extends FileField {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Unique common settings.
  //
  const TITLE = 'Assay Design';
  // Name of the form field.
  const FIELD_NAME = 'assay-design';
  // Name of the parent fieldset on form (!) not shared form state.
  // self::FORM_PARENT_FIELDSET.
  const FORM_PARENT_FIELDSET = 'SNPs';
  // Name of the parent element in shared state array.
  // Note: Differs if field was relocated. See versions in function
  // tpps_submission_form_version_changes().
  const SHARED_STATE_PARENT_FIELDSET = 'files';
  // Type of the data.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  const DATA_TYPE = 'genotype';
  // Form step where field is used.
  const FORM_STEP = 4;


  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Custom settings.
  //
  // Data types for field columns.
  const DATA_TYPE_SNP_ID = 1;
  const DATA_TYPE_SCAFFOLD = 2;
  const DATA_TYPE_POSITION = 3;
  const DATA_TYPE_FLANK_SEQUENCE = 4;

  /**
   * Gets list of column data types.
   *
   * Note:
   * To get name by key: (static::getColumnOptions()[$data_type_key] ?? NULL).
   *
   * @return array
   *   Returns list of options with column data types.
   */
  public static function getColumnOptions() {
    // List of data types to be used in the dropdown list from Meghan.
    // https://plantcompgenomics.slack.com/archives/C05R396U4QK/p1730737064312679?thread_ts=1730430961.236099&cid=C05R396U4QK
    return [
      0 => t('N/A'),
      self::DATA_TYPE_SNP_ID => t('SNP ID'),
      self::DATA_TYPE_SCAFFOLD => t('Scaffold'),
      self::DATA_TYPE_POSITION => t('Position'),
      self::DATA_TYPE_FLANK_SEQUENCE => t('Flank sequence'),
    ];
  }

  /**
   * Gets required columns.
   *
   * Valid only sets of columns:
   *   1. SNP ID, scaffold, position.
   *   2. SNP ID, Flank sequence.
   *
   * @return array
   *   Returns list of groups of the required columns.
   */
  public static function getRequiredColumns() {
    return [
      'case 1' => [
        self::DATA_TYPE_SNP_ID,
        self::DATA_TYPE_SCAFFOLD,
        self::DATA_TYPE_POSITION,
      ],
      'case 2' => [
        self::DATA_TYPE_SNP_ID,
        self::DATA_TYPE_FLANK_SEQUENCE,
      ],
    ];
  }

  /**
   * Builds file field.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($organism_index, array &$form, array $form_state) {
    if (!tpps_is_genotype_data_type($form_state)) {
      return;
    }
    $organism_name = 'organism-' . $organism_index;
    $sub_form = &$form[$organism_name][self::DATA_TYPE];

    // Add file upload field.
    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => [$organism_name, self::DATA_TYPE, self::FORM_PARENT_FIELDSET],
      'field_name' => self::FIELD_NAME,
      'title' => t(self::getFieldTitle()),
      'organism_name' => $organism_name,
      'type' => self::DATA_TYPE,

      'optional' => TRUE,
      'states' => [
        'visible' => [
          [
            self::getStateName($organism_index,
              [self::FORM_PARENT_FIELDSET, 'genotyping-type']
            ) => ['value' => TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY],
          ],
          'or',
          [
            self::getStateName($organism_index,
              [self::FORM_PARENT_FIELDSET, 'file-type']
            ) => ['value' => TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE],
          ],
        ],
      ],
      'extra_elements' => [
        'empty' => [
          '#default_value' => TppsArray::getValue($form_state,
            [
              'saved_values',
              self::FORM_STEP,
              $organism_name,
              self::DATA_TYPE,
              self::FORM_PARENT_FIELDSET,
              self::FIELD_NAME,
              'empty',
            ]
          ) ?? 'NA',
        ],
        'columns' => [
          '#description' => t('Please define which columns hold the '
            . 'required data: SNP ID, Scaffold, Position, Allele, '
            . 'Associated Trait, Confidence Value.'),
        ],
        'columns-options' => [
          '#type' => 'hidden',
          '#value' => self::getColumnOptions(),
        ],
        'no-header' => [],
      ],
    ]);

    // Field was relocated (v.2). ['files'] -> ['SNPs'].
    $sub_form[self::FORM_PARENT_FIELDSET]['assay-citation'] = [
      '#type' => 'textfield',
      '#title' => t('Assay Design Citation (Optional):'),
      '#description' => t('If your assay design file is from a different '
        . 'paper, please include the citation for that paper here.'),
      '#states' => [
        'visible' => [
          [
            self::getStateName($organism_index,
              [self::FORM_PARENT_FIELDSET, 'genotyping-type']
            ) => ['value' => TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY],
          ],
          'or',
          [
            self::getStateName($organism_index,
              [self::FORM_PARENT_FIELDSET, 'file-type']
            ) => ['value' => TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE],
          ],
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
    $genotype = &$form_state['values']['organism-' . $organism_index][self::DATA_TYPE] ?? NULL;
    $snps = &$genotype[self::FORM_PARENT_FIELDSET] ?? NULL;
    // Fields.
    $file_type = $snps['file-type'] ?? NULL;
    $genotyping_type = $snps['genotyping-type'] ?? NULL;
    $fid = self::getFileId($organism_index, $form_state);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if field is required.
    self::validateRequired($organism_index, $form_state, (
      // Requirement conditions:
      (
        $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
        || $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY
      )
      // Actually only this rule must be used:
      && !empty($snps['snps-assay'])
    ));
    // Do NOT allow further validation.
    if (form_get_errors()) {
      return FALSE;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if other required files was uploaded.
    if (empty($snps['snps-association'])) {
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'File "SNP Association" is required.',
        [
          '@field_title' => t(self::getFieldTitle()),
          '@oidx' => $organism_index,
        ]
      );
      form_set_error(self::getFormErrorName($organism_index), $error_message);
    }
    // Do NOT allow further validation.
    if (form_get_errors()) {
      return FALSE;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if required data types for columns was set.
    // Note: Assay Design has two (not one) groups of required columns.
    if (self::hasColumns($organism_index, $form_state)) {

      $error_name = self::getFormErrorName($organism_index);
      if (empty($assoc_file = $snps['snps-association'])) {
        // @todo Minor. Highlight 'SNP Association' file.
        $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
          . 'Requires "SNP Association" file.',
          [
            '@field_title' => t(self::getFieldTitle()),
            '@oidx' => $organism_index,
          ]
        );
        form_set_error(self::getFormErrorName($organism_index), $error_message);
      }
      if (empty($snps_assay = $snps['snps-assay'])) {
        // @todo Minor. Highlight 'SNP Assay' file.
        $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
          . 'Requires "SNP Assay" file.',
          [
            '@field_title' => t(self::getFieldTitle()),
            '@oidx' => $organism_index,
          ]
        );
        form_set_error(self::getFormErrorName($organism_index), $error_message);
      }

      // Validate column data types.
      self::validateColumns($organism_index, $form_state);
      if (form_get_errors()) {
        return;
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Match SNP Ids in current file and 'SNPs Assay' file (header).
      self::matchColumnValues([
        'organism_index' => $organism_index,
        'form_state' => $form_state,
        // Current file.
        'no_header' => self::getSubFieldValue(
          $organism_index,
          $form_state,
          'no-header'
        ),
        'column_letter' => self::getSelectedColumns(
          $organism_index,
          $form_state
        )[self::DATA_TYPE_SNP_ID],
        // Target file.
        'target_column_letter' => NULL,
        'target_form_element' => SnpAssay::getFormElement($organism_index, $form_state),
        'target_no_header' => NULL,
      ]);
      if (form_get_errors()) {
        return;
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Match SNP Ids in current file and 'SNP Association' file (column).
      self::matchColumnValues([
        'organism_index' => $organism_index,
        'form_state' => $form_state,
        // Curent file.
        'no_header' => $snps[self::FIELD_NAME . '-no-header'] ?? FALSE,
        'column_letter' => self::getSelectedColumns(
          $organism_index,
          $form_state
        )[self::DATA_TYPE_SNP_ID],
        // Target file.
        'target_form_element' => SnpAssociation::getFormElement(
          $organism_index,
          $form_state
        ),
        'target_column_letter' => SnpAssociation::getSelectedColumns(
          $organism_index,
          $form_state
        )[SnpAssociation::DATA_TYPE_SNP_ID],
        'target_no_header' => SnpAssociation::getSubFieldValue(
          $organism_index,
          $form_state,
          'no-header'
        ),
      ]);
      if (form_get_errors()) {
        return;
      }
    }
    self::preserveValidFile($organism_index, $form_state);
  }

  /**
   * Processes a file.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $shared_state
   *   Submission's Shared State.
   * @param array $options
   *   Options for processing. Could be used to return processing results.
   */
  public static function process($organism_index, array $shared_state, array &$options = []) {
    // This if statement caters for the Genotype Assay Design file
    // (which holds extra data like positions etc)
    // This is usually also accompanied with the Genotype SNP Assay
    // (which holds the snps)
    // We want to insert location data into the database
    // if both are found and the marker-type is snp
    // The previous step took care of the SNPs insertion via the Genotype
    // SNP Assay (not to be confused with genotype SNP assay design file)
    if (empty($fid = self::getFileId($organism_index, $shared_state))) {
      return;
    }
    tpps_add_project_file($shared_state, $fid);

    $genotype = TppsArray::getValue($shared_state, [
      'saved_values',
      self::FORM_STEP,
      'organism-' . $organism_index,
      self::DATA_TYPE,
    ]);

    // Setup the options array which the tpps_file_iterator custom function
    // will be able to access necessary details.
    $options['type'] = 'snp';

    print_r("\n");
    $options['marker'] = 'SNP';
    $options['type_cvterm'] = tpps_load_cvterm('snp')->cvterm_id;
    $options['ref-genome'] = $genotype['ref-genome'];
    $ref_genome = $genotype['ref-genome'];
    echo "Ref-genome: $ref_genome\n";
    // Lookup analysis id from reference genome and add it to options array.
    $options['analysis_id'] = tpps_get_analysis_id_from_ref_genome($ref_genome);
    tpps_log("ANALYSIS ID: " . $options['analysis_id'], [], TRIPAL_DEBUG);

    // We must have an analysis_id to tie back to the srcfeature.
    if ($options['analysis_id'] != NULL) {
      // Initialize new records with featureloc array to store records.
      $options['records']['featureloc'] = [];
      $options['records']['featureprop'] = [];

      $options['headers'] = tpps_file_headers($fid);
      tpps_log(
        "HEADERS:\n@headers\n",
        ['@headers' => print_r($options['headers'], 1)],
        TRIPAL_DEBUG
      );

      // Find the marker name header.
      $options['file_columns'] = [];
      foreach ($options['headers'] as $column => $column_name) {
        $column_name = strtolower(trim($column_name));
        tpps_log("Spreadsheet column name:" . $column_name . " column: $column",
          [], TRIPAL_DEBUG);
        switch ($column_name) {
          case 'chr':
            $options['file_columns']['chr'] = $column;
            break;

          case 'forward sequence':
            $options['file_columns']['forward_sequence'] = $column;
            break;

          case 'reverse sequence':
            $options['file_columns']['reverse_sequence'] = $column;
            break;

          case 'snp':
            $options['file_columns']['snp'] = $column;
            break;
        }
        if (strpos($column_name, 'position') !== FALSE) {
          $options['file_columns']['position'] = $column;
        }
        elseif (strpos($column_name, 'marker name') !== FALSE) {
          $options['file_columns']['marker_name'] = $column;
        }
        tpps_log(print_r($options['file_columns'], 1), [], TRIPAL_DEBUG);
      }

      // We want to process this Genotype SNP Assay Design file before
      // we add it as a project file.
      tpps_log('Processing genotype_snp_assay_design file data...', [], TRIPAL_INFO);
      tpps_file_iterator($fid, 'AssayDesign::processRow', $options);
      tpps_log('Done.', [], TRIPAL_INFO);

      tpps_log('Inserting genotype_snp_assay_design_spreadsheet data into '
        . 'database using insert_multi...', [], TRIPAL_INFO);
      tpps_log('Done.', [], TRIPAL_INFO);
      // Reset options[records] with empty records arrays.
      $options['records'] = [
        'feature' => [],
        'genotype' => [],
        'genotype_call' => [],
        'stock_genotype' => [],
      ];
    }
    else {
      tpps_log('Analysis ID could not be found, skipping assay design file processing.',
        [], TRIPAL_ERROR
      );
    }
  }

  /**
   * Processes a single row of the file.
   *
   * Must be used with tpps_file_iterator().
   * Processes genotype SNP design assay file.
   * Initially done to get position data from the assay design file.
   *
   * @param mixed $row
   *   The item yielded by the TPPS file generator.
   * @param array $options
   *   Additional options set when calling tpps_file_iterator().
   */
  public static function processRow($row, array &$options = []) {
    $debug_mode = FALSE;
    // Disabled by default.
    // If the scaffold is only the number then append scaffold_ to it (TGDR665).
    $use_scaffold_prefix = FALSE;

    $line_rows = $row;
    // Needed to lookup source features.
    $analysis_id = $options['analysis_id'];
    $headers = &$options['headers'];
    $records = &$options['records'];
    $columns = $options['file_columns'];
    $organism_index = $options['organism_index'];
    $seq_var_cvterm = $options['seq_var_cvterm'];
    if ($debug_mode) {
      tpps_log($columns, 'File columns');
      tpps_log($line_rows, 'Data in row');
    }

    $chr_name = $line_rows[$columns['chr']];

    if ($use_scaffold_prefix) {
      if (is_numeric($chr_name)) {
        $chr_name = "scaffold_" . $chr_name;
      }
      // If TGDR665, replace 'chr' with 'scaffold_'.
      // @TODO It's safe enough just to run str_replace() without extra checks.
      if (substr($chr_name, 0, 3) == 'chr') {
        $chr_name = str_replace('chr', 'scaffold_', $chr_name);
      }
    }

    $marker_name_raw = $line_rows[$columns['marker_name']];
    $position = intval($line_rows[$columns['position']]);
    // We could force 'SNP' here.
    $marker_type = $options['marker_type'];

    // RISH NOTES: This addition uses the organism_id based on the
    // organism order of the fourth page (we likely have to pass the
    // i from previous function here)
    // THIS TECHNICALLY OVERRIDES PETER'S LOGIC ABOVE.
    // TO BE DETERMINED IF RISH'S WAY IS CORRECT OR NOT [6/22/2023]
    // THIS WAS AN ISSUE BROUGHT UP BY EMILY REGARDING SNPS NOT BEING
    // ASSOCIATED WITH POP TRICH (665 STUDY)
    $species_codes = $options['species_codes'];
    $species_code = $organism_id = NULL;
    $count_tmp = 0;
    foreach ($species_codes as $organism_id_tmp => $species_code_tmp) {
      $count_tmp = $count_tmp + 1;
      // Check if count_tmp matches $organism_index.
      if ($count_tmp == $organism_index) {
        $species_code = $species_code_tmp;
        $organism_id = $organism_id_tmp;
        break;
      }
    }

    $srcfeature_id = NULL;
    // Get the srcfeature_id.
    $srcfeature_results = chado_query('
      SELECT feature.feature_id
      FROM chado.feature
      JOIN chado.analysisfeature ON feature.feature_id = analysisfeature.feature_id
      WHERE feature.name = :chr_name
        AND analysisfeature.analysis_id = :analysis_id',
      [
        ':chr_name' => $chr_name,
        ':analysis_id' => $analysis_id
      ]
    );

    foreach ($srcfeature_results as $row) {
      $srcfeature_id = $row->feature_id;
    }

    if ($srcfeature_id == NULL) {
      tpps_log("srcfeature_id for @name could not be found - we can't add featureloc data.",
        ['@name' => $chr_name], TRIPAL_ERROR);
      return;
    }

    tpps_log('srcfeature_id for @name: @id.',
      ['@name' => $chr_name, '@id' => $srcfeature_id]
    );

    $marker_name = $marker_name_raw . $marker_type;

    // We need to find the current marker_name in the feature table.
    // @TODO Replace with db_insert() which is much faster and allows to get
    // last inserted id.
    $sql = "SELECT * FROM chado.feature WHERE uniquename = :uniquename";
    $results = chado_query($sql, [':uniquename' => $marker_name]);
    $feature_id = NULL;
    foreach ($results as $feature) {
      $feature_id = $feature->feature_id;
    }

    if ($feature_id == NULL) {
      // We should add the marker (marker_name).
      chado_insert_record('feature', [
        'name' => $marker_name,
        'organism_id' => $organism_id,
        'uniquename' => $marker_name,
        'type_id' => $seq_var_cvterm,
      ]);
      // Recheck for the feature_id.
      $sql = "SELECT * FROM chado.feature WHERE uniquename = :uniquename";
      $results = chado_query($sql, [':uniquename' => $marker_name]);
      foreach ($results as $feature) {
        $feature_id = $feature->feature_id;
      }
    }

    if ($feature_id == NULL) {
      tpps_log('Marker name @marker_name feature_id could not be found.',
        ['@marker_name' => $marker_name],
        TRIPAL_ERROR
      );
      return;
    }
    tpps_log("Marker name $marker_name has feature_id: $feature_id");

    // Before we add a new featureloc record, check to make sure one
    // does not already exist in the featureloc table since we don't
    // currently delete previous featurelocs on study reloads.
    $featureloc_results = chado_query('
      SELECT count(*) as c1
      FROM chado.featureloc
      WHERE feature_id = :feature_id
        AND srcfeature_id = :srcfeature_id;',
      [
        ':feature_id' => $feature_id,
        ':srcfeature_id' => $srcfeature_id
      ]
    );
    $featureloc_count = 0;
    foreach ($featureloc_results as $row) {
      $featureloc_count = $row->c1;
    }
    // This means no featureloc exists, so insert it.
    if ($featureloc_count == 0) {
      // Check for indels (longer than 1 reads)
      $snp = trim($line_rows[$columns['snp']]);
      $snp_possible_reads = explode('/', $snp);
      // Default for non-indels.
      $read_length = 0;
      foreach ($snp_possible_reads as $read) {
        $tmp_read_length = strlen($read);
        // Only for indels, we need a read_length of more than 0.
        if ($tmp_read_length > 1) {
          if ($tmp_read_length > $read_length) {
            $read_length = $tmp_read_length;
          }
        }
      }

      // TODO: if read_length is more than 1, it is an indel, change marker type.
      // Also check code in tpps_process_genotype_sheet to check this or else
      // these 2 things will cause a submit failure - after conference (6/15/2023)
      $records['featureloc'][$marker_name] = [
        'fmin' => $position,
        'fmax' => ($position + $read_length),
        'srcfeature_id' => $srcfeature_id,
        'feature_id' => $feature_id,
      ];
      print_r($records['featureloc'][$marker_name]);
    }
    else {
      tpps_log('[GOOD ALTERNATIVE] Featureloc record already exists, no need to add.');
    }

    // Check if forward sequence information has been added.
    $forward_sequence_cvterm_id = NULL;
    // Get cvterm_id (assuming it exists)
    $results = chado_query("SELECT * FROM chado.cvterm
      WHERE name = 'five_prime_flanking_region' LIMIT 1;", []);
    foreach ($results as $row) {
      $forward_sequence_cvterm_id = $row->cvterm_id;
    }

    // Check if record already exists.
    $sql = "SELECT count(*) as c1 FROM chado.featureprop
      WHERE feature_id = :feature_id AND type_id = :type_id;";
    $results = chado_query($sql, [
      ':feature_id' => $feature_id,
      ':type_id' => $forward_sequence_cvterm_id,
    ]);
    $count = $results->fetchObject()->c1;
    // If record not found.
    if ($count == 0) {
      // Add to record to featureprop table.
      $records['featureprop'][$feature_id . $forward_sequence_cvterm_id] = [
        'feature_id' => $feature_id,
        'type_id' => $forward_sequence_cvterm_id,
        'value' => $line_rows[$columns['forward_sequence']],
      ];
    }

    // Check if reverse sequence information has been added.
    $reverse_sequence_cvterm_id = NULL;
    // Get cvterm_id (assuming it exists)
    $results = chado_query("SELECT * FROM chado.cvterm
      WHERE name = 'three_prime_flanking_region' LIMIT 1;", []);
    foreach ($results as $row) {
      $reverse_sequence_cvterm_id = $row->cvterm_id;
    }

    // Check if record already exists.
    $sql = "SELECT count(*) as c1 FROM chado.featureprop
      WHERE feature_id = :feature_id AND type_id = :type_id;";
    $results = chado_query($sql, [
      ':feature_id' => $feature_id,
      ':type_id' => $reverse_sequence_cvterm_id,
    ]);
    $count = $results->fetchObject()->c1;
    // If record not found.
    if ($count == 0) {
      // Add record to featureprop table.
      $records['featureprop'][$feature_id . $reverse_sequence_cvterm_id] = [
        'feature_id' => $feature_id,
        'type_id' => $reverse_sequence_cvterm_id,
        'value' => $line_rows[$columns['reverse_sequence']],
      ];
    }
  }

}
