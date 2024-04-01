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
    '#autocomplete_path' => 'tpps/autocomplete/author',
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('right'),
      'title' => array('First Author of the publication'),
    ),
  );
  // if(isset($values['primaryAuthor']) && $values['primaryAuthor'] != "") {
  //   $form['primaryAuthor']['#value'] = $values['primaryAuthor'];
  // }

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
    '#title' => '<div class="fieldset-title">' . t('PUBLICATION INFORMATION:')
    . '</div>'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
  );

  tpps_secondary_authors($form, $values, $form_state);

  $form['publication']['status'] = [
    '#type' => 'select',
    '#title' => t('Publication Status: *'),
    '#options' => array(
      0 => t('- Select -'),
      'In Preparation or Submitted' => t('In Preparation or Submitted'),
      'In Press' => t('In Press'),
      'Published' => t('Published'),
    ),
    '#ajax' => [
      'callback' => 'tpps_pub_status',
      'wrapper' => 'pubyear',
    ],
  ];

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

  return $form;
}

/**
 * Creates fields describing the species in the publication.
 *
 * The same form used for TPPS and TPPSc forms.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The form_state of the form to be populated.
 */
function tpps_organism(array &$form, array &$form_state) {
  $field = [
    '#type' => 'fieldset',
    'name' => [
      '#type' => 'textfield',
      '#title' => "Species !num: *",
      '#autocomplete_path' => 'tpps/autocomplete/species',
      '#attributes' => [
        'data-toggle' => ['tooltip'],
        'data-placement' => ['right'],
        'title' => ['If your species is not in the autocomplete list, '
        . 'don\'t worry about it! We will create a new organism entry '
        . 'in the database for you.'],
      ],
      '#description' => 'Example: '
        . '<a href"#" class="tpps-suggestion">Arabidopsis thaliana</a>.',
    ],
    'is_tree' => [
      '#type' => 'select',
      '#title' => t('This species is a tree:'),
      '#options' => [
        '1' => t('Yes'),
        '0' => t('No'),
      ],
    ],
  ];

  tpps_dynamic_list($form, $form_state, 'organism', $field, [
    'label' => 'Organism',
    'default' => 1,
    'substitute_fields' => [
      ['name', '#title'],
    ],
  ]);
}
// [/VS]

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

/**
 * Creates fields describing the species in the publication.
 *
 * This is code from TPPSc module which worked fine in 7.x-1.x-dev but
 * after movement to TGDR module Add/Remove buttons do not work anymore.
 * Left to have ability to compare and reuse code.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The form_state of the form to be populated.
 *
 * @return array
 *   The populated form.
 *
 *
 * @TODO Doublecheck and remove. Currently function not used.
 */
function tppsc_organism(array &$form, array &$form_state) {
  // TPPSc Form.
  $org_number = tpps_get_ajax_value($form_state, array('organism', 'number'), 1);

  $form['organism'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => '<div class="fieldset-title">' . t('ORGANISM INFORMATION:')
    . '</div>',
    '#description' => t('Please provide the name(s) of the species included in this publication.'),
    '#collapsible' => TRUE,
    '#prefix' => '<div id="organism-wrapper">',
    '#suffix' => '</div>',
  );

  $form['organism']['add'] = array(
    '#type' => 'button',
    '#title' => t('Add Organism'),
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
    '#title' => t('Remove Organism'),
    '#button_type' => 'button',
    '#value' => t('Remove Organism'),
    '#name' => t('Remove Organism'),
    '#ajax' => array(
      'wrapper' => 'organism-wrapper',
      'callback' => 'tpps_organism_callback',
    ),
  );

  //$publication_doi = tpps_get_ajax_value($form_state, ['publication', 'publication_doi']);
  //$form['organism']['number'] = array(
  //  '#type' => 'hidden',
  //  '#value' => !empty($publication_doi) ? $org_number : NULL,
  //);

  for ($i = 1; $i <= $org_number; $i++) {

    $form['organism']["$i"]['name'] = array(
      '#type' => 'textfield',
      '#title' => t("Species @num: *", array('@num' => $i)),
      '#autocomplete_path' => "tpps/autocomplete/species",
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('left'),
        'title' => array('If your species is not in the autocomplete list, don\'t worry about it! We will create a new organism entry in the database for you.'),
      ),
      // [VS]
      '#description' => 'Example: '
        . '<a href"#" class="tpps-suggestion">Arabidopsis thaliana</a>.',
      // [/VS]
    );
    $org = tpps_get_ajax_value($form_state, array('organism', $i, 'name'));
    $form['organism'][$i]['name']['#attributes']['value'] = $org ?? NULL;

    // [VS] #8669py203.
    $form['organism']["$i"]['is_tree'] =
    [
      '#type' => 'select',
      '#title' => t('This species is a tree:'),
      '#options' => [
        '1' => t('Yes'),
        '0' => t('No'),
        '-1' => t("I don't know"),
      ],
      '#default_value' => $form_state['saved_values'][TPPS_PAGE_1]['is_tree'] ?? '1',
    ];
    // [/VS].
  }
}

/**
 * Creates a list of DOI examples.
 *
 * This list will be used in field's description.
 * Each DOI could be inserted into field by mouse click.
 *
 * @return string
 *   Returns HTML string with list of DOI examples.
 */
function tpps_page_1_get_doi_examples() {
  $doi_suggestion_list = [
    // Fake.
    '10.1111/dryad.111',
    // Real but used. No species.
    '10.5061/dryad.91mk9',
    // Not used, no species.
    '10.21267/IN.2016.6.2294',
    // Raal, not used, with species.
    '10.5061/dryad.vk43j',
  ];
  foreach ($doi_suggestion_list as $doi_suggestion) {
    // @todo Use l().
    $list[] = '<a href"#" class="tpps-suggestion">'
      . $doi_suggestion . '</a>';
  }
  return 'Examples: <br />' . implode(', ', $list);
}

/**
 * Returns (or not) asterisk based on value of 'Publication Status' field.
 *
 * Result is statically cached.
 *
 * @param array $form_state
 *   Drupal Form API Status array.
 *
 * @return string
 *   Returns asterisk if certain value of 'Publication Status' was set.
 *   Returns empty string otherwise.
 */
function tpps_page_1_required_by_status(array $form_state) {
  $result = &drupal_static(__FUNCTION__);
  if (!isset($result)) {
    $publication_status = tpps_get_ajax_value(
      $form_state, ['publication', 'status'], NULL
    );
    $result = ($publication_status == 'Published' ? ' *' : '');
  }
  return $result;
}
