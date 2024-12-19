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
   * Validates SNP Association field.
   *
   * @param int $org_num
   *   Ordinal organism number on page.
   * @param array $form
   *   Drupal Form API array.
   * @param array $form_state
   *   Drupal Form API State array.
   */
  public static function validate($org_num, array &$form, array &$form_state) {
    dpm('aval');
    $id = "organism-$org_num";
    $snps_fieldset = 'SNPs';
    $snps = $genotype[$snps_fieldset] ?? NULL;
    $snps_assay = $snps['snps-assay'] ?? 0;
    $assoc_file = $snps['snps-association'] ?? 0;
    $file_type = $snps['file-type'] ?? NULL;

    if (
      form_get_errors()
      || $file_type != TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
      // No file uploaded.
      || empty($assoc_file)
    ) {
      return;
    }

    $required_groups = [
      'SNP ID' => ['id' => [self::DATA_TYPE_SNP_ID]],
      'Scaffold' => ['scaffold' => [self::DATA_TYPE_SCAFFOLD]],
      'Position' => ['position' => [self::DATA_TYPE_POSITION]],
      'Allele' => ['allele' => [self::DATA_TYPE_ALLELE]],
      'Associated Trait' => ['trait' => [self::DATA_TYPE_ASSOCIATED_TRAIT]],
      'Confidence Value' => ['confidence' => [self::DATA_TYPE_CONFIDENCE_VALUE]],
      // Optional:
      // 7 => 'Gene ID',
      // 8 => 'Annotation',
      // Required when Phenotype data file has 'Year' column:
      // 9 => 'Year'.
    ];

    // When Phenotype Data file has 'Year' column then 'Year' column also
    // required.
    if (self::isDataTypeSet($form_state, $org_num, self::DATA_TYPE_YEAR)) {
      $required_groups['Year'] = ['year' => [self::DATA_TYPE_YEAR]];
    }

    $file_element = $form[$id]['genotype'][$snps_fieldset]['snps-association'];
    $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

    if (!form_get_errors()) {
      // Check that SNP IDs match Genotype Assay.
      $snps_id_col = $groups['SNP ID'][self::DATA_TYPE_SNP_ID];
      $assoc_no_header = $snps['snps-association-no-header'] ?? FALSE;

      $assay_snps = tpps_file_headers($snps_assay);
      unset($assay_snps[key($assay_snps)]);
      $assoc_snps = tpps_parse_file_column($assoc_file, $snps_id_col, $assoc_no_header);
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
        $assoc_file, $trait_id_col, $assoc_no_header
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
      $positions = tpps_parse_file_column($assoc_file, $position_col, $assoc_no_header);
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
    }

    // Preserve file if it is valid.
    tpps_preserve_valid_file($form_state, $assoc_file, $org_num, "SNPs_Association");
  }

}
