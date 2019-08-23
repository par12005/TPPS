<?php

/**
 * @file
 * Define the helper functions for the Publication/Species Information page.
 */

/**
 * This function creates fields describing the primary author.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_user_info(array &$form, array $values) {

  $form['primaryAuthor'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Author: *'),
    '#autocomplete_path' => 'author/autocomplete',
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('right'),
      'title' => array('First Author of the publication'),
    ),
  );

  $form['organization'] = array(
    '#type' => 'textfield',
    '#title' => t('Organization: *'),
    '#autocomplete_path' => 'organization/autocomplete',
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('right'),
      'title' => array('Organization of the Primary Author'),
    ),
  );

  return $form;
}

/**
 * This function creates fields describing the publication.
 *
 * This includes the secondary authors, status, year, title, abstract, and
 * journal.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_publication(array &$form, array $values, array $form_state) {

  $form['publication'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Publication Information:</div>'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
  );

  tpps_secondary_authors($form, $values, $form_state);

  $form['publication']['status'] = array(
    '#type' => 'select',
    '#title' => t('Publication Status: *'),
    '#options' => array(
      0 => t('- Select -'),
      'In Preparation or Submitted' => t('In Preparation or Submitted'),
      'In Press' => t('In Press'),
      'Published' => t('Published'),
    ),
    '#ajax' => array(
      'callback' => 'tpps_pub_status',
      'wrapper' => 'pubyear',
    ),
  );

  tpps_year($form, $values, $form_state);

  $form['publication']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title of Publication/Study: *'),
  );

  $form['publication']['abstract'] = array(
    '#type' => 'textarea',
    '#title' => t('Abstract/Description: *'),
  );

  $form['publication']['journal'] = array(
    '#type' => 'textfield',
    '#title' => t('Journal: *'),
    '#autocomplete_path' => 'journal/autocomplete',
  );

  return $form;
}

/**
 * This function creates fields describing the species in the publication.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The form_state of the form to be populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_organism(array &$form, array &$form_state) {

  if (isset($form_state['values']['organism']['number']) and $form_state['triggering_element']['#name'] == "Add Organism") {
    $form_state['values']['organism']['number']++;
  }
  elseif (isset($form_state['values']['organism']['number']) and $form_state['triggering_element']['#name'] == "Remove Organism" and $form_state['values']['organism']['number'] > 1) {
    $form_state['values']['organism']['number']--;
  }
  $org_number = isset($form_state['values']['organism']['number']) ? $form_state['values']['organism']['number'] : NULL;

  if (!isset($org_number) and isset($form_state['saved_values'][TPPS_PAGE_1]['organism']['number'])) {
    $org_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
  }
  if (!isset($org_number)) {
    $org_number = 1;
  }

  $form['organism'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('<div class="fieldset-title">Organism information:</div>'),
    '#description' => t('Up to 5 organisms per submission.'),
    '#collapsible' => TRUE,
    '#prefix' => '<div id="organism-wrapper">',
    '#suffix' => '</div>',
  );

  $form['organism']['add'] = array(
    '#type' => 'button',
    '#button_type' => 'button',
    '#value' => t('Add Organism'),
    '#name' => t('Add Organism'),
    '#ajax' => array(
      'wrapper' => 'organism-wrapper',
      'callback' => 'tpps_organism_callback',
    ),
  );

  $form['organism']['remove'] = array(
    '#type' => 'button',
    '#button_type' => 'button',
    '#value' => t('Remove Organism'),
    '#name' => t('Remove Organism'),
    '#ajax' => array(
      'wrapper' => 'organism-wrapper',
      'callback' => 'tpps_organism_callback',
    ),
  );

  $form['organism']['number'] = array(
    '#type' => 'hidden',
    '#value' => $org_number,
  );

  for ($i = 1; $i <= $org_number; $i++) {

    $form['organism']["$i"] = array(
      '#type' => 'textfield',
      '#title' => t("Species @num: *", array('@num' => $i)),
      '#autocomplete_path' => "species/autocomplete",
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('If your species is not in the autocomplete list, don\'t worry about it! We will create a new organism entry in the database for you.'),
      ),
    );
  }

  return $form;
}

/**
 * This function creates the year field for the publication.
 *
 * This field changes its options based on the selection made for publication
 * status.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_year(array &$form, array $values, array $form_state) {

  if (isset($form_state['values']['publication']['status']) and $form_state['values']['publication']['status'] != '0') {
    $pub_status = $form_state['values']['publication']['status'];
  }
  elseif (isset($form_state['saved_values'][TPPS_PAGE_1]['publication']['status']) and $form_state['saved_values'][TPPS_PAGE_1]['publication']['status'] != '0') {
    $pub_status = $form_state['saved_values'][TPPS_PAGE_1]['publication']['status'];
  }

  if (isset($pub_status) and $pub_status != 'Published') {
    $yearArr = array(0 => '- Select -');
    for ($i = 2017; $i <= date('Y') + 1; $i++) {
      $yearArr[$i] = "$i";
    }
  }
  elseif (isset($pub_status)) {
    $yearArr = array(0 => '- Select -');
    for ($i = 1990; $i <= date('Y'); $i++) {
      $yearArr[$i] = "$i";
    }
  }
  else {
    $yearArr = array(0 => '- Select -');
  }

  $form['publication']['year'] = array(
    '#type' => 'select',
    '#title' => t('Year of Publication: *'),
    '#options' => $yearArr,
    '#states' => array(
      'invisible' => array(
        ':input[name="publication[status]"]' => array('value' => '0'),
      ),
    ),
    '#prefix' => '<div id="pubyear">',
    '#suffix' => '</div>',
  );

  return $form;
}

/**
 * This function creates fields for the secondary authors of the publication.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_secondary_authors(array &$form, array $values, array $form_state) {

  $file_upload_location = 'public://' . variable_get('tpps_author_files_dir', 'tpps_authors');

  $form['publication']['secondaryAuthors'] = array(
    '#type' => 'fieldset',
  );

  $form['publication']['secondaryAuthors']['add'] = array(
    '#type' => 'button',
    '#title' => t('Add Secondary Author'),
    '#button_type' => 'button',
    '#value' => t('Add Secondary Author'),
  );

  $form['publication']['secondaryAuthors']['remove'] = array(
    '#type' => 'button',
    '#title' => t('Remove Secondary Author'),
    '#button_type' => 'button',
    '#value' => t('Remove Secondary Author'),
  );

  $form['publication']['secondaryAuthors']['number'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($values['publication']['secondaryAuthors']['number']) ? $values['publication']['secondaryAuthors']['number'] : '0',
  );

  for ($i = 1; $i <= 30; $i++) {

    $form['publication']['secondaryAuthors'][$i] = array(
      '#type' => 'textfield',
      '#title' => t("Secondary Author @num: *", array('@num' => $i)),
      '#autocomplete_path' => 'author/autocomplete',
    );
  }

  $form['publication']['secondaryAuthors']['check'] = array(
    '#type' => 'checkbox',
    '#title' => t('I have >30 Secondary Authors'),
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('right'),
      'title' => array('Upload a file instead'),
    ),
  );

  $form['publication']['secondaryAuthors']['file'] = array(
    '#type' => 'managed_file',
    '#title' => t('Secondary Authors file: please upload a spreadsheet with columns for last name, first name, and middle initial of each author, in any order'),
    '#upload_location' => "$file_upload_location",
    '#upload_validators' => array(
      'file_validate_extensions' => array('txt csv xlsx'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="publication[secondaryAuthors][check]"]' => array('checked' => TRUE),
      ),
    ),
    '#tree' => TRUE,
  );

  $form['publication']['secondaryAuthors']['file']['empty'] = array(
    '#default_value' => isset($values['publication']['secondaryAuthors']['file']['empty']) ? $values['publication']['secondaryAuthors']['file']['empty'] : 'NA',
  );

  $form['publication']['secondaryAuthors']['file']['columns'] = array(
    '#description' => 'Please define which columns hold the required data: Last Name, First Name.',
  );

  $column_options = array(
    'N/A',
    'First Name',
    'Last Name',
    'Middle Initial',
  );

  $form['publication']['secondaryAuthors']['file']['columns-options'] = array(
    '#type' => 'hidden',
    '#value' => $column_options,
  );

  $form['publication']['secondaryAuthors']['file']['no-header'] = array();

  return $form;
}
