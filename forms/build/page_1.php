<?php

/**
 * @file
 * TPPS Page 1. Creates the Publication/Species Information page.
 */

require_once 'page_1_helper.php';
require_once 'page_1_ajax.php';

/**
 * Creates the Publication/Species Information form page.
 *
 * This function mainly calls the helper functions user_info, publication, and
 * organism.
 *
 * WARNING: $form is not empty and must be updated but not replaced.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 *
 * @return array
 *   The completed Publication/Species Information form.
 */
function tpps_page_1_create_form(array &$form, array &$form_state) {
  $is_tppsc = (($form_state['build_info']['form_id'] ?? 'tpps_main') == 'tppsc_main');
  if ($is_tppsc) {
    // TPPSc form provides more features for Curation Team.
    tpps_page_1_create_curation_form($form, $form_state);
  }
  else {
    // TPPS Form.
    $form = array_merge($form, tpps_page_1_create_regular_form($form, $form_state));
  }
  tpps_add_css_js(TPPS_PAGE_1, $form);
  return $form;
}

/**
 * Builds simple TPPS Page 1 form.
 *
 * @todo Change code to remove this function.
 */
function tpps_page_1_create_regular_form(array $form, array &$form_state) {
  // TPPS Version.
  $saved_values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];

  // @TODO [VS] Get rid of '$saved_values'.
  tpps_user_info($form, $saved_values);
  tpps_publication($form, $saved_values, $form_state);

  $file_upload_location = 'public://' . variable_get('tpps_study_photo_files_dir', 'tpps_study_photos');
  $form['study_photo'] = array(
    '#type' => 'fieldset',
    '#title' => '<div class="fieldset-title">Study Cover Photo: (Optional)</div>',
    '#tree' => FALSE,
    '#collapsible' => TRUE,
  );

  $form['study_photo']['photo'] = array(
    '#type' => 'managed_file',
    '#title' => t('Please upload a cover photo for your study. This photo will be displayed at the top of the landing page of the study.'),
    '#upload_location' => "$file_upload_location",
    '#upload_validators' => array(
      'file_validate_extensions' => array('img jpg jpeg png'),
    ),
    '#default_value' => $form_state['saved_values'][TPPS_PAGE_1]['photo'] ?? NULL,
  );

  tpps_organism($form, $form_state);
  tpps_add_buttons($form, 'page_1');
  return $form;
}

/**
 * Creates  TPPS Page 1 form for curation team.
 *
 * WARNING:
 * $form will be updated (not returned).
 *
 * How it works?
 * DOI information could be loaded in browser using AJAX.
 * Form must be build as usual and values must be found in $form_state.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 *
 * @todo Test if existing study is shown correctly.
 */
