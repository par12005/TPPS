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
  const DOM_NAME = 'snps-pop-struct';
  // Name of the parent fieldset on form (!) not shared form state.
  // static::SNPS_FIELDSET.
  const SNPS_FIELDSET = 'SNPs';
  // Type of the data.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  const DATA_TYPE = 'genotype';
  // Form step where field is used.
  const FORM_STEP = 4;

  /**
   * Gets lit of required columns for validation.
   *
   * Note:
   * File class must overwrite this method of column validation is required.
   *
   * @return array
   *   Returns list of groups of required columns.
   *   See AssayDesign::getRequiredColumns() for the structure example.
   */
  public static function getRequiredColumns() {
    return [];
  }

  /**
   * Gets parents of the field.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   * @param array $form_state
   *   Drupal Form State array.
   * @param string $dom_name
   *   When empty then list of the parents will have static::DOM_NAME
   *   as a last element and given DOM-name otherwise.
   *
   * @return array
   *   Returns parents for the checkbox which makes this field required.
   */
  public static function getParents($organism_index, array $form_state, $dom_name = NULL) {
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
      case 3:
      default:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::DOM_NAME,
        ];
        break;
    }
    return $parents;
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
    $genotype = &$form_state['values'][$organism_key][static::DATA_TYPE] ?? NULL;
    $snps = &$genotype[static::SNPS_FIELDSET] ?? NULL;
    // Fields.
    $file_type = $snps['file-type'] ?? NULL;
    $genotyping_type = $snps['genotyping-type'] ?? NULL;
    $fid = static::getFileId($organism_index, $form_state);

    static::validateRequired($organism_index, $form_state, (
      // Requirement conditions:
      (
        $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
        || $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY
      )
      && ($snps['snps-assay'] ?? 0)
      && ($snps['snps-association'] ?? 0)
      && $snps['upload_snp_population'] == TPPS_YES
    ));
    // Do NOT allow empty field validation.
    if (form_get_errors()) {
      return FALSE;
    }
    static::preserveValidFile($organism_index, $form_state);
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
    $organism_key = 'organism-' . $organism_index;
    $sub_form = &$form[$organism_key][static::DATA_TYPE];

    // Would you like to upload a SNPs Population Structure file?
    // New field in v2.
    $sub_form[static::SNPS_FIELDSET]['upload_snp_population'] = [
      '#type' => 'select',
      '#title' => t('Would you like to upload a SNPs Population Structure file?'),
      '#options' => [
        0 => t('- Select -'),
        TPPS_YES => t('Yes'),
        TPPS_NO => t('No'),
      ],
      '#default_value' => tpps_get_ajax_value($form_state,
        [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
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
      'parents' => static::getParents($organism_index, $form_state),
      'field_name' => static::DOM_NAME,
      'title' => t(static::getFieldTitle()),
      'organism_name' => $organism_key,
      'type' => static::DATA_TYPE,
      'optional' => TRUE,
      'states' => [
        'visible' => [
          static::buildCustomStateName($organism_index, [
            static::DATA_TYPE,
            static::SNPS_FIELDSET,
            'upload_snp_population',
          ]) => ['value' => TPPS_YES],
        ],
      ],
    ]);
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
    if (empty($fid = static::getFileId($organism_index, $shared_state))) {
      return;
    }
    tpps_add_project_file($shared_state, $fid);
  }

}
