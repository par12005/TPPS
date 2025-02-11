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


  //
  /**
   * This function validates that the file contains all of the required groups.
   *
   * Based on code from function tpps_file_validate_columns().
   *
   * A "required group" is a column type that must be present within the file for
   * the file to be considered valid. For instance, a required group for a plant
   * accession file would be the "Location" group. The required groups array
   * should satisfy the structure:
   *  array(
   *    '<group name>' => array(
   *      '<type>' => array(<column option index>[, <column option index>...])[,
   *      '<type>' => array(<column option index>[, <column option index>...])...]
   *    )[,
   *    '<group name>' => array(
   *      '<type>' => array(<column option index>[, <column option index>...])[,
   *      '<type>' => array(<column option index>[, <column option index>...])...]
   *    )...]
   *  )
   * where <group name> is the name of the required group, <type> is the name of
   * the set of options that could satisfy the required group, and each of the
   * <column option index>s is a key within the file element['column-options']
   * field.
   *
   * @param int $organism_index
   *   Ordinal organism number at page.
   * @param array $form_state
   *   Drupal Form API State Array.
   * @param bool $silent_mode
   *   Default is FALSE.
   *   When FALSE and silent mode is disabled then form_set_error() will be called
   *   which means error message on page will appear and form validation fails.
   *   When TRUE and silent mode is enabled then no error message will be shown
   *   and form will be left valid. This could be useful when not all columns are
   *   required and multiple groups or columns must be checked.
   *   For example:
   *   case 1: Required columns: A, B, C;
   *   case 2: Required columns: A, D, E, F.
   *
   * @return array
   *   The groups and the column option types that the file was valid for.
   *   Example:
   *   [
   *     // <Name> => [
   *     //   <id in the option's list> => <column index>,
   *     //   '#type' => <key>
   *     // ],
   *     'SNP ID' => [1 => 'B', '#type' => 'id'],
   *     'Scaffold' => [
   *       '#type' => [], // < ---- Not found!
   *     ],
   *     'Position' => [3 => 'D', '#type' => 'position'],
   *     'Allele' => [4 => 'E', '#type' => 'allele'],
   *     'Associated Trait' => [5 => 'F', '#type' => 'trait'],
   *     'Confidence Value' => [6 => 'G', '#type' => 'confidence'],
   *   ];
   *
   * @TODO Move to class 'FileField'.
   * @todo Minor. Replace tpps_file_validate_columns() with this code to allow
   * multiple groups of required columns.
   */
  public static function validateColumns($organism_index, array &$form_state, $silent_mode = FALSE) {
    $required_groups = self::getRequiredColumns();
    $file_element = self::getFormElement($organism_index, $form_state);
    // Check if file has zero length (empty).
    if (!($file_element['#file']->filesize ?? NULL)) {
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
        . ': File is empty.'
        [
          '@field_title' => self::getFieldTitle(),
          '@oidx' => $organism_index,
        ]
      );
      // We highlight an empty value field because file field not visible
      // when file was uploaded.
      // $file_element['#name'] . '[empty]',
      // There is no 'empty' element for file fields.
      form_set_error($file_element['#name'], $error_message);
      // No need to validate empty file.
      return [];
    }

    $cols = $file_element['#value']['columns'] ?? NULL;
    $parents = $file_element['#parents'];
    $new_end_columns = end($parents) . "-columns";
    $new_end_no_header = end($parents) . "-no-header";
    $new_end_empty = end($parents) . "-empty";
    $group_path = array_pop($parents) . "-groups";
    $values = drupal_array_get_nested_value($form_state['values'], $parents);
    // Initialize form column values in form state.
    $values[$new_end_columns] = [];
    // Hold onto the location of the columns in form state.
    $state_column_values = &$values[$new_end_columns];
    $values[$new_end_no_header] = $file_element['#value']['no-header'] ?? NULL;
    $values[$new_end_empty] = $file_element['#value']['empty'] ?? NULL;


    $columns = [];

    foreach ($required_groups as $case) {
      $required_groups_flat = [];
      foreach ($case as $group => $combinations) {
        $columns[$group] = [];
        $required_groups_flat[$group] = [];
        foreach ($combinations as $name => $combination) {
          $required_groups_flat[$group] = array_merge(
            $required_groups_flat[$group],
            $combination
          );
        }
      }

//dpm($required_groups_flat);

      if ($cols) {
        // Note: $cols will be empty when user just added file (not clicked 'Upload'
        // button and submitted form (clicked 'Next' button).
        // Probably columns could be detected automatically because they are shown
        // at page and so we could use default values here.
        // @TODO Get default set of columns.
        foreach ($cols as $name => $type) {
          $state_column_values[$name] = $type;
          foreach ($required_groups_flat as $group => $types) {
            if (in_array($type, $types)) {
              if (!isset($columns[$group][$type])) {
                $columns[$group][$type] = [$name];
              }
              else {
                $columns[$group][$type][] = $name;
              }
              break;
            }
          }
        }
      }

      foreach ($case as $group => $combinations) {
        $group_valid = FALSE;
        $columns[$group]['#type'] = [];
        foreach ($combinations as $name => $combination) {
          $combination_valid = TRUE;
          foreach ($combination as $type) {
            if (!isset($groups[$group][$type])) {
              $combination_valid = FALSE;
              break;
            }
          }
          if ($combination_valid) {
            $columns[$group]['#type'][] = $name;
            $group_valid = TRUE;
          }
        }

        if (!$group_valid && !$silent_mode) {
          $error_message = t('Organism #@oidx / "<strong>@field_title</strong>": '
            . 'Please specify a column or columns that hold @group.',
            [
              '@field_title' => self::getFieldTitle(),
              '@oidx' => $organism_index,
              '@group' => $group,
            ]
          );
          form_set_error($file_element['#name'] . "[columns][$group", $error_message);
        }
      }

      // Replaces list of column names with a single value when was found only one. Instead of this:
      // [
      //   'Flank sequence' => [
      //     '4' => ['0' => 'B'],
      //     '#type' => ['0' => 'sequence'],
      //   ]
      // ];
      // we will get this:
      // [
      //   'Flank sequence' => ['4' => 'B', '#type' => 'sequence'],
      // ];
      foreach ($columns as $key => $group) {
        foreach ($group as $opt_num => $col_names) {
          if (count($col_names) == 1) {
            $columns[$key][$opt_num] = $col_names[0];
          }
        }
      }

      $values[$group_path] = $columns;
      drupal_array_set_nested_value($form_state['values'], $parents, $values);

// @todo Check if original $form_state array was updated.

    }
//dpm(print_r($columns, 1));
    return $columns;
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