function tpps_page_1_create_curation_form(array &$form, array &$form_state) {
  $saved_values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];
  $values = $form_state['values'];

  $form['#attached']['js'][] = [
    'type' => 'setting',
    'data' => [
      'tpps' => [
        'ajaxUrl' => TPPS_AJAX_URL,
        'cache' => variable_get('tpps_page_1_cache_ajax_responses', TRUE),
      ]
    ],
    'scope' => 'footer',
  ];

  $doi = tpps_get_ajax_value($form_state, ['doi'], '');
  $org_number = tpps_get_ajax_value($form_state, ['organism', 'number']) ?? 1;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Publication.
  $form['publication'] = [
    '#type' => 'fieldset',
    '#title' => t('Publication Information'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#prefix' => '<div id="publication-container">',
    '#suffix' => '</div>',
  ];
  $form['publication']['status'] = [
    '#type' => 'select',
    '#title' => t('Publication Status: *'),
    '#options' => [
      0 => t('- Select -'),
      'In Preparation or Submitted' => t('In Preparation or Submitted'),
      'In Press' => t('In Press'),
      'Published' => t('Published'),
    ],
    '#default_value' => tpps_get_ajax_value($form_state, ['publication', 'status'], ''),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // DOI Fields.
  //
  // Note:
  // Checkbox 'use_old_tgdr' is defined in TPPSc/forms/build/front.php.
  // Accession will be stored in 'old_tgdr' field.
  // @TODO Minor. Rename field to 'publication_doi'.
  $form['publication']['doi'] = [
    '#type' => 'textfield',
    '#title' => t('Publication DOI:')
      . tpps_page_1_required_by_status($form_state),
    '#tree' => FALSE,
    '#parents' => ['doi'],
    '#default_value' => $doi,
    '#description' => tpps_page_1_get_doi_examples(),
    '#prefix' => '<div id="doi-message"></div>',
    '#states' => [
      'visible' => [
        [':input[name="publication[status]"]' => ['value' => 'Published']],
        'or',
        [':input[name="publication[status]"]' => ['value' => 'In Preparation or Submitted']],
      ],
    ],
  ];

  $form['publication']['dataset_doi'] = [
    '#type' => 'textfield',
    '#title' => t('Dataset DOI:'),
    '#parents' => ['dataset_doi'],
    '#tree' => FALSE,
    '#default_value' => tpps_get_ajax_value($form_state, ['dataset_doi'], ''),
    '#description' => tpps_page_1_get_doi_examples(),
    '#states' => [
      'visible' => [
        [':input[name="publication[status]"]' => ['value' => 'Published']],
        'or',
        [':input[name="publication[status]"]' => ['value' => 'In Preparation or Submitted']],
      ],
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Primary Author.
  // Element '#parents' doesn't work but '#tree' => FALSE works.
  $form['publication']['primaryAuthor'] = [
    '#type' => 'textfield',
    '#title' => t('Primary Author: *'),
    '#tree' => FALSE,
    '#autocomplete_path' => 'tpps/autocomplete/author',
    '#attributes' => [
      'data-toggle' => ['tooltip'],
      'data-placement' => ['right'],
      'title' => ['First Author of the publication'],
    ],
    '#description' => t('Note: please format in ‘Last, First’ format.'),
    '#default_value' => tpps_get_ajax_value($form_state, ['primaryAuthor'], NULL),
  ];
  // Update field's value.

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show publication extra fields.
  // Publication Year.
  $year_options = range(1900, date('Y'), 1);
  $year_options = [0 => '- Select -']
    + array_combine($year_options, $year_options);
  $form['publication']['year'] = [
    '#type' => 'select',
    '#title' => t('Year of Publication:')
      . tpps_page_1_required_by_status($form_state),
    '#options' => $year_options,
    '#description' => t('If your publication has not been published yet, '
      . 'please choose the expected year of publication.'),
    '#default_value' => tpps_get_ajax_value($form_state, ['publication', 'year'], 0),
    '#states' => [
      'visible' => [
        [':input[name="publication[status]"]' => ['value' => 'Published']],
        'or',
        [':input[name="publication[status]"]' => ['value' => 'In Preparation or Submitted']],
      ],
    ],
  ];

  $form['publication']['title'] = [
    '#type' => 'textfield',
    '#title' => t('Title of Publication/Study:')
      . tpps_page_1_required_by_status($form_state),
    '#default_value' => tpps_get_ajax_value($form_state, ['publication', 'title'], ''),
    '#states' => [
      'visible' => [
        [':input[name="publication[status]"]' => ['value' => 'Published']],
        'or',
        [':input[name="publication[status]"]' => ['value' => 'In Preparation or Submitted']],
      ],
    ],
  ];

  $form['publication']['abstract'] = [
    '#type' => 'textarea',
    '#title' => t('Abstract/Description:')
      . tpps_page_1_required_by_status($form_state),
    '#default_value' => tpps_get_ajax_value($form_state, ['publication', 'abstract'], ''),
    '#states' => [
      'visible' => [
        [':input[name="publication[status]"]' => ['value' => 'Published']],
        'or',
        [':input[name="publication[status]"]' => ['value' => 'In Preparation or Submitted']],
      ],
    ],
  ];

  $form['publication']['journal'] = [
    '#type' => 'textfield',
    '#title' => t('Journal:')
      . tpps_page_1_required_by_status($form_state),
    '#autocomplete_path' => 'tpps/autocomplete/journal',
    '#default_value' => tpps_get_ajax_value($form_state, ['publication', 'journal'], ''),
    '#states' => [
      'visible' => [
        [':input[name="publication[status]"]' => ['value' => 'Published']],
        'or',
        [':input[name="publication[status]"]' => ['value' => 'In Preparation or Submitted']],
      ],
    ],
  ];

  // @TODO Check. Mockup has no fieldset - just buttons.
  tpps_secondary_authors($form, $saved_values, $form_state);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Organisms.
  tpps_organism($form, $form_state);

  // Note: $form_state['ids']['project_id'] is widly used.
  $form_state['ids']['project_id'] = tpps_get_project_id($form_state['dbxref_id']);

  // Load existing study data.
  if (!empty($form_state['saved_values']['frontpage']['use_old_tgdr'])) {
    // Usage of existing accession:
    // $values['accession'] = TGDR864;
    // $values['use_old_tgdr'] => 1;
    // $values['old_tgdr'] = 10514311;
    // Refers to public.tpps_submission.dbxref_id;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Note: Code which restores submission data from different db tables
    // and fills form fields was removed in branch VS/page1_improvements.
  }

  tpps_add_buttons($form, 'page_1');
  return $form;
}

/**
 * Gets Project Id.
 *
 * Those Project Id is used in submit_all.php script reports.
 * Project Id could be shared between multiple submissions. So when submitted
 * the same data Project Id will be the same.
 *
 * @param int $dbxref_id
 *   This number could be obtained from table chado.dbxref
 *   by accession (TGDRxxx) and db_id = 92 (TreeGenes database).
 *
 * @return int
 *   Returns Project Id.
 */
function tpps_get_project_id($dbxref_id) {
  $result = chado_select_record(
    'project_dbxref', ['project_id'], ['dbxref_id' => $dbxref_id]
  );
  return ($result) ? $result[0]->project_id : FALSE;
}
