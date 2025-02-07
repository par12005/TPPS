<?php

/**
 * @file
 * Contains 'SNPs Kinship' class.
 */

/**
 * Manages 'SNPs Kinship' file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SnpsKinship extends FileField {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Common settings.
  //
  const TITLE = 'SNPs Kinship';
  // Name of the form field.
  const FIELD_NAME = 'snps-kinship';
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

    if (form_get_errors()) {
      // Do not validate if any errors already found.
      return;
    }
    self::validateRequired($organism_index, $form_state, (
      // Requirement conditions:
      (
        $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
        || $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY
      )
      && ($snps['snps-assay'] ?? 0)
      && ($snps['snps-association'] ?? 0)
      && $snps['upload_snp_kinship'] == 'Yes'
    ));
    self::preserveValidFile($organism_index, $form_state);
  }

}
