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
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class AssayDesignCitation {

  // Machine name of the field which allow to select one of existing study
  // which has 'Assay Design' file. See SubmissionCache::getAssayDesignList().
  const EXISTING_STUDY_FIELD_NAME = 'existing_study';
  // Machine name of the 'SNPs' fieldset (parent of the parent).
  const SNPS_FIELDSET = 'SNPs';
  // Key of the option which allows to specify Paper DOI or Dataset DOI when
  // existing study couldn't be used.
  const EXISTING_STUDY_OTHER_OPTION_KEY = 'other';
  // Machine name of the parent fieldset on form.
  const PARENT_FIELDSET = 'assay-design-citation';
  // Name of the fieldset on PAGE 4.
  const DATA_TYPE = 'genotype';

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
          ':input[name="' . $organism_name . '[' . implode('][',
          [
            static::DATA_TYPE,
            static::SNPS_FIELDSET,
            'genotyping-type'
          ]) . ']"]' => ['value' => TPPS_GENOTYPING_TYPE_GENOTYPING],
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
    // Instructions above the inputs.
    $form_element['instructions'] = [
      '#markup' => '<p>' . t('Provide a DOI referencing the original marker design, '
        . 'if you did not design these markers. If your assay design is not '
        . 'published, you will be expected to upload it.') . '</p>',
      '#states' => $subelement_states,
    ];
    // Paper DOI – Preferred citation.
    $form_element['paper-doi'] = [
      '#type' => 'textfield',
      '#title' => t('Paper DOI:'),
      '#states' => $subelement_states,
      '#description' => tpps_form_get_doi_examples(),
      // @TODO Must be required when visible.
    ];
    // Dataset DOI – Alternate if no paper.
    $form_element['dataset-doi'] = [
      '#type' => 'textfield',
      '#title' => t('Dataset DOI:'),
      '#states' => $subelement_states,
      '#description' => tpps_form_get_doi_examples(),
      // @TODO Must be required when 'Paper DOI' is empty.
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

  }

}
