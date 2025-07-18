<?php

/**
 * @file
 * Contains 'AssayDesign' class.
 */

/**
 * Manages 'Assay Design' file field.
 *
 * TPPSc Form / Page 4 / Genotype / SNPs.
 *
 * Submission Form Versions:
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class AssayDesign {

  // Machine name of the 'SNPs' fieldset (parent of the parent).
  const SNPS_FIELDSET = 'SNPs';
  // Name of the fieldset on PAGE 4.
  const DATA_TYPE = 'genotype';
  const PARENT_FIELDSET = AssayDesignCitation::PARENT_FIELDSET;
  // HTML name of the field.
  const FIELD_NAME = 'assay-design';
  const FIELD_TITLE = 'Assay Design File';

  /**
   * Builds set of form fields for 'Assay Design Citation' feature.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($organism_index, array &$form, array $form_state) {
    $organism_key = 'organism-' . $organism_index;





// @TODO Field was relocated. Credate new version.





    // Add file upload field.
    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    // @TODO Get parents from the AssayDesignCitation class.
    $states = [
      ':input[name="' . $organism_key . '[' . implode('][', [
        AssayDesign::DATA_TYPE,
        AssayDesign::SNPS_FIELDSET,
        AssayDesignCitation::PARENT_FIELDSET,
        AssayDesignCitation::EXISTING_STUDY_FIELD_NAME,
      ]) . ']"]' => [
        'value' => AssayDesignCitation::EXISTING_STUDY_OTHER_OPTION_KEY,
      ],
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // We should also check this field because when 'genotyping-type' set into
      // 'Genotyping Array' field 'file-type' disapears with value which allows
      // this field to be shown and required.
      ':input[name="' . $organism_key . '[' . implode('][', [
        AssayDesign::DATA_TYPE,
        AssayDesign::SNPS_FIELDSET,
        'genotyping-type',
      ]) . ']"]' => ['value' => TPPS_GENOTYPING_TYPE_GENOTYPING],
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      ':input[name="' . $organism_key . '[' . implode('][', [
        AssayDesign::DATA_TYPE,
        AssayDesign::SNPS_FIELDSET,
        'file-type',
      ]) . ']"]' => [
        'value' => TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE,
      ],
    ];
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => [
        $organism_key,
        static::DATA_TYPE,
        static::SNPS_FIELDSET,
        static::PARENT_FIELDSET,
      ],
      'field_name' => static::FIELD_NAME,
      'title' => t(static::FIELD_TITLE),
      'organism_name' => $organism_key,
      'type' => static::DATA_TYPE,
      'optional' => TRUE,
      // Required when visible.
      'states' => [
        'visible' => $states,
        'required' => $states,
      ],
    ]);
  }

  /**
   * Validates set of form fields for 'Assay Design Citation' feature.
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

    // Get 'SNPs' fieldset.
    $snps = tpps_array_get_value(
      $form_state['values'],
      [
        $organism_key,
        static::DATA_TYPE,
        static::SNPS_FIELDSET,
      ]
    );

    // Get values of selectors.
    $file_type = $snps['file-type'] ?? NULL;
    $genotyping_type = $snps['genotyping-type'] ?? NULL;
    $existing_study = tpps_array_get_value($form_state['values'],
    [
      $organism_key,
      AssayDesignCitation::DATA_TYPE,
      AssayDesignCitation::SNPS_FIELDSET,
      AssayDesignCitation::PARENT_FIELDSET,
      AssayDesignCitation::EXISTING_STUDY_FIELD_NAME,
    ]);

//dpm($file_type, 'file_type');
//dpm($genotyping_type, 'genotyping_type');
//dpm($existing_study, 'exitsitn');

    if (
      $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
      && $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING
      && $existing_study == AssayDesignCitation::EXISTING_STUDY_OTHER_OPTION_KEY
    ) {
      //dpm('required');
      $fid = tpps_array_get_value(
        $form_state['values'],
        [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          AssayDesignCitation::EXISTING_STUDY_FIELD_NAME,
          static::FIELD_NAME,
        ]
      );
      if ($fid) {
        // Preserve file if it is valid.
        tpps_preserve_valid_file(
          $form_state,
          $fid,
          $organism_index,
          'Genotype_Assay_Design'
        );
      }
    }
  }

  /**
   * Processes.
   *
   * Note: called from /forms/submit/submit_all.php script.
   *
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $shared_state
   *   Submission's Shared State.
   * @param array $options
   *   Necessary data.
   *   WARNING: 'genotype_id' is required to store 'year' column on file.
   */
  public static function process($organism_index, array $shared_state, array &$options = []) {
  }

  /**
   * Gets parents of the field.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   * @param int $version
   *   Submission Form Version.
   *   Default is lates version (higher number).
   *
   * @return array
   *   Returns parents for the checkbox which makes this field required.
   */
  public static function getParents($organism_index, $version = NULL) {
    $organism_key = 'organism-' . $organism_index;

    switch ($version) {
      case 1:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          'files',
          static::FIELD_NAME,
        ];
        break;

      case 2:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::FIELD_NAME,
        ];
        break;

      case 3:
      default:
        $parents = [
          $organism_key,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::PARENT_FIELDSET,
          static::FIELD_NAME,
        ];
        break;
    }
    return $parents;
  }

}
