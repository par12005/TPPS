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
  const DOM_NAME = 'snps-kinship';
  // Name of the parent fieldset on form (!) not shared form state.
  // static::SNPS_FIELDSET.
  const SNPS_FIELDSET = 'SNPs';
  // Type of the data.
  // Could be 'phenotype', 'genotype' or 'environmental'.
  const DATA_TYPE = 'genotype';
  // Form step where field is used.
  const FORM_STEP = 4;

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

    switch (SubmissionVersion::get($form_state)) {
      case 1:
        $parents = [
          static::DATA_TYPE,
          'files',
          static::DOM_NAME,
        ];
        break;

      case 2:
      case 3:
      default:
        $parents = [
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::DOM_NAME,
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
      && $snps['upload_snp_kinship'] == 'Yes'
    ));
    // Do NOT allow empty field validation.
    $fid = static::getFileId($organism_index, $form_state);
    if (form_get_errors() || empty($fid)) {
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
    // Would you like to upload a SNPs Kinship File?
    // Relocated in v2. ['genotype', 'files'] -> ['genotype', 'SNPs'].
    $sub_form[static::SNPS_FIELDSET]['upload_snp_kinship'] = [
      '#type' => 'select',
      '#title' => t('Would you like to upload a SNPs Kinship File?'),
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
          'upload_snp_kinship'
        ],
        'Yes'
      ),
    ];
    // Upload File Field.
    // Field was relocated (v.2). ['files'] -> ['SNPs'].
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
            'upload_snp_kinship',
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
