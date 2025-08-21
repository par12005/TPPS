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

}
