<?php

/**
 * @file
 * Contains 'AssayDesign' class.
 *
 * Specific implementation:
 *
 * This fiels uses more then one group (set) of required columns.
 * Also it extends FileField class.
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
  const TITLE = 'Assay Design';
  // Name of the fieldset on PAGE 4.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  const DATA_TYPE = 'genotype';
  // HTML name of the field.
  const DOM_NAME = 'assay-design';
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
  // Mapping of the old column names with new options.
  // - 'chr' = 'Scaffold'
  // - 'Position' = 'position'
  // - 'SNP ID' = 'marker name'
  // - 'forward sequence' = 'Flank sequence'
  // - 'reverse sequence' = 'Reverse Flank Sequence'.
  // Extra items which uses
  // 'snp': $options['file_columns']['snp']; ? Is it 'SNP ID'?
  const DATA_TYPE_SNP_ID = 1;
  const DATA_TYPE_SCAFFOLD = 2;
  const DATA_TYPE_POSITION = 3;
  const DATA_TYPE_FLANK_SEQUENCE = 4;
  const DATA_TYPE_REVERSE_FLANK_SEQUENCE = 5;
  const DATA_TYPE_QUALITY_SCORE = 6;

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
      'parents' => static::getParents($organism_index, $form_state),
      'field_name' => static::DOM_NAME,
      'title' => t(static::TITLE),
      'organism_name' => $organism_key,
      'type' => static::DATA_TYPE,
      'optional' => TRUE,
      // Required when visible.
      'states' => [
        'visible' => $states,
        // Doesn't works as expected. Field is always required. Probably because
        // 'required_when_visible' feature doesn't support complex rules.
        // See tpps_form_validate_required_field().
        // 'required' => $states,
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
   * Validation rules:
   *  For ‘SNP Association' file:
   *   1. Optional and shouldn't be required to process ‘Assay Design’ file.
   *   2. Column 'SNP_ID' must match with 'SNP_ID' column in 'Assay Design' file.
   *   3. The columns ‘Associated trait’ should only contain values also present
   *      in the ‘Phenotype’ file.
   *      Trait names in a phenotype value are header values.
   *
   * For ‘SNP Assay' file:
   *  1. Required to process 'Assay Design' file.
   *  2. Column 'SNP_ID' must match with column 'SNP_ID' in 'Assay Design' file.
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
    // Do NOT allow empty field validation.
    $fid = static::getFileId($organism_index, $form_state);
    if (form_get_errors() || empty($fid)) {
      return FALSE;
    }

    // @todo Implement:
    // Do not allow the same data type for multiple columns.
    // static::allowColumnDataTypeUseOnce($organism_index, $groups);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if required data types for columns was set.
    // Note: Assay Design has more then one group of required columns.
    if (!static::hasColumns($organism_index, $form_state)) {
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'Data types for columns must be set.',
        [
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
      return FALSE;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // 'SNP Assay' is required.
    if (empty(SnpAssay::getFileId($organism_index, $form_state))) {
      // @todo Minor. Highlight 'SNP Assay' file.
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . 'Requires "SNP Assay" file to be processed.',
        [
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Validate column data types.
    static::validateColumns($organism_index, $form_state);
    if (form_get_errors()) {
      return;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if (SnpAssociation::getFileId($organism_index, $form_state)) {
      // Match 'SNP_ID' column with 'SNP_ID' column in 'SNP Association' file.
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
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
    $params = ['@title' => static::TITLE];
    if ($debug_mode) {
      $message = t('"@title": Started file processing.', $params);
      tpps_log_header($message);
    }
    // Check if file was already processed.
    // WARNING: This extra check was missing in submit_all.php script.
    if (static::wasProcessed($organism_index, $shared_state)) {
      $message = '"@title": File was already processed.';
      tpps_log($message, $params, TRIPAL_DEBUG);
      return;
    }
    $fid = static::getFileId($organism_index, $shared_state);
    if (empty($fid)) {
      if ($debug_mode ?? FALSE) {
        $message = '"@title": File Id is empty.';
        tpps_log($message, $params, TRIPAL_DEBUG);
      }
      return;
    }
    tpps_add_project_file($shared_state, $fid);
    // Note: currently not used.
    $options['selected_columns'] = static::getSelectedColumns($organism_index, $shared_state);

    if ($debug_mode) {
      $message = t('"@title": Complete file processing.', $params);
      tpps_log_header($message);
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

    // Note: new simplier structure was used for the user-selected data types
    // for file columns.
    // The format of the $columns array:
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
        $parents = [
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          $dom_name ?? static::DOM_NAME,
        ];
        break;

      case 3:
      default:
        $parents = [
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::PARENT_FIELDSET,
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
   * Note:
   * To get name by key: (static::getColumnOptions()[$data_type_key] ?? NULL).
   *
   * @return array
   *   Returns list of options with column data types.
   */
  public static function getColumnOptions() {
    // List of data types to be used in the dropdown list from Meghan.
    // https://plantcompgenomics.slack.com/archives/C05R396U4QK/p1730737064312679?thread_ts=1730430961.236099&cid=C05R396U4QK
    // 'Qual' option:
    // https://plantcompgenomics.slack.com/archives/C05R396U4QK/p1762254283851269
    return [
      0 => t('N/A'),
      static::DATA_TYPE_SNP_ID => t('SNP ID'),
      static::DATA_TYPE_SCAFFOLD => t('Scaffold'),
      static::DATA_TYPE_POSITION => t('Position'),
      static::DATA_TYPE_FLANK_SEQUENCE => t('Flank sequence'),
      self::DATA_TYPE_REVERSE_FLANK_SEQUENCE => t('Reverse Flank sequence'),
      self::DATA_TYPE_QUALITY_SCORE => t('Quality Score'),
    ];
  }

  /**
   * Gets required columns.
   *
   * This is newer version which allows a sets of columns
   * (not only one flat list of the required columns).
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
        $data = SubmissionCache::getAll($reset);
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


  /**
   * This function validates that the file contains all of the required groups.
   *
   * Based on code from function tpps_file_validate_columns() but allows
   * multiple sets of required columns.
   * To use this method class must implement 'getRequiredColumns' method and
   * allow multiple sets of required columns.
   *
   * WARNING:
   * - Not compatible with tpps_file_validate_columns().
   * - Allows multiple required groups.
   * - Uses different structure in getColumnOptions().
   * - Uses diferent output structure (single level instead of nested array).
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   * @param array $form
   *   Drupal Form API Array.
   *
   * @return array
   *   Returns mapping of the column letters (starting from A) as keys and
   *   data type keys as values.
   *   Example: ['A' => 1, 'B' => '2'].
   *
   * @todo Merge with FileField::validateColumns().
   */
  public static function validateColumns($organism_index, array &$form_state, array $form = NULL) {
    $debug_mode = FALSE;

    if (!static::isMultipleRequiredColumns()) {
      return FileField::validateColumns($organism_index, $form_state, $form);
    }

    // @todo Check if validation and processing uses new structure of the
    // required columns and remove array_flip() if possible.

    if (!method_exists(static::class, 'getRequiredColumns')) {
      // Method shouldn't be called if there is no required columns so it's not
      // an exception but probably method 'getRequiredColumns' wasn't implemented
      // by mistake and watchdog-message is enough here.
      $message = 'Method "getRequiredColumns" is required for method "@method".';
      $params = ['@method' => __METHOD__];
      watchdog('tpps', $message, $params);
      return;
    }
    $required_cases = static::getRequiredColumns();
    $parents = static::getParents($organism_index, $form_state);

    $fid = TppsArray::getValue($form_state['input'],
      array_merge($parents, ['fid'])
    );
    if (empty($file = tpps_file_load($fid)) || empty($file->filesize)) {
      $error_message = t(
        'Organism #@oidx / "<strong>@field_title</strong>": @message',
        [
          '@field_title' => static::getFieldTitle(),
          '@oidx' => $organism_index,
          '@message' => 'File is empty.',
        ]
      );
      // We highlight an empty value field because file field not visible
      // when file was uploaded.
      // $file_element['#name'] . '[empty]',
      // There is no 'empty' element for file fields.
      form_set_error(
        static::getFormErrorName($organism_index),
        $error_message
      );
      // No need to validate empty file.
      return [];
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get user selected data types for columns.
    // Example of structure: ['A' => '1', 'B' => '4',];
    // Note: there is no sense to use FileField::getSelectedColumns() here
    // because it uses different structure of output array.
    $selected_columns = TppsArray::getValue(
      $form_state['input'],
      array_merge($parents, ['columns'])
    );
    if (empty($selected_columns)) {
      // Note: $selected_columns will be empty when user just added file (not clicked 'Upload'
      // button and submitted form (clicked 'Next' button).
      // Probably columns could be detected automatically because they are shown
      // at page and so we could use default values here.
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . "Data types for columns wasn't specified.",
        [
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,

        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
      return [];
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // No need to storey list of options under $form_state['saved_values']
    // because it's a constant and will not be changed.
    $column_options = static::getColumnOptions();
    // @todo Think if we should store list of options? If this list will be
    // changed in future.

    if ($debug_mode ?? FALSE) {
      dpm(print_r($column_options, 1), 'options');
      dpm(print_r($selected_columns, 1), 'selected');
      dpm($fid, 'fid');
    }
    // Store under $form_state['saved_values'].
    TppsArray::setValue(
      $form_state['saved_values'][static::FORM_STEP],
      $parents,
      TppsArray::getValue($form_state['input'], $parents)
    );

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if the same data type not used twice.
    static::allowColumnDataTypeUseOnce($organism_index, $selected_columns);
    if (form_get_errors()) {
      return [];
    }
    $columns = [];
    foreach ($required_cases as $case_name => $case_data) {

      // Since file can contain extra columns we can't check if lists are equal
      // but must check if $selected_columns list includes all required columns.
      if (TppsArray::contains($case_data, $selected_columns)) {
        // Values are equal.
        $columns = $case_data;
        break;
      }
    }

    // Report about falied validation of the equired groups of the data types.
    if (empty($columns)) {
      foreach ($required_cases as $case_name => $case_data) {
        $required_print_list[] = implode(', ', static::getDataType($case_data)) . '.';
      }
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . "Minimal required data types for columns not set."
        . "<br />Selected data types are: <strong>!selected_columns</strong>"
        . "<br /> There are @group_number possible minimal data type options; "
        . "you must set at least columns containing all data types in one of "
        . "the following groups: <strong>!required_cases</strong>",
        [
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
          '!selected_columns' => implode(', ', static::getDataType($selected_columns)),
          '!required_cases' => theme('item_list', ['items' => $required_print_list]),
          '@group_number' => static::countRequiredColumns(),
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
    }

    // See tpps_file_validate_columns().
    // Format: ['1' => 'A', '4' => 'B'].
    return array_flip($selected_columns);
  }

}
