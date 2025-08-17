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
class AssayDesignCitation {

  // Name of the fieldset on PAGE 4.
  const DATA_TYPE = 'genotype';
  // Machine name of the 'SNPs' fieldset (parent of the parent).
  const SNPS_FIELDSET = 'SNPs';
  // Machine name of the parent fieldset on form.
  const PARENT_FIELDSET = 'assay-design-citation';
  // Machine name of the field which allow to select one of existing study
  // which has 'Assay Design' file. See AssayDesign::getSubmissionList().
  const EXISTING_STUDY_FIELD_NAME = 'existing_study';
  // Key of the option which allows to specify Paper DOI or Dataset DOI when
  // existing study couldn't be used.
  const EXISTING_STUDY_OTHER_OPTION_KEY = 'other';

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
    $organism_key = 'organism-' . $organism_index;
    $value = TppsArray::getValue($form_state['values'],
    [
      $organism_key,
      static::DATA_TYPE,
      static::SNPS_FIELDSET,
      static::PARENT_FIELDSET,
      static::EXISTING_STUDY_FIELD_NAME,
    ]);
    return $value;
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
    $organism_name = 'organism-' . $organism_index;
    $fields = &$form[$organism_name][static::DATA_TYPE];

    $states = [
      'visible' => [
        [
          ':input[name="' . $organism_name . '[' . implode('][', [
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
        [
          $organism_name,
          static::DATA_TYPE,
          static::SNPS_FIELDSET,
          static::PARENT_FIELDSET,
          static::EXISTING_STUDY_FIELD_NAME,
        ]
      ),
    ];
    $subelement_states = [
      'visible' => [
        [
          ':input[name="' . $organism_name . '[' . implode('][',
          [
            static::DATA_TYPE,
            static::SNPS_FIELDSET,
            static::PARENT_FIELDSET,
            static::EXISTING_STUDY_FIELD_NAME,
          ]
          ) . ']"]' => ['value' => static::EXISTING_STUDY_OTHER_OPTION_KEY],
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
    $existing_studies_value = TppsArray::getValue($form_state['values'], [
      'organism-' . $organism_index,
      static::DATA_TYPE,
      static::SNPS_FIELDSET,
      static::PARENT_FIELDSET,
      static::EXISTING_STUDY_FIELD_NAME,
    ]);
    if ($existing_studies_value == static::EXISTING_STUDY_OTHER_OPTION_KEY) {
      $parents = [
        'organism-' . $organism_index,
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
