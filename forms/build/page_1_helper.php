<?php

/**
 * @file
 */

/**
 *
 */
function user_info(&$form, $values) {

  $form['primaryAuthor'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Author: *'),
    '#autocomplete_path' => 'author/autocomplete',
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('left'),
      'title' => array('First Author of the publication'),
    ),
  );

  $form['organization'] = array(
    '#type' => 'textfield',
    '#title' => t('Organization: *'),
    '#autocomplete_path' => 'organization/autocomplete',
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('left'),
      'title' => array('Organization of the Primary Author'),
    ),
  );

  return $form;
}

/**
 *
 */
function publication(&$form, $values, $form_state) {

  $form['publication'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Publication Information:</div>'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
  );

  secondary_authors($form, $values, $form_state);

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
      'callback' => 'page_1_pub_status',
      'wrapper' => 'pubyear',
    ),
  );

  year($form, $values, $form_state);

  $form['publication']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title of Publication: *'),
  );

  $form['publication']['abstract'] = array(
    '#type' => 'textarea',
    '#title' => t('Abstract: *'),
  );

  $form['publication']['journal'] = array(
    '#type' => 'textfield',
    '#title' => t('Journal: *'),
    '#autocomplete_path' => 'journal/autocomplete',
  );

  return $form;
}

/**
 *
 */
function organism(&$form, $values) {

  $form['organism'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('<div class="fieldset-title">Organism information:</div>'),
    '#description' => t('Up to 5 organisms per submission.'),
    '#collapsible' => TRUE,
  );

  $form['organism']['add'] = array(
    '#type' => 'button',
    '#title' => t('Add Organism'),
    '#button_type' => 'button',
    '#value' => t('Add Organism'),
  );

  $form['organism']['remove'] = array(
    '#type' => 'button',
    '#title' => t('Remove Organism'),
    '#button_type' => 'button',
    '#value' => t('Remove Organism'),
  );

  $form['organism']['number'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($values['organism']['number']) ? $values['organism']['number'] : '1',
  );

  for ($i = 1; $i <= 5; $i++) {

    $form['organism']["$i"] = array(
      '#type' => 'textfield',
      '#title' => t("Species $i: *"),
      '#autocomplete_path' => "species/autocomplete",
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('left'),
        'title' => array('If your species is not in the autocomplete list, don\'t worry about it! We will create a new organism entry in the database for you.'),
      ),
    );
  }

  return $form;
}

/**
 *
 */
function year(&$form, $values, $form_state) {

  if (isset($form_state['values']['publication']['status']) and $form_state['values']['publication']['status'] != '0') {
    $pub_status = $form_state['values']['publication']['status'];
  }
  elseif (isset($form_state['saved_values'][PAGE_1]['publication']['status']) and $form_state['saved_values'][PAGE_1]['publication']['status'] != '0') {
    $pub_status = $form_state['saved_values'][PAGE_1]['publication']['status'];
  }

  if (isset($pub_status) and $pub_status != 'Published') {
    $yearArr = array(0 => '- Select -');
    for ($i = 2015; $i <= 2018; $i++) {
      $yearArr[$i] = "$i";
    }
  }
  elseif (isset($pub_status)) {
    $yearArr = array(0 => '- Select -');
    for ($i = 1990; $i <= 2018; $i++) {
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
 *
 */
function secondary_authors(&$form, $values, $form_state) {

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
      '#title' => t("Secondary Author $i: *"),
      '#autocomplete_path' => 'author/autocomplete',
    );
  }

  $form['publication']['secondaryAuthors']['check'] = array(
    '#type' => 'checkbox',
    '#title' => t('I have >30 Secondary Authors'),
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('left'),
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
