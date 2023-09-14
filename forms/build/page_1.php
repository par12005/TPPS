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
  tpps_add_css_js($form, TPPS_PAGE_1);
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
 * WARNING: Update $form passed by reference.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 */
function tpps_page_1_create_curation_form(array &$form, array &$form_state) {
  $saved_values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];
  $values = $form_state['values'];

  $publication_status = tpps_get_ajax_value(
    $form_state, ['publication', 'status'], NULL
  );
  if (!empty($doi = tpps_get_ajax_value($form_state, ['doi']))) {
    module_load_include('inc', 'tpps', 'includes/manage_doi');
    $doi_info = tpps_doi_info($doi);
    dpm($doi_info, 'doi');
  }
  $species = $doi_info['species'] ?? [];
  $org_number = tpps_get_ajax_value($form_state, ['organism', 'number'])
    ?? $form_state['values']['organism']['number'] ?? count($species) ?? 1;

  //dpm(print_r($doi, 1));
  //dpm(print_r($doi_info, 1));
  //dpm(print_r($form_state, 1));

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Publication.
  $form['publication'] = [
    '#type' => 'fieldset',
    '#title' => t('Publication Information'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#theme_wrappers' => ['publication-container'],
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


// @TODO Remove debug code!

    '#default_value' => 'Published',
    //'#default_value' => (!empty($doi_info) ? 'Published' : $publication_status),


    '#disabled' => !empty($doi_info),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // DOI Fields.
  //
  // Note:
  // Checkbox 'use_old_tgdr' is defined in TPPSc/forms/build/front.php.
  // Accession will be stored in 'old_tgdr' field.
  $form['publication']['doi_container'] = [
    '#type' => 'container',
    '#states' => [
      'visible' => [
        ':input[name="publication[status]"]' => ['value' => 'Published'],
      ],
    ],
  ];

  // @TODO Minor. Rename field to 'publication_doi'.
  $parents = ['doi'];
  $form['publication']['doi_container']['doi'] = [
    '#type' => 'textfield',
    '#title' => t('Publication DOI: *'),
    '#ajax' => [
      'callback' => 'tpps_ajax_doi_callback',
      'wrapper' => 'publication-container',
    ],
    '#parents' => $parents,
    '#default_value' => tpps_get_ajax_value($form_state, $parents, NULL),
    '#description' => 'Example: '
      // @TODO Use l().
      . '<a href"#" class="tpps-suggestion">10.1111/dryad.111</a>, '
      . '<a href"#" class="tpps-suggestion">10.25338/B8864J</a>',
    // AJAX-callback 'tpps_ajax_doi_callback()' will search database for
    // doi field in the root of form we change parents here.
    // This allows to reuse existing code and existing studies.
  ];
  $parents = ['dataset_doi'];
  $form['publication']['doi_container']['dataset_doi'] = [
    '#type' => 'textfield',
    '#title' => t('Dryad DOI:'),
    '#parents' => $parents,
    '#default_value' => tpps_get_ajax_value($form_state, $parents, NULL),
    '#description' => 'Examples: '
      // @TODO Use l().
      . '<a href"#" class="tpps-suggestion">10.1111/dryad.111</a>, '
      . '<a href"#" class="tpps-suggestion">10.25338/B8864J</a>',
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Primary Author.
  $parents = ['primaryAuthor'];
  $form['publication']['primaryAuthor'] = [
    '#type' => 'textfield',
    '#title' => t('Primary Author: *'),
    '#autocomplete_path' => 'tpps/autocomplete/author',
    '#attributes' => [
      'data-toggle' => ['tooltip'],
      'data-placement' => ['right'],
      'title' => ['First Author of the publication'],
    ],
    '#description' => t('Note: please format in ‘Last, First’ format.'),
    '#parents' => $parents,
    '#default_value' => $doi_info['primary']
    ?? tpps_get_ajax_value($form_state, $parents, NULL),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show extra fields.
  $form['publication']['extra'] = [
    '#type' => 'container',
    '#states' => [
      'visible' => [
        ':input[name="publication[status]"]' => ['value' => 'Published'],
      ],
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Publication Year.
  $year_options = range(1900, date('Y'), 1);
  $year_options = [0 => '- Select -']
    + array_combine($year_options, $year_options);
  $parents = ['publication', 'year'];
  $form['publication']['extra']['year'] = [
    '#type' => 'select',
    '#title' => t('Year of Publication: *'),
    '#options' => $year_options,
    '#description' => t('If your publication has not been published yet, '
      . 'please choose the expected year of publication.'),
    // Exclude 'extra' just to have clear data structure.
    '#parents' => $parents,
    '#default_value' => $doi_info['year']
    ?? tpps_get_ajax_value($form_state, $parents, NULL),
  ];

  $parents = ['publication', 'title'];
  $form['publication']['extra']['title'] = [
    '#type' => 'textfield',
    '#title' => t('Title of Publication/Study: *'),
    // Exclude 'extra' just to have clear data structure.
    '#parents' => $parents,
    '#default_value' => $doi_info['title']
    ?? tpps_get_ajax_value($form_state, $parents, NULL),
  ];

  $parents = ['publication', 'abstract'];
  $form['publication']['extra']['abstract'] = [
    '#type' => 'textarea',
    '#title' => t('Abstract/Description: *'),
    // Exclude 'extra' just to have clear data structure.
    '#parents' => $parents,
    '#default_value' => $doi_info['abstract']
    ?? tpps_get_ajax_value($form_state, $parents, NULL),
  ];

  $parents = ['publication', 'journal'];
  $form['publication']['extra']['journal'] = [
    '#type' => 'textfield',
    '#title' => t('Journal: *'),
    '#autocomplete_path' => 'tpps/autocomplete/journal',
    // Exclude 'extra' just to have clear data structure.
    '#parents' => $parents,
    '#default_value' => $doi_info['journal']
    ?? tpps_get_ajax_value($form_state, $parents, NULL),
  ];

  // @TODO Check. Mockup has no fieldset - just buttons.
  tpps_secondary_authors($form, $saved_values, $form_state);

  if (!empty($doi_info)) {
    $form['doi_container']['doi_message']['#markup'] =
      "The publication has been successfully loaded from Dryad";
  }
  else {
    // DOI Info is empty.
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Organisms.
  //
  // Get data from DOI.
  // Fill form with values from:
  // 1. Used previously stored study. From $form_state['saved_values']?
  // 2. PUblished. From DOI. Read only?
  // 3. From $form_state (when returned from step 2).
  for ($i = 1; $i <= $org_number; $i++) {
    $org = tpps_get_ajax_value($form_state, ['organism', $i]);
    if (empty($org) and !empty($species[$i - 1])) {
      $form_state['values']['organism'][$i] = $species[$i - 1];
      //dpm($species[$i - 1]);
    }
  }

  tpps_organism($form, $form_state);
  // $form_state['ids']['project_id'] is widly used.
  $form_state['ids']['project_id'] = tpps_get_project_id($form_state['dbxref_id']);

  // Load existing study data.
  if (!empty($form_state['saved_values']['frontpage']['use_old_tgdr'])) {
    // Usage of existing accession:
    // $values['accession'] = TGDR864;
    // $values['use_old_tgdr'] => 1;
    // $values['old_tgdr'] = 10514311;
    // Refers to public.tpps_submission.dbxref_id;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Publication status.
    $form['publication']['status'] = 'Published';
    $form['publication']['status']['#disabled'] = TRUE;
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
  )[0]->project_id;
  return ($result) ? $result[0]->project_id : FALSE;
}
