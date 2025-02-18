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
              TPPS_PAGE_4,
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
    $debug_mode = 0;
    $genotype = &$form_state['values']['organism-' . $organism_index][self::DATA_TYPE] ?? NULL;
    $snps = &$genotype[self::FORM_PARENT_FIELDSET] ?? NULL;
    // Fields.
    $file_type = $snps['file-type'] ?? NULL;
    $genotyping_type = $snps['genotyping-type'] ?? NULL;
    $fid = self::getFileId($organism_index, $form_state);

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

    // Check for optional (and required) fields.
    // Empty required field must create error messages and break validation
    // before but to be sure and to allow switch to optional field using UI
    // settings page (or variable) use for both (optional and required) fields.
    if (empty($fid)) {
      return;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check required data types for columns.
    // Note: Usually fields has a single group of required columns
    // but Assay Design has 2 of them.
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


      // @todo There are 2 modes: validate and just get columns for processing.
      $columns = self::validateColumns($organism_index, $form_state, $silent_mode = FALSE);
      if (form_get_errors() || empty($columns)) {
        return;
      }

      // Note: $columns => ['1' => 'A', '4' => 'B'];


      // @TODO Compare 3 files. SNP Id must match.
      // Check that SNP IDs match Genotype Assay.
      //
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Check if number and names of SNPs are the same in current file and in
      // 'SNPs Assay' file.
      $match_result = self::matchColumnValues([
        'organism_index' => $organism_index,
        'form_state' => $form_state,
        'target_parents' => [],

        'no_header' => $snps[self::FIELD_NAME . '-no-header'] ?? FALSE,
        // Target file to compare with.
        'target_fid' => $snps['snps-assay'],
        // Use header/1st line of the target file.
        'is_header' => TRUE,
        'column_letter' => $columns[self::DATA_TYPE_SNP_ID],
      ]);
      if (!$match_result) {
        dpm('not match');
        // Validation failed if values not match.
        return;
      }





// @todo Update.
      if (0) {

// @Todo review and remove.

        // Check that Phenotype names match phenotype metadata section.
        $association_phenotypes = tpps_parse_file_column(
          $assoc_file,
          $columns['Associated Trait'][5],
          $snps[self::FIELD_NAME . '-no-header'] ?? FALSE
        );

        $phenotype = $form_state['values'][$organism_index]['phenotype'];
        $phenotype_meta = $phenotype['metadata'];
        $phenotype_number = $phenotype['phenotypes-meta']['number'];

        $phenotype_meta_names = [];
        $phenotype_name_col = $phenotype['metadata-groups']['Phenotype Id']['1'] ?? NULL;
        if (isset($phenotype_name_col)) {
          $phenotype_meta_names = tpps_parse_file_column(
            $phenotype_meta,
            $phenotype_name_col
          );
        }

        for ($i = 1; $i <= $phenotype_number; $i++) {
          $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
        }

        $missing_phenotypes = array_diff(
          $association_phenotypes,
          $phenotype_meta_names
        );
        if (!empty($missing_phenotypes)) {
          $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
            . 'We detected Associated Traits that were not specified in the '
            . 'Phenotype Metadata Section. Please either remove  these Traits '
            . 'from your Association file, or add them to your Phenotype '
            . 'Metadata section. The Trait names we foud were: @phenotype_names',
            [
              '@field_title' => t(self::getFieldTitle()),
              '@oidx' => $organism_index,
              '@phenotype_names' => implode(', ', array_unique($missing_phenotypes)),
            ]
          );
          form_set_error(self::getFormErrorName($organism_index), $error_message);
        }

        // Check that position values are correctly formatted.
        $positions = tpps_parse_file_column(
          $assoc_file,
          $columns['Position'][3],
          $assoc_no_header
        );



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
            . 'We detected SNP positions that do '
            . 'not match the required format: @position.'
            . '<br />The correct format is: "start:stop" or integer.',
            [
              '@field_title' => t(self::getFieldTitle()),
              '@oidx' => $organism_index,
              '@position' => $position,
            ]
          );
          form_set_error(self::getFormErrorName($organism_index), $error_message);
          break;
        }
      }
    }

    // Do NOT allow further validation.
    if (form_get_errors()) {
      return FALSE;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    self::preserveValidFile($organism_index, $form_state);
  }

}
