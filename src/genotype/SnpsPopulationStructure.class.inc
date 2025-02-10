<?php

/**
 * @file
 * Contains 'SnpsPopulationStructure' class.
 */

/**
 * Manages 'SnpsPopulationStructure' file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SnpsPopulationStructure extends FileField {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Common settings.
  //
  const TITLE = 'SNPs Population Structure';
  // Name of the form field.
  const FIELD_NAME = 'snps-pop-struct';
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
    $fid = self::getFileId($organism_index, $form_state);

    self::validateRequired($organism_index, $form_state, (
      // Requirement conditions:
      (
        $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
        || $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY
      )
      && ($snps['snps-assay'] ?? 0)
      && ($snps['snps-association'] ?? 0)
      && $snps['upload_snp_population'] == TPPS_YES
    ));
    self::preserveValidFile($organism_index, $form_state);
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
    // @todo Minor. Rename $fields to $subform.
    $fields = &$form[$organism_name][self::DATA_TYPE];

    // Would you like to upload a SNPs Population Structure file?
    // New field in v2.
    $fields[self::FORM_PARENT_FIELDSET]['upload_snp_population'] = [
      '#type' => 'select',
      '#title' => t('Would you like to upload a SNPs Population Structure file?'),
      '#options' => [
        0 => t('- Select -'),
        TPPS_YES => t('Yes'),
        TPPS_NO => t('No'),
      ],
      '#default_value' => tpps_get_ajax_value($form_state,
        [
          $organism_name,
          self::DATA_TYPE,
          self::FORM_PARENT_FIELDSET,
          'upload_snp_population'
        ],
        'Yes'
      ),
    ];
    // SNPs Population Structure File.
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
          self::getStateName($organism_index,
            [self::FORM_PARENT_FIELDSET, 'upload_snp_population']
          ) => ['value' => TPPS_YES],
        ],
      ],
    ]);
  }

}
