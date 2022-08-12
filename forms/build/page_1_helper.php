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
  // dpm($values);
  $form['primaryAuthor'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Author: *'),
    '#autocomplete_path' => 'tpps/autocomplete/author',
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('right'),
      'title' => array('First Author of the publication'),
    ),
  );
  if(isset($values['primaryAuthor']) && $values['primaryAuthor'] != "") {
    $form['primaryAuthor']['#value'] = $values['primaryAuthor'];
  }

  $form['organization'] = array(
    '#type' => 'textfield',
    '#title' => t('Organization: *'),
    '#autocomplete_path' => 'tpps/autocomplete/organization',
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
  if(isset($values['publication']['journal']) && $values['publication']['journal'] != "") {
    $form['publication']['journal']['#value'] = $values['publication']['journal'];
  }  

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

  $field = array(
    '#type' => 'fieldset',
    '#title' => "Species !num",
    'name' => array(
      '#type' => 'textfield',
      '#autocomplete_path' => 'tpps/autocomplete/species',
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('If your species is not in the autocomplete list, don\'t worry about it! We will create a new organism entry in the database for you.'),
      ),
    ),
    'is_tree' => array(
      '#type' => 'checkbox',
      '#title' => t('This species is a tree.'),
      '#default_value' => 1,
    ),
  );

  tpps_dynamic_list($form, $form_state, 'organism', $field, array(
    'label' => 'Organism',
    'default' => 1,
    'substitute_fields' => array(
      '#title',
    ),
  ));

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
    '#description' => t('If your publication has not been published yet, please choose the expected year of publication.'),
    '#prefix' => '<div id="pubyear">',
    '#suffix' => '</div>',
  );
  // if(isset($values['publication']['year'])) {
  //   $form['publication']['year']['#value'] = $values['publication']['year'];
  // }

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
  $field = array(
    '#type' => 'textfield',
    '#title' => "Secondary Author !num",
    '#autocomplete_path' => 'tpps/autocomplete/author',
  );

  tpps_dynamic_list($form, $form_state, 'secondaryAuthors', $field, array(
    'label' => 'Secondary Author',
    'callback' => 'tpps_authors_callback',
    'substitute_fields' => array(
      '#title',
    ),
    'parents' => array(
      'publication',
    ),
  ));

  $form['publication']['secondaryAuthors']['#title'] = "<div class=\"fieldset-title\" style=\"font-size:.8em\">Secondary Author Information</div>";

  return $form;
}
