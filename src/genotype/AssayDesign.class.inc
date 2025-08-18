<?php

/**
 * @file
 * Contains 'AssayDesign' class.
 *
 * Specific implementation:
 *
 * This fiels uses more then one group (set) of required columns.
 * Also it extends FileField (not TppsFile) class.
 * Instead of $options['file_columns'] class uses $options['selected_columns']
 *   which comes directly from 'assay-design-columns' element which holds
 *   user's selection. Other fields are using very complex method of getting
 *   this data which is overkill for this task.
 */

/**
 * Manages 'Assay Design' file field.
 *
 * TPPSc Form / Page 4 / Genotype / SNPs.
 *
 * Field relocation history:
 * v1: ['genotype', 'files', 'assay-design'].
 * v2: -> ['genotype', 'SNPs', 'assay-design'].
 * v3: -> ['genotype', 'SNPs', 'assay-design-citation', 'assay-design'].
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class AssayDesign extends FileField {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Required constants.
  //
  // Name of the fieldset on PAGE 4.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  const DATA_TYPE = 'genotype';
  // HTML name of the field.
  const DOM_NAME = 'assay-design';
  const TITLE = 'Assay Design File';
  // Integer Form Page number.
  const FORM_STEP = TPPS_PAGE_4;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Optional constants. Class specific.
  //
  // Machine name of the 'SNPs' fieldset (parent of the parent).
  const SNPS_FIELDSET = 'SNPs';
  // Direct parent element DOM-name.
  const PARENT_FIELDSET = AssayDesignCitation::PARENT_FIELDSET;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Custom settings.
  //
  // Data types for field columns.
  const DATA_TYPE_SNP_ID = 1;
  const DATA_TYPE_SCAFFOLD = 2;
  const DATA_TYPE_POSITION = 3;
  const DATA_TYPE_FLANK_SEQUENCE = 4;
  const DATA_TYPE_REVERSE_FLANK_SEQUENCE = 5;

  /**
   * Builds set of form fields for 'Assay Design Citation' feature.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($organism_index, array &$form, array $form_state) {
    $organism_key = 'organism-' . $organism_index;
    // @TODO Get parents from classes using getParents() method to respect
    // Submission's form version.
    $states = [
      // Existing studies with an "Assay design file":
      // 'The citation for my assay design is not in this list'.
      [
        // 'does_study_include_snp_data': 'yes'.
        ':input[name="' . $organism_key . '[' . implode('][', [
          AssayDesign::DATA_TYPE,
          'does_study_include_snp_data',
        ]) . ']"]' => ['value' => "yes"],
        ':input[name="' . $organism_key . '[' . implode('][', [
          AssayDesign::DATA_TYPE,
          AssayDesign::SNPS_FIELDSET,
          AssayDesignCitation::PARENT_FIELDSET,
          AssayDesignCitation::EXISTING_STUDY_FIELD_NAME,
        ]) . ']"]' => [
          'value' => AssayDesignCitation::EXISTING_STUDY_OTHER_OPTION_KEY,
        ],
      ],
      'or',
      [
        // 'does_study_include_snp_data': 'yes'.
        ':input[name="' . $organism_key . '[' . implode('][', [
          AssayDesign::DATA_TYPE,
          'does_study_include_snp_data',
        ]) . ']"]' => ['value' => "yes"],
        // 'Genotyping Type': 'Genotyping'
        // We should also check this field because when 'genotyping-type' set into
        // 'Genotyping Array' field 'file-type' disapears with value which allows
        // this field to be shown and required.
        ':input[name="' . $organism_key . '[' . implode('][', [
          AssayDesign::DATA_TYPE,
          AssayDesign::SNPS_FIELDSET,
          'genotyping-type',
        ]) . ']"]' => ['value' => TPPS_GENOTYPING_TYPE_GENOTYPING],
        // 'Genotyping file type': 'SNP Assay file and Assay design file'
        ':input[name="' . $organism_key . '[' . implode('][', [
          AssayDesign::DATA_TYPE,
          AssayDesign::SNPS_FIELDSET,
          'file-type',
        ]) . ']"]' => [
          'value' => TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE,
        ],
      ],
    ];
    // Add file upload field.
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => [
        $organism_key,
        static::DATA_TYPE,
        static::SNPS_FIELDSET,
        static::PARENT_FIELDSET,
      ],
      'field_name' => static::DOM_NAME,
      'title' => t(static::TITLE),
      'organism_name' => $organism_key,
      'type' => static::DATA_TYPE,
      'optional' => TRUE,
      // Required when visible.
      'states' => [
        'visible' => $states,
        //'required' => $states,
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
          '#value' => static::getColumnOptions(),
        ],
        'no-header' => [],
      ],
    ]);
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

    // Get 'SNPs' fieldset.
    $snps = TppsArray::getValue(
      $form_state['values'],
      [
        $organism_key,
        static::DATA_TYPE,
        static::SNPS_FIELDSET,
      ]
    );

    // Get values of selectors.
    $file_type = $snps['file-type'] ?? NULL;
    $genotyping_type = $snps['genotyping-type'] ?? NULL;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if field is required.
    $existing_study = AssayDesignCitation::getExistingStudy($organism_index, $form_state);
    static::validateRequired($organism_index, $form_state, (
      // Requirement conditions:
      $existing_study == AssayDesignCitation::EXISTING_STUDY_OTHER_OPTION_KEY
      || (
        $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING
        && $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
      )
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
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
    }
    // Do NOT allow further validation.
    if (form_get_errors()) {
      return FALSE;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if required data types for columns was set.
    // Note: Assay Design has more then one group of required columns.
    if (static::hasColumns($organism_index, $form_state)) {
      $error_name = static::getFormErrorName($organism_index);
      if (empty($assoc_file = $snps['snps-association'])) {
        // @todo Minor. Highlight 'SNP Association' file.
        $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
          . 'Requires "SNP Association" file.',
          [
            '@field_title' => t(static::getFieldTitle()),
            '@oidx' => $organism_index,
          ]
        );
        form_set_error(static::getFormErrorName($organism_index), $error_message);
      }
      if (empty($snps_assay = $snps['snps-assay'])) {
        // @todo Minor. Highlight 'SNP Assay' file.
        $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
          . 'Requires "SNP Assay" file.',
          [
            '@field_title' => t(static::getFieldTitle()),
            '@oidx' => $organism_index,
          ]
        );
        form_set_error(static::getFormErrorName($organism_index), $error_message);
      }
      // Validate column data types.
      static::validateColumns($organism_index, $form_state);
      if (form_get_errors()) {
        return;
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Match SNP Ids in current file and 'SNPs Assay' file (header).
      static::matchColumnValues([
        'organism_index' => $organism_index,
        'form_state' => $form_state,
        // Current file.
        'no_header' => static::getSubFieldValue(
          $organism_index,
          $form_state,
          'no-header'
        ),
        'column_letter' => static::getSelectedColumns(
           $organism_index,
          $form_state
        )[static::DATA_TYPE_SNP_ID],
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
      static::matchColumnValues([
        'organism_index' => $organism_index,
        'form_state' => $form_state,
        // Curent file.
        'no_header' => $snps[static::DOM_NAME . '-no-header'] ?? FALSE,
        'column_letter' => static::getSelectedColumns(
          $organism_index,
          $form_state
        )[static::DATA_TYPE_SNP_ID],
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

    // Create new '-columns', '-no-header' and '-empty' fields in 'saved_values'.
    static::storeSubFields($organism_index, $form_state);
    static::preserveValidFile($organism_index, $form_state);
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
    $organism_key = 'organism-' . $organism_index;

    $debug_mode = FALSE;
    if ($debug_mode) {
      tpps_log_header('Started Assay Design file procesing');
    }
    // Check if file was already processed.
    // WARNING: This extra check was missing in submit_all.php script.
    if (static::wasProcessed($organism_index, $shared_state)) {
      $message = '"@file_title" file was already processed.';
      tpps_log($message, ['@file_title' => static::TITLE], TRIPAL_DEBUG);
      return;
    }
    // This if statement caters for the Genotype Assay Design file
    // (which holds extra data like positions etc)
    // This is usually also accompanied with the Genotype SNP Assay
    // (which holds the snps)
    // We want to insert location data into the database
    // if both are found and the marker-type is snp
    // The previous step took care of the SNPs insertion via the Genotype
    // SNP Assay (not to be confused with genotype SNP assay design file)
    if (empty($fid = static::getFileId($organism_index, $shared_state))) {
      return;
    }
    tpps_add_project_file($shared_state, $fid);

    $genotype = TppsArray::getValue($shared_state, [
      'saved_values',
      static::FORM_STEP,
      $organism_key,
      static::DATA_TYPE,
    ]);

    // Setup the options array which the tpps_file_iterator custom function
    // will be able to access necessary details.
    $options['organism_index'] = $organism_index;
    $options['type'] = 'snp';
    $options['marker'] = 'SNP';
    $options['type_cvterm'] = tpps_load_cvterm('snp')->cvterm_id;
    $options['ref-genome'] = $genotype['ref-genome'];
    $ref_genome = $genotype['ref-genome'];
    tpps_log($ref_genome, 'Ref-genome');
    // Lookup analysis id from reference genome and add it to options array.
    if (!empty($ref_genome)) {
      $options['analysis_id'] = tpps_get_analysis_id_from_ref_genome($ref_genome);
      tpps_log($options['analysis_id'], 'Analysis Id');
    }
    // analysis id is required  to tie back to the srcfeature.
    if (empty($options['analysis_id'])) {
      tpps_log('Analysis ID could not be found, skipping assay design file processing.',
        [], TRIPAL_ERROR
      );
      return;
    }

    // Initialize new records with featureloc array to store records.
    $options['records']['featureloc'] = [];
    $options['records']['featureprop'] = [];
    $options['headers'] = tpps_file_headers($fid);
    tpps_log("HEADERS:\n@headers\n",
      ['@headers' => print_r($options['headers'], 1)],
      TRIPAL_DEBUG
    );

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // WARNING:
    // New simple format of the selected columns are in use.
    $selected_columns = TppsArray::getValue($shared_state, [
      'saved_values',
      static::FORM_STEP,
      $organism_key,
      static::DATA_TYPE,
      static::SNPS_FIELDSET,
      // For searching purposes: 'assay-design-columns'.
      static::DOM_NAME . '-columns',
    ]);
    if ($debug_mode) {
      tpps_log_line();
      tpps_log($selected_columns, 'selected_columns');
    }
    $options['selected_columns'] = $selected_columns;
    // Mapping of the old column names with new options.
    // - 'chr' = 'Scaffold'
    // - 'Position' = 'position'
    // - 'SNP ID' = 'marker name'
    // - 'forward sequence' = 'Flank sequence'
    // - 'reverse sequence' = 'Reverse Flank Sequence'.
    // Extra items which uses
    // 'snp': $options['file_columns']['snp']; ? Is it 'SNP ID'?

    // We want to process this file before we add it as a project file.
    tpps_log('Processing "Assay Design" file data...');
    tpps_file_iterator($fid, 'AssayDesign::processRow', $options);
    tpps_log('Done.');

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Process data from Assay Design file field.
    //
    // There is no need to insert Assay Design file's data into database
    // because this file will be used to build VCF file.

    // Reset options[records] with empty records arrays.
    $options['records'] = [
      'feature' => [],
      'genotype' => [],
      'genotype_call' => [],
      'stock_genotype' => [],
    ];
    if ($debug_mode) {
      tpps_log_header('Ended Assay Design file procesing');
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
   *   Keys are column letters and value is a cell value. Example of structure:
   *   [
   *     [A] => SNP ID
   *     [B] => Scaffold
   *     [C] => Position
   *     [D] => Flank sequence
   *   ].
   * @param array $options
   *   Additional options set when calling tpps_file_iterator().
   */
  public static function processRow($row, array &$options = []) {
    $debug_mode = FALSE;
    // Needed to lookup source features.
    $analysis_id = $options['analysis_id'];
    $headers = &$options['headers'];
    $records = &$options['records'];
    $organism_index = $options['organism_index'];
    $seq_var_cvterm = $options['seq_var_cvterm'];

    // Format of the $columns array:
    // - key is the data type key (not name),
    // - value is the column letter.
    $columns = array_flip($options['selected_columns']);
    if ($debug_mode) {
      tpps_log($columns, static::TITLE . " file selected column's data types");
    }
    $chr_name = $row[$columns[self::DATA_TYPE_SCAFFOLD]];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // If the scaffold is only the number then append scaffold_ to it (TGDR665).
    // Note: This code was commented out in submit_all.php.
    // Disabled by default.
    $use_scaffold_prefix = FALSE;
    if ($use_scaffold_prefix) {
      if (is_numeric($chr_name)) {
        $chr_name = "scaffold_" . $chr_name;
      }
      // @TODO Minor. It's safe just to run str_replace() without extra checks.
      // If TGDR665, replace 'chr' with 'scaffold_'.
      if (substr($chr_name, 0, 3) == 'chr') {
        $chr_name = str_replace('chr', 'scaffold_', $chr_name);
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $position = intval($row[$columns[self::DATA_TYPE_POSITION]]);

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
      if ($count_tmp == $options['organism_index']) {
        $species_code = $species_code_tmp;
        $organism_id = $organism_id_tmp;
        break;
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get the srcfeature_id.
    $query = db_select('chado.feature', 'cf')
      ->fields('cf', ['feature_id'])
      ->condition('cf.name', $chr_name);
    $query->join('chado.analysisfeature', 'caf', 'cf.feature_id = caf.feature_id');
    $query->condition('caf.analysis_id', $analysis_id);
    $srcfeature_id = $query->range(0, 1)->execute()->fetchField();

    if (empty($srcfeature_id)) {
      tpps_log("srcfeature_id for '@name' could not be found - we can't add "
        . "featureloc data.", ['@name' => $chr_name], TRIPAL_ERROR
      );
      return;
    }
    tpps_log('srcfeature_id for "@name": @id.',
      ['@name' => $chr_name, '@id' => $srcfeature_id]
    );

    $marker_name = $row[$columns[static::DATA_TYPE_SNP_ID]] . $options['marker_type'];

    // We need to find the current marker_name in the feature table.
    // @TODO Ask if we could safely insert without extra checks.
    $feature_id = db_select('chado.feature', 'cf')
      ->fields('cf', ['feature_id'])
      ->condition('uniquename', $marker_name)
      ->range(0, 1)
      ->execute()
      ->fetchField();

    if (empty($feature_id)) {
      // We should add the marker (marker_name).
      chado_insert_record('feature', [
        'name' => $marker_name,
        'organism_id' => $organism_id,
        'uniquename' => $marker_name,
        'type_id' => $seq_var_cvterm,
      ]);
      // Recheck for the feature_id.
      // @TODO Replace with db_insert() which is much faster and allows
      // to get last inserted id.
      $feature_id = db_select('chado.feature', 'cf')
        ->fields('cf', ['feature_id'])
        ->condition('uniquename', $marker_name)
        ->range(0, 1)
        ->execute()
        ->fetchField();
    }
    if (empty($feature_id)) {
      tpps_log("Marker name '@marker_name' wasn't found in table "
        . "'chado.feature' after insert.", ['@marker_name' => $marker_name],
        TRIPAL_ERROR
      );
      return;
    }

    tpps_log("Marker '$marker_name' has feature_id: '$feature_id'.");

    // Before we add a new featureloc record, check to make sure one
    // does not already exist in the featureloc table since we don't
    // currently delete previous featurelocs on study reloads.
    $featureloc_id = db_select('chado.featureloc', 'cfl')
      ->fields('cfl', 'featureloc_id')
      ->condition('feature_id', $feature_id)
      ->condition('srcfeature_id', $srcfeature_id)
      ->range(0, 1)->execute()->fetchField();

    // This means no featureloc exists, so insert it.
    if (!empty($featureloc_id)) {
      tpps_log("[GOOD ALTERNATIVE] Featureloc record already exists, no need to add.");
    }
    elseif (!empty($rows[$columns[static::DATA_TYPE_SNP_ID]])) {
      // Check for indels (longer than 1 reads)
      $snp = trim($rows[$columns[static::DATA_TYPE_SNP_ID]]);
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
      // @todo If read_length is more than 1, it is an indel, change marker type.
      // Also check code in tpps_process_genotype_sheet to check this
      // or else these 2 things will cause a submit failure - after
      // conference (6/15/2023).
      $records['featureloc'][$marker_name] = [
        'fmin' => $position,
        'fmax' => ($position + $read_length),
        'srcfeature_id' => $srcfeature_id,
        'feature_id' => $feature_id,
      ];
      print_r($records['featureloc'][$marker_name]);
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Add only when column is not empty and exists.
    if (!empty($row[$columns[static::DATA_TYPE_FLANK_SEQUENCE]])) {
      // Value: 1824.
      $forward_sequence_cvterm_id = tpps_load_cvterm('five_prime_flanking_region')->cvterm_id;
      // Check if record already exists.
      $featureprop_id = db_select('chado.featureprop', 'cfp')
        ->fields('cfp', ['featureprop_id'])
        ->condition('feature_id', $feature_id)
        ->condition('type_id', $forward_sequence_cvterm_id)
        ->range(0, 1)->execute()->fetchField();
      // If record not found.
      if (empty($featureprop_id)) {
        // Add to record to featureprop table.
        $records['featureprop'][$feature_id . $forward_sequence_cvterm_id] = [
          'feature_id' => $feature_id,
          'type_id' => $forward_sequence_cvterm_id,
          'value' => $row[$columns[static::DATA_TYPE_FLANK_SEQUENCE]],
        ];
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Add only when column is not empty and exists.
    if (!empty($row[$columns[static::DATA_TYPE_REVERSE_FLANK_SEQUENCE]])) {
      // Check if reverse sequence information has been added.
      // Value: 1825.
      $reverse_sequence_cvterm_id = tpps_load_cvterm('three_prime_flanking_region')->cvterm_id;
      // Check if record already exists.
      $featureprop_id = db_select('chado.featureprop', 'cfp')
        ->fields('cfp', ['featureprop_id'])
        ->condition('feature_id', $feature_id)
        ->condition('type_id', $reverse_sequence_cvterm_id)
        ->range(0, 1)->execute()->fetchField();
      // If record not found.
      if (empty($featureprop_id)) {
        // Add record to featureprop table.
        $records['featureprop'][$feature_id . $reverse_sequence_cvterm_id] = [
          'feature_id' => $feature_id,
          'type_id' => $reverse_sequence_cvterm_id,
          'value' => $row[$columns[static::DATA_TYPE_REVERSE_FLANK_SEQUENCE]],
        ];
      }
    }
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
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::DOM_NAME,
        ];
        break;

      case 3:
      default:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::PARENT_FIELDSET,
          static::DOM_NAME,
        ];
        break;
    }
    return $parents;
  }

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
      static::DATA_TYPE_SNP_ID => t('SNP ID'),
      static::DATA_TYPE_SCAFFOLD => t('Scaffold'),
      static::DATA_TYPE_POSITION => t('Position'),
      static::DATA_TYPE_FLANK_SEQUENCE => t('Flank sequence'),
      self::DATA_TYPE_REVERSE_FLANK_SEQUENCE => t('Reverse Flank sequence'),
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
        static::DATA_TYPE_SNP_ID,
        static::DATA_TYPE_SCAFFOLD,
        static::DATA_TYPE_POSITION,
      ],
      'case 2' => [
        static::DATA_TYPE_SNP_ID,
        static::DATA_TYPE_FLANK_SEQUENCE,
      ],
      'case 3' => [
        self::DATA_TYPE_SNP_ID,
        self::DATA_TYPE_REVERSE_FLANK_SEQUENCE,
      ],
    ];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // List of Submissions which has 'Assay Design' file.
  // This list used by 'Assay Design Citation' feature.

  /**
   * Clears cached list of studies which has AssayDesign files.
   */
  public static function clearSubmissionList() {
    $key = 'AssayDesign::getSubmissionList';
    // Clear cache in db.
    $cid = $key;
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    $cache = cache_get($cid, $cache_bin);
    if (!empty($cache)) {
      $cache->data[$key] = NULL;
      cache_set($cid, $cache->data, $cache_bin);
    }
    // Clear static cache.
    $static_cache = &drupal_static($key);
    $static_cache[$key] = NULL;
  }

  /**
   * Gets list of accessions which has (or doesn't have) 'Assay Design' file.
   *
   * @param bool $has
   *   Filter which allows to get list of submissions which doesn't have
   *   'Assay Design' file when set to FALSE. Default is TRUE.
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *
   * @return array
   *   Returns list of accession as a key and shorten titles as a value.
   */
  public static function getSubmissionList($has = TRUE, $reset = FALSE) {
    $key = __METHOD__;
    $static_cache = &drupal_static($key);
    if ($reset || !isset($static_cache[$key])) {
      // Get value.
      $cid = $key;
      $cache_bin = TPPS_CACHE_BIN ?? 'cache';
      $cache = cache_get($cid, $cache_bin);
      if ($reset || empty($cache) || !isset($cache->data[$key])) {
        $cache->data[$key] = NULL;
        // Get new data.
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        $data = SubmissionCache::get($reset);
        $list = [];
        foreach ($data as $accession => $cached_submission) {
          $title = $cached_submission['publication']['title'] ?? t('No Title');
          $title = (strlen($title) > 97) ? substr($title, 0, 97) . '...' : $title;
          $item = $accession . ' - ' . $title;
          if (empty($cached_submission[SubmissionCache::FILES_CONTAINER][static::DOM_NAME])) {
            $list['has_not'][$accession] = $item;
          }
          else {
            $list['has'][$accession] = $item;
          }
        }
        $cache->data[$key] = $list;
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        cache_set($cid, $cache->data, $cache_bin);
      }
      $static_cache[$key] = $cache->data[$key] ?? NULL;
    }
    if ($has) {
      return $static_cache[$key]['has'] ?? [];
    }
    else {
      return $static_cache[$key]['has_not'] ?? [];
    }
  }

}
