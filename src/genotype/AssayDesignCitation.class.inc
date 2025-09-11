<?php

/**
 * @file
 * Contains 'AssayDesignCitation' class.
 */

/**
 * Manages fields in 'Assay Design Citation' fieldset.
 *
 * TPPSc Form / Page 4 / Genotype / SNPs.
 *
 * Submission Form Versions:
 * This fieldset is missing in Form Version 1 and 2.
 * But v1 and v2 had 'assay-design-citation' textfield which wasn't actually
 * used anywhere and was replaced with this fieldset.
 *
 * Note:
 * 'Assay Design' field belongs to the fieldset but won't be processed here.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class AssayDesignCitation extends FileField {

  // Name of the fieldset on PAGE 4.
  const DATA_TYPE = 'genotype';
  // Machine name of the 'SNPs' fieldset (parent of the parent).
  const SNPS_FIELDSET = 'SNPs';
  // Machine name of the parent fieldset on form.
  const PARENT_FIELDSET = 'assay-design-citation';
  // Integer Form Page number.
  const FORM_STEP = TPPS_PAGE_4;

  // Machine name of the field which allow to select one of existing study
  // which has 'Assay Design' file. See AssayDesign::getSubmissionList().
  const EXISTING_STUDY_FIELD_NAME = 'existing_study';
  // Key of the option which allows to specify Paper DOI or Dataset DOI when
  // existing study couldn't be used.
  const EXISTING_STUDY_OTHER_OPTION_KEY = 'other';

  /**
   * Gets required columns.
   *
   * @return array
   *   Returns list of groups of the required columns.
   */
  public static function getRequiredColumns() {
    return [];
  }

  /**
   * Gets value of the Existing Study field.
   *
   * @param int $organism_index
   *   Ordinal number on form.
   * @param array $form_state
   *   Drupal Form State array.
   *
   * @return string
   *   Returns value of the field EXISTING_STUDY_FIELD_NAME.
   */
  public static function getExistingStudy($organism_index, array $form_state) {
    return TppsArray::getValue(
      $form_state['values'],
      static::getParents($organism_index, $form_state,
        static::EXISTING_STUDY_FIELD_NAME
      )
    );
  }

  /**
   * Gets parents of the field.
   *
   * Note: Currently implemented only 'Existing Study' field.
   *
   * Usage:
   * To get parents for $form skip $organism_index.
   *   $parents = AssayDesign::getParents();
   * To get latest version or skip version detection - skip $form_state array.
   *   $parents = AssayDesign::getParents($organism_index);
   * To detect current version of the $form_state array:
   *   $parents = AssayDesign::getParents($organism_index, $form_state);
   *
   * Usage example:
   * $parent_list = AssayDesignCitation::getParents(
   *   $organism_index,
   *   $form_state,
   *   AssayDesignCitation::EXISTING_STUDY_FIELD_NAME
   * );
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

    if ($dom_name == static::EXISTING_STUDY_FIELD_NAME) {
      switch ($version) {
        case 1:
        case 2:
          // Field didn't exist.
          $parents = [];
          break;

        case 3:
          $parents = [
            $organism_key,
            static::DATA_TYPE,
            static::SNPS_FIELDSET,
            static::PARENT_FIELDSET,
            static::EXISTING_STUDY_FIELD_NAME,
          ];
      }
    }
    else {
      $message = 'Requested list of the parents for not yet defined field "@dom_name".';
      $params = ['@dom_name' => $dom_name];
      throw new Exception($message);
    }

    if (!is_null($organism_index)) {
      $organism_key = 'organism-' . $organism_index;
      $parents = array_merge([$organism_key], $parents);
    }
    return $parents;
  }

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
    $fields = &$form[$organism_key][static::DATA_TYPE];

    $states = [
      'visible' => [
        [
          ':input[name="' . $organism_key . '[' . implode('][', [
            static::DATA_TYPE, 'does_study_include_snp_data'
          ]) . ']"]' => ['value' => 'yes'],
        ],
      ],
    ];

    $fields[static::SNPS_FIELDSET][static::PARENT_FIELDSET] = [
      '#type' => 'fieldset',
      '#title' => t('Assay Design Citation'),
      '#states' => $states,
    ];
    $form_element = &$fields[static::SNPS_FIELDSET][static::PARENT_FIELDSET];

    $other_option_title = t('The citation for my assay design is not in this list');
    $form_element[static::EXISTING_STUDY_FIELD_NAME] = [
      '#type' => 'select',
      '#title' => t('Existing studies with an “Assay design file”'),
      '#options' => array_merge(
        AssayDesign::getSubmissionList(),
        [static::EXISTING_STUDY_OTHER_OPTION_KEY => $other_option_title]
      ),
      '#default_value' => tpps_get_ajax_value(
        $form_state,
        static::getParents($organism_index, $form_state,
          static::EXISTING_STUDY_FIELD_NAME
        )
      ),
    ];
    $subelement_states = [
      'visible' => [
        [
          ':input[name="' . static::buildDomName($organism_index, $form_state,
            static::EXISTING_STUDY_FIELD_NAME) . '"]'
          => ['value' => static::EXISTING_STUDY_OTHER_OPTION_KEY],
        ],
      ],
    ];
    $form_element['instructions'] = [
      '#type' => 'container',
      '#states' => $subelement_states,
      'message' => [
        '#markup' => '<p>' . t('Provide a DOI referencing the original marker design, '
        . 'if you did not design these markers. If your assay design is not '
        . 'published, you will be expected to upload it.') . '</p>',
      ],
    ];
    $form_element['paper-doi'] = [
      '#type' => 'textfield',
      '#title' => t('Paper DOI:'),
      '#description' => tpps_form_get_doi_examples(),
      '#states' => $subelement_states,
      // Optional.
    ];
    $form_element['dataset-doi'] = [
      '#type' => 'textfield',
      '#title' => t('Dataset DOI:'),
      '#states' => $subelement_states,
      '#description' => tpps_form_get_doi_examples(),
      // Optional.
    ];
    tpps_add_css_js('suggestion', $form);
  }

  /**
   * Validates set of form fields for 'Assay Design Citation' feature.
   *
   * Note: 'Assay Design File' field belongs to the fieldset but not processed
   * here. See AssayDesign::validate().
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
    $existing_studies_value = TppsArray::getValue(
      $form_state['values'],
      static::getParents($organism_index, $form_state,
        static::EXISTING_STUDY_FIELD_NAME
      )
    );
    if ($existing_studies_value == static::EXISTING_STUDY_OTHER_OPTION_KEY) {
      $parents = [
        $organism_key,
        static::DATA_TYPE,
        static::SNPS_FIELDSET,
      ];
      // Paper DOI and Dataset DOI are visible.
      // Validate DOI format if fields not empty because they are optional.
      foreach (['paper-doi', 'dataset-doi'] as $field_name) {
        $value = trim(TppsArray::getValue(
          $form_state['values'],
          array_merge($parents, [static::PARENT_FIELDSET, $field_name])
        ));
        if (empty($value)) {
          continue;
        }
        if (!tpps_is_valid('doi', $value)) {
          $field_title = 'Assay Design Citation / ' . ucwords(
            str_replace('-', ' ', str_replace('doi', 'DOI', $field_name))
          );
          $message = $field_title . ': '
            . t('Field is optional but DOI format is invalid.'
            . '<br />Example of valid DOI: "10.1111/dryad.111".'
            );
          $field_name = implode('][',
            array_merge($parents, [static::PARENT_FIELDSET, $field_name])
          );
          form_set_error($field_name, $message);
        }
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

}
