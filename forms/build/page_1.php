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
    '#default value' => $publication_status,
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
  $form['publication']['doi_container']['doi'] = [
    '#type' => 'textfield',
    '#title' => t('Publication DOI: *'),
    '#ajax' => [
      'callback' => 'tpps_ajax_doi_callback',
      'wrapper' => 'publication-extra-container',
    ],
    '#description' => 'Example: '
      // @TODO Use l().
      . '<a href"#" class="tpps-suggestion">10.1111/dryad.111</a>, '
      . '<a href"#" class="tpps-suggestion">10.25338/B8864J</a>',
    // AJAX-callback 'tpps_ajax_doi_callback()' will search database for
      // doi field in the root of form we change parents here.
      // This allows to reuse existing code and existing studies.
    '#parents' => ['doi'],
  ];
  $form['publication']['doi_container']['dataset_doi'] = [
    '#type' => 'textfield',
    '#title' => t('Dryad DOI:'),
    '#description' => 'Examples: '
      // @TODO Use l().
      . '<a href"#" class="tpps-suggestion">10.1111/dryad.111</a>, '
      . '<a href"#" class="tpps-suggestion">10.25338/B8864J</a>',
    '#parents' => ['dataset_doi'],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Primary Author.
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
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show extra fields.
  $form['publication']['extra'] = [
    // Using '#attributes' blocks '#states' so '#prefix' and '#suffix' are used.
    '#prefix' => '<div id="publication-extra-container">',
    '#suffix' => '</div>',
    // '#attributes' => ['id' => ['publication-extra-container']],
    '#type' => 'container',
    '#states' => [
      'visible' => [
        ':input[name="publication[status]"]' => ['value' => 'Published'],
      ],
    ],
  ];
  // if(isset($saved_values['primaryAuthor']) && $saved_values['primaryAuthor'] != "") {
  //   $form['publication']['primaryAuthor']['#value'] = $saved_values['primaryAuthor'];
  // }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Publication Year.
  $year_options = range(1900, date('Y'), 1);
  $year_options = [0 => '- Select -']
    + array_combine($year_options, $year_options);
  $form['publication']['extra']['year'] = [
    '#type' => 'select',
    '#title' => t('Year of Publication: *'),
    '#options' => $year_options,
    '#description' => t('If your publication has not been published yet, '
      . 'please choose the expected year of publication.'),
    // Exclude 'extra' just to have clear data structure.
    '#parents' => ['publication', 'year'],
  ];
  // if(isset($values['publication']['year'])) {
  //   $form['publication']['year']['#value'] = $values['publication']['year'];
  // }

  $form['publication']['extra']['title'] = [
    '#type' => 'textfield',
    '#title' => t('Title of Publication/Study: *'),
    // Exclude 'extra' just to have clear data structure.
    '#parents' => ['publication', 'title'],
  ];
  // if(isset($saved_values['publication']['title']) && $saved_values['publication']['title'] != "") {
  //   $form['publication']['title']['#value'] = $saved_values['publication']['title'];
  // }

  $form['publication']['extra']['abstract'] = [
    '#type' => 'textarea',
    '#title' => t('Abstract/Description: *'),
    // Exclude 'extra' just to have clear data structure.
    '#parents' => ['publication', 'abstract'],
  ];
  // if(isset($saved_values['publication']['abstract']) && $saved_values['publication']['abstract'] != "") {
  //   $form['publication']['abstract']['#value'] = $saved_values['publication']['abstract'];
  // }

  $form['publication']['extra']['journal'] = [
    '#type' => 'textfield',
    '#title' => t('Journal: *'),
    '#autocomplete_path' => 'tpps/autocomplete/journal',
    // Exclude 'extra' just to have clear data structure.
    '#parents' => ['publication', 'journal'],
  ];
  // if(isset($saved_values['publication']['journal']) && $saved_values['publication']['journal'] != "") {
  //   $form['publication']['journal']['#value'] = $saved_values['publication']['journal'];
  // }



  // @TODO Mockup has no fieldset - just buttons.
  tpps_secondary_authors($form, $saved_values, $form_state);

  // @TODO Review. This could be useful for existing studies.
  if (!empty($doi_info)) {
    $form['publication']['status']['#value'] = 'Published';
    $form['publication']['status']['#disabled'] = TRUE;

    $form['doi_container']['doi_message']['#markup'] =
      "The publication has been successfully loaded from Dryad";

    $form['primaryAuthor']['#default_value'] = $doi_info['primary'] ?? "";
    $form['publication']['extra']['year']['#default_value'] = $doi_info['year'] ?? "";
    $form['publication']['extra']['title']['#default_value'] = $doi_info['title'] ?? "";
    $form['publication']['extra']['abstract']['#default_value'] = $doi_info['abstract'] ?? "";
    $form['publication']['extra']['journal']['#default_value'] = $doi_info['journal'] ?? "";
  }
  else {
    // DOI Info is empty.

  }
  if (empty($form_state['saved_values']['frontpage']['use_old_tgdr'])) {
    // Show DOI fieldset only for 'Published' status.
    //$form['doi_wrapper']['#states'] = [
    //  'visible' => [
    //    ':input[name="publication[status]"]' => ['value' => 'Published'],
    //  ],
    //];
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
    }
  }
  tpps_organism($form, $form_state);





  if (!empty($saved_values['frontpage']['use_old_tgdr'])) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Not empty DOI.
    $form_state['saved_values'][TPPS_PAGE_1]['publication']['status'] = 'Published';

    $form_state['ids']['project_id'] = $project_id = chado_select_record(
      'project_dbxref', ['project_id'], ['dbxref_id' => $form_state['dbxref_id']]
    )[0]->project_id;

    $publication = tpps_get_publication_data($project_id, $form_state);
    $form['publication']['primaryAuthor']['#default_value'] = $publication['primary_default'];
    $form['publication']['title']['#default_value'] = $publication['title_default'];
    $form['publication']['year']['#default_value'] = $publication['year_default'];
    $form['publication']['abstract']['#default_value'] = $publication['abs_default'];



    $form['publication']['journal']['#title'] = t('Journal:');
    $form['publication']['status']['#title'] = t('Publication Status:');
    $form['publication']['status']['#disabled'] = TRUE;

    $i = 1;
    foreach ($publication['secondary_authors'] as $author) {
      $form['publication']['secondaryAuthors'][$i]['#default_value']
        = $saved_values['publication']['secondaryAuthors'][$i]
        ?? "$author->givennames $author->surname";
      $i++;
    }

    tppsc_organism($form, $form_state);
    $organisms = chado_query('SELECT genus, species '
      . 'FROM chado.organism WHERE organism_id IN ('
        . 'SELECT DISTINCT organism_id '
        . 'FROM chado.stock '
        . 'WHERE stock_id IN ('
          . 'SELECT stock_id '
          . 'FROM chado.project_stock '
          . 'WHERE project_id = :project_id));',
          [':project_id' => $project_id]
    );
    $i = 1;
    foreach ($organisms as $org) {
      $form['organism'][$i]['#default_value'] = $saved_values['organism'][$i]
        ?? "$org->genus $org->species";
      $i++;
    }
    $form['organism']['number'] = [
      '#type' => 'hidden',
      '#value' => tpps_get_ajax_value($form_state, ['organism', 'number'], $i - 1),
    ];
  }

  tpps_add_buttons($form, 'page_1');
  return $form;









}

