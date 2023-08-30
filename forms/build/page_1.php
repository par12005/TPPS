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
  $values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];

  // @TODO [VS] Get rid of '$values'.
  tpps_user_info($form, $values);
  tpps_publication($form, $values, $form_state);

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
 * Creates fields related to publication for TPPSc form.
 *
 * New field and their relations described:
 * https://www.figma.com/file/RghCxBg7HLFWAC04i1QleS/TPPSc.
 *
 * Note:
 * For now TPPS and TPPSc (version for curation team) are using different form
 * element's layout and logic so there is no need to reuse the same code
 * because it increases difficulty of management of the code.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 *
 * @return array
 *   Returns 'Publication Information' fieldset with form elements.
 */
function tpps_curation_publication(array &$form, array $form_state) {

  $values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];
  $form['publication'] = [
    '#type' => 'fieldset',
    '#title' => t('Publication Information:'),
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
    '#ajax' => [
      'callback' => 'tpps_pub_status',
      'wrapper' => 'pubyear',
    ],
  ];
  $form['publication']['primaryAuthor'] = [
    '#type' => 'textfield',
    '#title' => t('Primary Author: *'),
    '#autocomplete_path' => 'tpps/autocomplete/author',
    '#attributes' => [
      'data-toggle' => ['tooltip'],
      'data-placement' => ['right'],
      'title' => ['First Author of the publication'],
    ],
  ];

  return $form;


  if (!empty($doi_info) && 0) {
    $form_state['saved_values'][TPPS_PAGE_1]['publication']['status'] = 'Published';
    $tpps_form = [];

    //tpps_curation_publication($form, $form_state);
    //$tpps_form = tpps_page_1_create_regular_form($form, $form_state);
    $form['publication']['primaryAuthor'] = $tpps_form['primaryAuthor'];
    $form['publication'] = $tpps_form['publication'];

    $form['publication']['journal']['#title'] = t('Journal:');
    $form['publication']['status']['#title'] = t('Publication Status:');
    $form['publication']['status']['#disabled'] = TRUE;

    $form['doi']['#suffix'] = "The publication has been successfully loaded from Dryad<br>";
    $form['primaryAuthor']['#default_value'] = $doi_info['primary'] ?? "";
    $form['publication']['title']['#default_value'] = $doi_info['title'] ?? "";
    $form['publication']['abstract']['#default_value'] = $doi_info['abstract'] ?? "";
    $form['publication']['journal']['#default_value'] = $doi_info['journal'] ?? "";
    $form['publication']['year']['#default_value'] = $doi_info['year'] ?? "";
  }
  else {
    // DOI Info is empty.
    $form['publication']['primaryAuthor'] = [
      '#type' => 'hidden',
      '#disabled' => TRUE,
    ];
    $form['publication'] = ['#type' => 'hidden'];
    $form['organism'] = ['#type' => 'fieldset'];
  }




  // @TODO Check if $values respects '#tree'.
  // if(isset($values['primaryAuthor']) && $values['primaryAuthor'] != "") {
  //   $form['publication']['primaryAuthor']['#value'] = $values['primaryAuthor'];
  // }

  if (0) {
    tpps_secondary_authors($form, $values, $form_state);
    tpps_year($form, $values, $form_state);

    $form['publication']['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Publication/Study: *'),
    );
    // if(isset($values['publication']['title']) && $values['publication']['title'] != "") {
    //   $form['publication']['title']['#value'] = $values['publication']['title'];
    // }

    $form['publication']['abstract'] = array(
      '#type' => 'textarea',
      '#title' => t('Abstract/Description: *'),
    );
    // if(isset($values['publication']['abstract']) && $values['publication']['abstract'] != "") {
    //   $form['publication']['abstract']['#value'] = $values['publication']['abstract'];
    // }

    $form['publication']['journal'] = array(
      '#type' => 'textfield',
      '#title' => t('Journal: *'),
      '#autocomplete_path' => 'tpps/autocomplete/journal',
    );
    // if(isset($values['publication']['journal']) && $values['publication']['journal'] != "") {
    //   $form['publication']['journal']['#value'] = $values['publication']['journal'];
    // }
  }

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
  $values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];

  //$publication_status = tpps_get_ajax_value(
  //  $form_state, ['publication', 'status'], NULL
  //);
  //dpm($publication_status);

  $org_number = tpps_get_ajax_value($form_state, ['organism', 'number']);
  if (!empty($doi = tpps_get_ajax_value($form_state, ['doi']))) {
    $doi_info = tpps_doi_info($doi);
  }
  $species = $doi_info['species'] ?? [];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Publication.
  tpps_curation_publication($form, $form_state);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // DOI Fields.
  //
  // Note:
  // Checkbox 'use_old_tgdr' is defined in TPPSc/forms/build/front.php.
  // Accession will be stored in 'old_tgdr' field.
  $form['doi_wrapper'] = [
    '#type' => 'container',
    '#attributes' => ['id' => 'doi-wrapper'],
    '#tree' => FALSE,
    // @TODO Before 'doi-wrapper' included whole form so when doi field
    // was filled whole form was rebuilt.
  ];
  // @TODO Minor. Rename field to 'publication_doi'.
  $form['doi_wrapper']['doi'] = [
    '#type' => 'textfield',
    '#title' => t('Publication DOI: *'),
    '#ajax' => [
      'callback' => 'tpps_ajax_doi_callback',
      'wrapper' => "doi-wrapper",
    ],
    '#description' => 'Example: '
      // @TODO Use l().
      . '<a href"#" class="tpps-suggestion">10.1111/dryad.111</a>, '
      . '<a href"#" class="tpps-suggestion">10.25338/B8864J</a>',
  ];
  $form['doi_wrapper']['dataset_doi'] = [
    '#type' => 'textfield',
    '#title' => t('Dryad DOI:'),
    '#description' => 'Examples: '
      // @TODO Use l().
      . '<a href"#" class="tpps-suggestion">10.1111/dryad.111</a>, '
      . '<a href"#" class="tpps-suggestion">10.25338/B8864J</a>',
  ];

  if (empty($form_state['saved_values']['frontpage']['use_old_tgdr'])) {


    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Organism.
    if (empty($org_number) and !empty($species)) {
      $org_number = $form_state['values']['organism']['number'] = count($species);
    }
    for ($i = 1; $i <= $org_number; $i++) {
      $org = tpps_get_ajax_value($form_state, ['organism', $i]);
      if (empty($org) and !empty($species[$i - 1])) {
        $form_state['values']['organism'][$i] = $species[$i - 1];
      }
    }
    tpps_organism($form, $form_state);
  }
  else {
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

    tpps_organism($form, $form_state);
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
  if (empty($form_state['saved_values']['frontpage']['use_old_tgdr'])) {
    // Show DOI fieldset only for 'Published' status.
    $form['doi_wrapper']['#states'] = [
      'visible' => [
        ':input[name="publication[status]"]' => ['value' => 'Published'],
      ],
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
