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
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 *
 * @return array
 *   The completed Publication/Species Information form.
 */
function tpps_page_1_create_form(array &$form, array &$form_state) {
  //dpm($form_state['build_info']['form_id']);
  // TPPSc was created to provide more features for Curation Team.
  // See 'TPPSc' module.
  $is_tppsc = (($form_state['build_info']['form_id'] ?? 'tpps_main') == 'tppsc_main');
  if ($is_tppsc) {
    // TPPSc Version (Curation version).
    $saved_values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];

    if (empty($form_state['saved_values']['frontpage']['use_old_tgdr'])) {
      // [VS]
      // @TODO Minor. Rename field to 'dataset_doi'.
      $form['doi'] = [
        '#type' => 'textfield',
        '#title' => t('Publication DOI: *'),
        '#ajax' => [
          'callback' => 'tppsc_ajax_doi_callback',
          'wrapper' => "doi-wrapper",
        ],
        '#description' => 'Example: '
          . '<a href"#" class="tpps-suggestion">10.1111/dryad.111</a>, '
          . '<a href"#" class="tpps-suggestion">10.25338/B8864J</a>',
        '#prefix' => '<div style="text-align: right;"></div>',
      ];
      $form['dataset_doi'] = [
        '#type' => 'textfield',
        '#title' => t('Dataset DOI:'),
        '#description' => 'Examples: '
          . '<a href"#" class="tpps-suggestion">10.1111/dryad.111</a>, '
          . '<a href"#" class="tpps-suggestion">10.25338/B8864J</a>',
        '#prefix' => '<div style="text-align: right;"></div>',
      ];

      $doi = tpps_get_ajax_value($form_state, ['doi']);

      $species = [];
      $form['primaryAuthor'] = ['#type' => 'hidden', '#disabled' => TRUE];
      $form['publication'] = ['#type' => 'hidden'];
      $form['organism'] = ['#type' => 'fieldset'];
      if (!empty($doi)) {
        // @todo Replace with TPPS module.
        module_load_include('inc', 'tppsc', 'includes/doi');
        $doi_info = tppsc_doi_info($doi);
        if (!empty($doi_info)) {
          // dpm('DOI info contains pub info');
          $species = $doi_info['species'] ?? [];

          $form_state['saved_values'][TPPS_PAGE_1]['publication']['status'] = 'Published';
          $tpps_form = [];
          $tpps_form = tpps_simple_page_1_create_form($form, $form_state);
          $form['primaryAuthor'] = $tpps_form['primaryAuthor'];
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
          // dpm('DOI Info is empty');
          $tpps_form = [];
          $tpps_form = tpps_simple_page_1_create_form($form, $form_state);
          $form['primaryAuthor'] = $tpps_form['primaryAuthor'];
          $form['publication'] = $tpps_form['publication'];
        }
      }
      // [/VS]
      $org_number = tpps_get_ajax_value($form_state, array('organism', 'number'));
      if (!isset($org_number) and !empty($species)) {
        $org_number = $form_state['values']['organism']['number'] = count($species);
      }
      for ($i = 1; $i <= $org_number; $i++) {
        $org = tpps_get_ajax_value($form_state, array('organism', $i));
        if (empty($org) and !empty($species[$i - 1])) {
          $form_state['values']['organism'][$i] = $species[$i - 1];
        }
      }

      tpps_organism($form, $form_state);

      $form['#prefix'] = '<div id="doi-wrapper">' . $form['#prefix'];
      $form['#suffix'] = (!empty($form['#suffix'])) ? $form['#suffix'] . '</div>' : '</div>';

      $form['Save'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
        '#prefix' => '<div class="input-description">* : Required Field</div>',
      );

      $form['Next'] = array(
        '#type' => 'submit',
        '#value' => t('Next'),
      );

      return $form;
    }

    $form_state['saved_values'][TPPS_PAGE_1]['publication']['status'] = 'Published';
    $tpps_form = array();
    $tpps_form = tpps_page_1_create_form($tpps_form, $form_state);

    $form_state['ids']['project_id'] = chado_select_record('project_dbxref', array('project_id'), array(
      'dbxref_id' => $form_state['dbxref_id'],
    ))[0]->project_id;

    $pub_id = chado_select_record('project_pub', array('pub_id'), array(
      'project_id' => $form_state['ids']['project_id'],
    ))[0]->pub_id;

    $pub = chado_select_record('pub', array('*'), array(
      'pub_id' => $pub_id,
    ))[0];
    $title_default = $saved_values['publication']['title'] ?? $pub->title;
    $year_default = $saved_values['publication']['year'] ?? $pub->pyear;

    $abstract = chado_select_record('pubprop', array('value'), array(
      'pub_id' => $pub_id,
      'type_id' => array(
        'name' => 'Abstract',
        'cv_id' => array(
          'name' => 'tripal_pub',
        ),
      ),
    ))[0]->value;
    $abs_default = $saved_values['publication']['abstract'] ?? $abstract;

    $primary_author = chado_select_record('pubauthor', array('givennames', 'surname'), array(
      'pub_id' => $pub_id,
      'rank' => 0,
    ))[0];
    $primary_default = $saved_values['primaryAuthor'] ?? "$primary_author->givennames $primary_author->surname";

    $secondary_authors = chado_select_record('pubauthor', array('givennames', 'surname'), array(
      'pub_id' => $pub_id,
      'rank' => array(
        'op' => '!=',
        'data' => 0,
      ),
    ));

    $organisms = chado_query('SELECT genus, species '
      . 'FROM chado.organism WHERE organism_id IN ('
        . 'SELECT DISTINCT organism_id '
        . 'FROM chado.stock '
        . 'WHERE stock_id IN ('
          . 'SELECT stock_id '
          . 'FROM chado.project_stock '
          . 'WHERE project_id = :project_id));', array(':project_id' => $form_state['ids']['project_id']));

    $form['primaryAuthor'] = $tpps_form['primaryAuthor'];
    $form['primaryAuthor']['#default_value'] = $primary_default;

    $form['publication'] = $tpps_form['publication'];
    $form['publication']['title']['#default_value'] = $title_default;
    $form['publication']['year']['#default_value'] = $year_default;
    $form['publication']['abstract']['#default_value'] = $abs_default;

    $form['organism'] = $tpps_form['organism'];

    $form['publication']['journal']['#title'] = t('Journal:');
    $form['publication']['status']['#title'] = t('Publication Status:');
    $form['publication']['status']['#disabled'] = TRUE;

    $i = 1;
    foreach ($secondary_authors as $author) {
      $form['publication']['secondaryAuthors'][$i]['#default_value'] = $saved_values['publication']['secondaryAuthors'][$i] ?? "$author->givennames $author->surname";
      $i++;
    }

    $i = 1;
    foreach ($organisms as $org) {
      $form['organism'][$i]['#default_value'] = $saved_values['organism'][$i] ?? "$org->genus $org->species";
      $i++;
    }
    $form['organism']['number'] = array(
      '#type' => 'hidden',
      '#value' => tpps_get_ajax_value($form_state, array('organism', 'number'), $i - 1),
    );

    $form['Save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#prefix' => '<div class="input-description">* : Required Field</div>',
    );

    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );

  }
  else {
    // TPPS Form.
    $form = array_merge($form, tpps_simple_page_1_create_form($form, $form_state));
  }

  return $form;
}

/**
 * Builds simple TPPS Page 1 form.
 *
 * @todo Change code to remove this function.
 */
function tpps_simple_page_1_create_form(array $form, array &$form_state) {
  // TPPS Version.
  if (isset($form_state['saved_values'][TPPS_PAGE_1])) {
    $values = $form_state['saved_values'][TPPS_PAGE_1];
  }
  else {
    $values = array();
  }

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

  $form['Save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#prefix' => '<div class="input-description">* : Required Field</div>',
  );

  $form['Next'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );
  return $form;
}
