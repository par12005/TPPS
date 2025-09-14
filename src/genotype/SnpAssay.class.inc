<?php

/**
 * @file
 * Contains 'SnpAssay' class.
 */

/**
 * Manages 'SnpAssay Design' file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SnpAssay extends FileField {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Unique common settings.
  //
  const TITLE = 'SNP Assay';
  // Name of the form field.
  // WARNING: Extra 's'!
  const DOM_NAME = 'snps-assay';
  // Name of the parent fieldset on form (!) not shared form state.
  // static::SNPS_FIELDSET.
  const SNPS_FIELDSET = 'SNPs';
  // Name of the parent element in shared state array.
  // Note: Differs if field was relocated. See versions in function
  // tpps_submission_form_version_changes().
  //
  //
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
    $organism_key = 'organism-' . $organism_index;

    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => static::getParents($organism_index, $form_state),
      'field_name' => static::DOM_NAME,
      'title' => t(static::getFieldTitle()),
      'organism_name' => $organism_key,
      'type' => static::DATA_TYPE,

      // According to Meghan's mockup.
      // 'allow_file_reuse' => TRUE,
      'description' => t('Please provide a spreadsheet with columns '
        . 'for the Plant ID of genotypes used in this study'
        . '<br />The format of this file is very important! '
        . '<br />The first column of your file should contain plant '
        . 'identifiers which match the plant identifiers you provided '
        . 'in your plant accession file, and all of the remaining '
        . 'columns should contain SNP data.'),
      'use_fid' => TRUE,
      'states' => [
        'visible' => [
          [
            static::buildCustomStateName($organism_index, [
              static::DATA_TYPE,
              static::SNPS_FIELDSET,
              'genotyping-type',
            ]) => ['value' => TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY],
          ],
          'or',
          [
            static::buildCustomStateName($organism_index, [
              static::DATA_TYPE,
              static::SNPS_FIELDSET,
              'file-type',
            ]) => ['value' => TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE],
          ],
        ],
      ],
    ]);
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
    ));
    // Do NOT allow empty field validation.
    $fid = static::getFileId($organism_index, $form_state);
    if (form_get_errors() || empty($fid)) {
      return FALSE;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Find duplicates.
    $headers = tpps_file_headers($fid);
    while (($k = array_search(NULL, $headers))) {
      $message = t('Following header column is Null which needs to be fixed. '
        . '@data', ['@data' => $k]);
      drupal_set_message($message, 'error');
      unset($headers[$k]);
    }
    $num_columns = tpps_file_width($fid) - 1;
    $num_unique_columns = count(array_unique($headers)) - 1;
    if ($num_unique_columns != $num_columns) {
      $duplicates = array_diff_assoc($headers, array_unique($headers));
      if (!empty($duplicates)) {
        drupal_set_message(
          t('Following header values are duplicate in provided snp file. @data',
            ['@data' => implode(',', $duplicates)]
          ), 'error'
        );
      }
      $error_message = t('Organism #@oidx / "<strong>@field_title</strong>":'
        . 'Some columns in the file you provided are missing or have duplicate '
        . 'header values. Please either enter valid header values for those '
        . 'columns or remove those columns, then reupload your file.',
        [
          '@field_title' => t(static::getFieldTitle()),
          '@oidx' => $organism_index,
        ]
      );
      form_set_error(static::getFormErrorName($organism_index), $error_message);
    }

    static::validateTrees($organism_index, $form_state);
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
  }

  /**
   * Gets list of SNP names in file.
   *
   * Note: 
   * 1st row (header) must contain SNP names except 1st column which is SNP_ID.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return array
   *   Returns list of SNP names in the file.
   */
  public static function getSnpList($organism_index, array $form_state) {
    // Get header of the 'SNP Assay' file field.
    $list = static::getFileHeader($organism_index, $form_state);
    // Note: File field 'SnpAssay' has no data type selectors for columns so
    // 1st column must be a 'SNP_ID' column. Other columns will have SNP
    // names in the header (1st row).
    TppsArray::removeFirst($list)
    return $list;
  }

}
