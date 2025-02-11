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
          '#default_value' => tpps_array_get_value($form_state,
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
    $genotype = &$form_state['values']['organism-' . $organism_index][self::DATA_TYPE] ?? NULL;
    $snps = &$genotype[self::FORM_PARENT_FIELDSET] ?? NULL;
    // Fields.
    $file_type = $snps['file-type'] ?? NULL;
    $genotyping_type = $snps['genotyping-type'] ?? NULL;
    $fid = self::getFileId($organism_index, $form_state);

    // @TODO Check if field really required.
    if (variable_get('tpps_assay_design_is_required', TRUE)) {
      self::validateRequired($organism_index, $form_state, (
        // Requirement conditions:
        $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
        || $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY
      ));
      // Do NOT allow further validation.
      if (form_get_errors()) {
        return FALSE;
      }
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

      $assoc_file = $snps['snps-association'] ?? 0;
      $snps_assay = $snps['snps-assay'] ?? 0;


// @todo Update.
      $columns = self::validateColumns($organism_index, $form_state, $silent_mode = TRUE);
      if (form_get_errors()) {
        return;
      }

        // Check that SNP IDs match Genotype Assay.
        $snps_id_col = $columns['SNP ID'][1];
        $assoc_no_header = $snps[$this->name . '-no-header'] ?? FALSE;

        $assay_snps = tpps_file_headers($snps_assay);
        unset($assay_snps[key($assay_snps)]);
        $assoc_snps = tpps_parse_file_column($assoc_file, $snps_id_col, $assoc_no_header);
        $missing_snps = array_diff($assoc_snps, $assay_snps);

        if (!empty($missing_snps)) {
          $error_message = t('Organism #@oidx / "<strong>@field_title</strong>":'
            . 'We detected SNP IDs that were not in your Genotype Assay. '
            . 'Please either remove these SNPs from your Association file, or '
            . 'add them to your Genotype Assay. The SNP Identifiers we '
            . 'found were: @snps_id_list.',
            [
              '@field_title' => t(self::getFieldTitle()),
              '@oidx' => $organism_index,
              '@snps_id_list' => implode(', ', array_unique($missing_snps)),
            ]
          );
          form_set_error(self::getFormErrorName($organism_index), $error_message);
        }

        // Check that Phenotype names match phenotype metadata section.
        $association_phenotypes = tpps_parse_file_column(
          $assoc_file,
          $columns['Associated Trait'][5],
          $assoc_no_header
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
          $error_message = t('Organism #@oidx / "<strong>@field_title</strong>":'
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
          if (!preg_match('/^(\d+):(\d+)$/', $position)) {
            $error_message = t('Organism #@oidx / "<strong>@field_title</strong>":'
              . 'We detected SNP positions that do not match the required format. '
              . 'The correct format is: "start:stop".',
              [
                '@field_title' => t(self::getFieldTitle()),
                '@oidx' => $organism_index,
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


  /**
   * Gets required columns.
   *
   * @return array
   *   Returns list of groups of the required columns.
   */
  public static function getRequiredColumns() {
    return [
      // Name => key => <id in the option's list>.
      'case 1' => [
        'SNP ID' => ['id' => [1]],
        'Scaffold' => ['scaffold' => [2]],
        'Position' => ['position' => [3]],
      ],
      'case 2' => [
        'SNP ID' => ['id' => [1]],
        'Flank sequence' => ['sequence' => [4]],
      ],
    ];
  }

}