/**
 * Gets publication data from database or from previously submitted data.
 *
 * @param int $project_id
 *   Project Id.
 * @param array $form_state
 *   Drupal Form API state array.
 *
 * @return array
 *   Returns publication's data to be used to set default values.
 *
 * @TODO [VS] Pass $form by reference and update default values of fields.
 */
function tpps_get_publication_data($project_id, array $form_state) {
  $saved_values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];
  // Get publication Id.
  $pub_id = chado_select_record(
    'project_pub', ['pub_id'], ['project_id' => $project_id]
  )[0]->pub_id;

  // Publication Title and Year.
  $pub = chado_select_record('pub', ['*'], ['pub_id' => $pub_id])[0];
  $data['title_default'] = $saved_values['publication']['title'] ?? $pub->title;
  $data['year_default'] = $saved_values['publication']['year'] ?? $pub->pyear;
  // Abstract.
  if (!empty($saved_values['publication']['abstract'])) {
    $data['abs_default'] = $saved_values['publication']['abstract'];
  }
  else {
    $data['abs_default'] = chado_select_record(
      'pubprop',
      ['value'],
      [
        'pub_id' => $data['pub_id'],
        'type_id' => [
          'name' => 'Abstract',
          'cv_id' => ['name' => 'tripal_pub'],
        ],
      ]
    )[0]->value;
  }
  // Primary Author.
  if (!empty($saved_values['publication']['primaryAuthor'])) {
    $data['primary_default'] = $saved_values['publication']['primaryAuthor'];
  }
  else {
    $primary_author = chado_select_record(
      'pubauthor',
      ['givennames', 'surname'],
      [
        'pub_id' => $pub_id,
        'rank' => 0,
      ]
    )[0];
    $data['primary_default'] = "$primary_author->givennames $primary_author->surname";
  }
  // List of secondary authors.
  $data['secondary_authors'] = chado_select_record(
    'pubauthor',
    ['givennames', 'surname'],
    [
      'pub_id' => $pub_id,
      'rank' => ['op' => '!=', 'data' => 0],
    ]
  );
  return $data;
}
