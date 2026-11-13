<?php

/**
 * @file
 * TPPS Organism Name parsing tool.
 */

/**
 * TPPS Organism Name parsing tool.
 */
function tpps_organism_name_form(array $form, array &$form_state) {
  $organism_name = $form_state['input']['organism_name'] ?? '';

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Suggestions.
  $suggestions = [];
  $params = ['attributes' => ['class' => ['tpps-suggestion']]];
  $example_list = [
    'Juniperus communis x juniperus nana',
    'Juniperus communis x Juniperus nana',
    'Eucalyptus  globulus ssp. bicostata',
    'Salix aurita ×',
    'Salix caprea subsp. var. caprea',
    'Rubroshorea macrantha',
    'Podocarpus aff. degeneri MtLoftyBG',
    'Populus x tomentiglandulosa',
    'Eucalyptus dunnii (D) x Eucalyptus grandis (G)] x [Eucalyptus urophylla (U) x Eucalyptus globulus (GL)]',
    'Isaurica',
    'Salix x smithiana',
  ];
  foreach ($example_list as $value) {
    $suggestions[] = l($value, '', $params);
  }
  tpps_add_css_js('suggestion', $form);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Organism name.
  $form['organism_name'] = [
    '#type' => 'textfield',
    '#title' => t('Organism name'),
    '#default_value' => $organism_name,
    '#description' => theme('item_list', [
      'title' => t('Organism name examples'),
      'items' => $suggestions,
    ]),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  TppsForm::autofocus($form, ['organism_name']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('main', $form);

  if ($organism_name) {
    $organism = new Organism($organism_name);

    $metadata = [
      'Name' => $organism->name,
      'Id' => $organism->id ?? '-',
      'Is Hybrid' =>  $organism->isHybrid ? 'Yes' : 'No',
      'Is Graft' => $organism->isGraft ? 'Yes' : 'No',
    ];
    $form['meta_data'] = [
      '#markup' => TppsArray::print($metadata, t('Organism Meta Data')),
    ];
    $rows[] = [
      $organism->abbreviation,
      $organism->genus,
      $organism->species,
      NULL,
      $organism->infraspecificName,
      InfraspecificRank::getCvtermId($organism->infraspecificRank),
    ];
    $form['parsed_name'] = [
      '#markup' => '<h3>' . (string) t('Name parsing result')
      . '</h3>' . (string) theme('table', [
        'header' => array_keys(Organism::getEmptyParsedName()),
          'rows' => $organism->parsedName,
      ]),
    ];
  }

  // Show buttons.
  $form['actions'] = $form['actions'] ?? ['#type' => 'container'];
  $action_list = ['parse', 'reset'];
  foreach ($action_list as $key) {
    $form['actions'][$key] = [
      '#type' => 'submit',
      '#name' => $key,
      '#value' => t(ucfirst($key)),
    ];
  }
  return $form;
}


/**
 * Form submitter for 'tpps_organism_name_form' form.
 */
function tpps_organism_name_form_submit(array &$form, array &$form_state) {
  if ($form_state['triggering_element']['#name'] == 'parse') {
    $form_state['rebuild'] = TRUE;
  }
}
