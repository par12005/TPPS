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
 * Field relocation history:
 * v1: ['genotype', 'files', 'assay-design'].
 * v2: -> ['genotype', 'SNPs', 'assay-design'].
 * v3: -> ['genotype', 'SNPs', 'assay-design-citation', 'assay-design'].
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
  // Integer Form Page number.
  const PAGE = TPPS_PAGE_4;

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
    // @TODO Get parents from classes using getParents() method to respect
    // Submission's form version.
    $states = [
      // Existing studies with an "Assay design file":
      // 'The citation for my assay design is not in this list'.
      [
        ':input[name="' . $organism_key . '[' . implode('][', [
          AssayDesign::DATA_TYPE,
          AssayDesign::SNPS_FIELDSET,
          AssayDesignCitation::PARENT_FIELDSET,
          AssayDesignCitation::EXISTING_STUDY_FIELD_NAME,
        ]) . ']"]' => [
          'value' => AssayDesignCitation::EXISTING_STUDY_OTHER_OPTION_KEY,
        ],
      ],
      'or',
      [
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // 'Genotyping Type': 'Genotyping'
        // We should also check this field because when 'genotyping-type' set into
        // 'Genotyping Array' field 'file-type' disapears with value which allows
        // this field to be shown and required.
        ':input[name="' . $organism_key . '[' . implode('][', [
          AssayDesign::DATA_TYPE,
          AssayDesign::SNPS_FIELDSET,
          'genotyping-type',
        ]) . ']"]' => ['value' => TPPS_GENOTYPING_TYPE_GENOTYPING],
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // 'Genotyping file type': 'SNP Assay file and Assay design file'
        ':input[name="' . $organism_key . '[' . implode('][', [
          AssayDesign::DATA_TYPE,
          AssayDesign::SNPS_FIELDSET,
          'file-type',
        ]) . ']"]' => [
          'value' => TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE,
        ],
      ],
    ];
    // Add file upload field.
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
    $existing_study = AssayDesignCitation::getExistingStudy($organism_index, $form_state);
    if (
      $existing_study == AssayDesignCitation::EXISTING_STUDY_OTHER_OPTION_KEY
      || (
        $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING
        && $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
      )
    ) {
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
   * Gets field's value.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns integer File Id.
   *   Returns NULL if field wasn't found.
   *   Returns string '0' if field wasn't set (file wasn't uploaded).
   */
  public static function getValue($organism_index, array $form_state) {
    $parents = static::getParents($organism_index, $form_state);
    return tpps_array_get_value($form_state['values'], $parents);
  }

  /**
   * Alias for getValue() method.
   *
   * @todo Move to TppsFile class.
   */
  public static function getFileId($organism_index, array $form_state) {
    return static::getValue($organism_index, $form_state);
  }

  /**
   * Gets parents of the field.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return array
   *   Returns parents for the checkbox which makes this field required.
   */
  public static function getParents($organism_index, array $form_state) {
    $organism_key = 'organism-' . $organism_index;
    switch (SubmissionVersion::get($form_state)) {
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

  /**
   * Removes field's file by File Id.
   *
   * WARNING: for validation only.
   *
   * @param int $organism_index
   *   Ordinal number of organism at page.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function removeFile($organism_index, array &$form_state) {
    $fid = static::get($organism_index, $form_state);
    if (tpps_file_remove($fid)) {
      $parents = static::getParents($organism_index, $form_state);
      tpps_array_set_value($form_state['values'], $parents, $value = 0);
    }
  }

  /**
   * Clears cached list of studies which has AssayDesign files.
   */
  public static function clearSubmissionList() {
    $key = 'AssayDesign::getSubmissionList';
    // Clear cache in db.
    $cid = $key;
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    $cache = cache_get($cid, $cache_bin);
    if (!empty($cache)) {
      $cache->data[$key] = NULL;
      cache_set($cid, $cache->data, $cache_bin);
    }
    // Clear static cache.
    $static_cache = &drupal_static($key);
    $static_cache[$key] = NULL;
  }

  /**
   * Gets list of accessions which has (or doesn't have) 'Assay Design' file.
   *
   * @param bool $has
   *   Filter which allows to get list of submissions which doesn't have
   *   'Assay Design' file when set to FALSE. Default is TRUE.
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *
   * @return array
   *   Returns list of accession as a key and shorten titles as a value.
   */
  public static function getSubmissionList($has = TRUE, $reset = FALSE) {
    $key = __METHOD__;
    $static_cache = &drupal_static($key);
    if ($reset || !isset($static_cache[$key])) {
      // Get value.
      $cid = $key;
      $cache_bin = TPPS_CACHE_BIN ?? 'cache';
      $cache = cache_get($cid, $cache_bin);
      if ($reset || empty($cache) || !isset($cache->data[$key])) {
        $cache->data[$key] = NULL;
        // Get new data.
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        $data = SubmissionCache::get($reset);
        $list = [];
        foreach ($data as $accession => $cached_submission) {
          $title = $cached_submission['publication']['title'] ?? t('No Title');
          $title = (strlen($title) > 97) ? substr($title, 0, 97) . '...' : $title;
          $item = $accession . ' - ' . $title;
          if (empty($cached_submission[SubmissionCache::FILES_CONTAINER][static::FIELD_NAME])) {
            $list['has_not'][$accession] = $item;
          }
          else {
            $list['has'][$accession] = $item;
          }
        }
        $cache->data[$key] = $list;
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        cache_set($cid, $cache->data, $cache_bin);
      }
      $static_cache[$key] = $cache->data[$key] ?? NULL;
    }
    if ($has) {
      return $static_cache[$key]['has'] ?? [];
    }
    else {
      return $static_cache[$key]['has_not'] ?? [];
    }
  }

}
