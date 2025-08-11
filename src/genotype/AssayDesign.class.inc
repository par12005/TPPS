<?php

/**
 * @file
 * Contains 'AssayDesign' class.
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
class AssayDesign {

  // Machine name of the 'SNPs' fieldset (parent of the parent).
  const SNPS_FIELDSET = 'SNPs';
  // Name of the fieldset on PAGE 4.
  const DATA_TYPE = 'genotype';
  const PARENT_FIELDSET = AssayDesignCitation::PARENT_FIELDSET;
  // HTML name of the field.
  const FIELD_NAME = 'assay-design';
  const FIELD_TITLE = 'Assay Design File';
  // Integer Form Page number.
  const PAGE = TPPS_PAGE_4;

// @TODO Review.
  // From VS/assay_design_file_columns branch.
  if (0) {
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
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Custom settings.
  //
  // Data types for field columns.
  const DATA_TYPE_SNP_ID = 1;
  const DATA_TYPE_SCAFFOLD = 2;
  const DATA_TYPE_POSITION = 3;
  const DATA_TYPE_FLANK_SEQUENCE = 4;

  /**
   * Builds set of form fields for 'Assay Design Citation' feature.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
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
      ':input[name="' . $organism_key . '[' . implode('][', [
        AssayDesign::DATA_TYPE,
        AssayDesign::SNPS_FIELDSET,
        AssayDesignCitation::PARENT_FIELDSET,
        AssayDesignCitation::EXISTING_STUDY_FIELD_NAME,
      ]) . ']"]' => [
        'value' => AssayDesignCitation::EXISTING_STUDY_OTHER_OPTION_KEY,
      ],
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // We should also check this field because when 'genotyping-type' set into
      // 'Genotyping Array' field 'file-type' disapears with value which allows
      // this field to be shown and required.
      ':input[name="' . $organism_key . '[' . implode('][', [
        AssayDesign::DATA_TYPE,
        AssayDesign::SNPS_FIELDSET,
        'genotyping-type',
      ]) . ']"]' => ['value' => TPPS_GENOTYPING_TYPE_GENOTYPING],
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      ':input[name="' . $organism_key . '[' . implode('][', [
        AssayDesign::DATA_TYPE,
        AssayDesign::SNPS_FIELDSET,
        'file-type',
      ]) . ']"]' => [
        'value' => TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE,
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
      'field_name' => static::FIELD_NAME,
      'title' => t(static::FIELD_TITLE),
      'organism_name' => $organism_key,
      'type' => static::DATA_TYPE,
      'optional' => TRUE,
      // Required when visible.
      'states' => [
        'visible' => $states,
        'required' => $states,
      ],
    ]);
  }

  /**
   * Validates set of form fields for 'Assay Design Citation' feature.
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
    $snps = tpps_array_get_value(
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
    $existing_study = AssayDesignCitation::getExistingStudy($organism_index, $form_state);
    if (
      $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
      && $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING
      && $existing_study == AssayDesignCitation::EXISTING_STUDY_OTHER_OPTION_KEY
    ) {
      //dpm('required');
      $fid = tpps_array_get_value(
        $form_state['values'],
        [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          AssayDesignCitation::EXISTING_STUDY_FIELD_NAME,
          static::FIELD_NAME,
        ]
      );
      if ($fid) {
        // Preserve file if it is valid.
        tpps_preserve_valid_file(
          $form_state,
          $fid,
          $organism_index,
          'Genotype_Assay_Design'
        );
      }
    }
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
      'organism-' . $organism_index,
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
      ['@headers' => print_r($options['headers'], 1)], TRIPAL_DEBUG
    );

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // WARNING: new simple format of the selected columns are in use.
    $selected_columns = TppsArray::getValue($shared_state, [
      'saved_values',
      static::FORM_STEP,
      'organism-' . $organism_index,
      static::DATA_TYPE,
      static::FORM_PARENT_FIELDSET,
      // For searching purposes: 'assay-design-columns'
      static::FIELD_NAME . '-columns'
    ]);
    if ($debug_mode) {
      tpps_log_line();
      tpps_log($selected_columns, 'selected_columns');
      //tpps_log($required_columns, 'required_columns');
      //tpps_log($options['headers'], 'header');
    }
    $options['selected_columns'] = $selected_columns;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// @TODO Review this code. It was in submit_all.php but was missing in
// VS/assay_design_file_columns branch. Maybe in processRow()?
    if (0) {
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
    }


    // We want to process this Genotype SNP Assay Design file before
    // we add it as a project file.
    tpps_log('Processing "Assay Design" file data...');
    tpps_file_iterator($fid, 'AssayDesign::processRow', $options);
    tpps_log('Done.');

    tpps_log('Inserting "Assay Design" data into database using insert_multi...');


// @TODO Insert data. Was no code here.


    tpps_log('Done.');
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
    // Format: keys are data type keys and values are column letters.
    $columns = array_flip($options['selected_columns']);
    if ($debug_mode) {
      tpps_log($columns, static::TITLE . " file selected column's data types");
    }

    $line_rows = $row;
    // Needed to lookup source features:
    $analysis_id = $options['analysis_id'];
    $headers = &$options['headers'];
    $records = &$options['records'];
    $columns = $options['file_columns'];
    $organism_index = $options['organism_index'];
    $seq_var_cvterm = $options['seq_var_cvterm'];
    if ($debug_mode ?? FALSE) {
      print_r("File columns: ");
      print_r($columns);
      print_r("\n");
      print_r("Data in row:");
      print_r($line_rows);
      print_r("\n");
    }

    // $chr_name = $line_rows[$columns['chr']];
    $chr_name = $row[static::DATA_TYPE_SCAFFOLD];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Note: This code was commented out in submit_all.php.
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

    $marker_name_raw = $line_rows[$columns['marker_name']];
    //$position = intval($rows[$columns['position']]);
    $position = intval($row[static::DATA_TYPE_POSITION]);
    $marker_type = $options['marker_type']; // we could force 'SNP' here

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

    // $row[$columns[static::DATA_TYPE_SNP_ID]]  = $line_rows[$columns['marker_name']];
    $marker_name = $row[$columns[static::DATA_TYPE_SNP_ID]] . $marker_type;

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
      if (empty($feature_id)) {
        tpps_log("Marker name '@marker_name' wasn't found in table "
          . "'chado.feature' after insert.", ['@marker_name' => $marker_name],
          TRIPAL_ERROR
        );
        return;
      }
    }
    if (empty($feature_id)) {
      tpps_log("Marker name $marker_name feature_id could not be found.", [], TRIPAL_ERROR);
      return;
    }

    tpps_log("Marker '$marker_name' has feature_id: '$feature_id'.");

    // Before we add a new featureloc record, check to make sure one does not already exist
    // in the featureloc table since we don't currently delete previous featurelocs on
    // study reloads.
    $featureloc_results = chado_query('SELECT count(*) as c1 FROM chado.featureloc
      WHERE feature_id = :feature_id AND srcfeature_id = :srcfeature_id;', [
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
      $read_length = 0; // default for non-indels
      foreach ($snp_possible_reads as $read) {
        $tmp_read_length = strlen($read);
        if ($tmp_read_length > 1) { // only for indels, we need a read_length of more than 0
          if ($tmp_read_length > $read_length) {
            $read_length = $tmp_read_length;
          }
        }
      }

      // TODO: if read_length is more than 1, it is an indel, change marker type
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
      tpps_log("[GOOD ALTERNATIVE] Featureloc record already exists, no need to add");
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
          // 'value' => $rows[$columns['forward_sequence']],
          'value' => $row[$columns[self::DATA_TYPE_FLANK_SEQUENCE]],
        ];
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Add only when column is not empty and exists.
    // @TODO Minor. Update 'reverse_sequence' with actual column data type.
    if (!empty($row[$columns['reverse_sequence']])) {
      // Check if reverse sequence information has been added.
      // Value: 1825.
      $reverse_sequence_cvterm_id = tpps_load_cvterm('three_prime_flanking_region')->cvterm_id;
      // Check if record already exists.
      $featureprop_id = db_select('chado.featureprop', 'cfp')
        ->fields('cfp', 'featureprop_id')
        ->condition('feature_id', $feature_id)
        ->condition('type_id', $reverse_sequence_cvterm_id)
        ->range(0, 1)->execute()->fetchField();
      // If record not found.
      if (empty($featureprop_id)) {
        // Add record to featureprop table.
        $records['featureprop'][$feature_id . $reverse_sequence_cvterm_id] = [
          'feature_id' => $feature_id,
          'type_id' => $reverse_sequence_cvterm_id,
          // @TODO Use class' constant.
          'value' => $row[$columns['reverse_sequence']],
        ];
      }
    }
  }

  /**
   * Gets field's value.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns integer File Id.
   *   Returns NULL if field wasn't found.
   *   Returns string '0' if field wasn't set (file wasn't uploaded).
   */
  public static function getValue($organism_index, array $form_state) {
    $parents = static::getParents($organism_index, $form_state);
    return tpps_array_get_value($form_state['values'], $parents);
  }

  /**
   * Alias for getValue() method.
   *
   * @todo Move to TppsFile class.
   */
  public static function getFileId($organism_index, array $form_state) {
    return static::getValue($organism_index, $form_state);
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
          static::FIELD_NAME,
        ];
        break;

      case 2:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::FIELD_NAME,
        ];
        break;

      case 3:
      default:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::PARENT_FIELDSET,
          static::FIELD_NAME,
        ];
        break;
    }
    return $parents;
  }

  /**
   * Removes field's file by File Id.
   *
   * WARNING: for validation only.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function removeFile($organism_index, array &$form_state) {
    $fid = static::get($organism_index, $form_state);
    if (tpps_file_remove($fid)) {
      $parents = static::getParents($organism_index, $form_state);
      tpps_array_set_value($form_state['values'], $parents, $value = 0);
    }
  }

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
          if (empty($cached_submission[SubmissionCache::FILES_CONTAINER][static::FIELD_NAME])) {
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
