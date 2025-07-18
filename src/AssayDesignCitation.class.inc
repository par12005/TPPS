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
  // which has 'Assay Design' file. See SubmissionCache::getAssayDesignList().
  const EXISTING_STUDY_FIELD_NAME = 'existing_study';
  // Key of the option which allows to specify Paper DOI or Dataset DOI when
  // existing study couldn't be used.
  const EXISTING_STUDY_OTHER_OPTION_KEY = 'other';

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
        SubmissionCache::getAssayDesignList(),
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
   * @param int $organism_index
   *   Ordinal organism number on page.
   * @param array $form
   *   Drupal Form API array.
   * @param array $form_state
   *   Drupal Form API State array.
   */
  public static function validate($organism_index, array &$form, array &$form_state) {
    // All fields are optional.
    $parents = [
      'organism-' . $organism_index,
      static::DATA_TYPE,
      static::SNPS_FIELDSET,
    ];
    $genotyping_type = tpps_array_get_value(
      $form_state['values'], array_merge($parents, ['genotyping-type'])
    );
    if ($genotyping_type != TPPS_GENOTYPING_TYPE_GENOTYPING) {
      return;
    }

    $existing_study = tpps_array_get_value(
      $form_state['values'],
      array_merge($parents, [static::PARENT_FIELDSET, 'existing_study'])
    );
    if ($existing_study == 'other') {
      $paper_doi = tpps_array_get_value(
        $form_state['values'],
        array_merge($parents, [static::PARENT_FIELDSET, 'paper-doi'])
      );
      $dataset_doi = tpps_array_get_value(
        $form_state['values'],
        array_merge($parents, [static::PARENT_FIELDSET, 'dataset-doi'])
      );
      if (empty($paper_doi) && empty($dataset_doi)) {
        // At least one field is required.
        //$message = t('Please specify "Assay Design Citation / '
        //  . 'Paper DOI" or "Assay Design Citation / Dataset DOI".');
        //$field_name = implode('][',
        //  array_merge($parents, [static::PARENT_FIELDSET, 'paper-doi'])
        //);
        //form_set_error($field_name, $message);
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
