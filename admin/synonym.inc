<?php

/**
 * @file
 * Add new Phenotype Synonym form.
 */

/**
 * Creates form to add new phenotype synonym.
 *
 * @todo Minor. Add autocomplete for form fields.
 */
function tpps_synonym_add_form(array $form, array &$form_state) {
  $values = $form_state['values']['synonym'] ?? [];

  $form['links'] = [
    '#markup' => theme('item_list', [
      'title' => t('Useful links'),
      'items' => [
        l(t('List of all Phenotype Synonyms'), 'admin/reports/tpps/synonym/list'),
        l(t('List of phenotypes without Synonyms'), 'admin/reports/tpps/synonym/missing'),
        l(t('List of phenotypes without synonym'), 'admin/reports/tpps/synonym/unit_warning'),
      ],
    ]),
  ];
  $form['synonym'] = [
    '#type' => 'container',
    '#tree' => TRUE,
  ];
  $form['synonym']['synonym'] = [
    '#type' => 'textfield',
    '#title' => 'Phenotype Synonym Name',
    '#default_value' => $values['synonym'] ?? '',
    '#required' => TRUE,
  ];
  $form['synonym']['structure'] = [
    '#type' => 'textfield',
    '#title' => 'Structure Name',
    '#default_value' => $values['structure'] ?? '',
    '#required' => TRUE,
  ];
  $form['synonym']['structure_id'] = [
    '#type' => 'textfield',
    '#title' => 'Structure Id',
    '#attributes' => ['maxlength' => 10, 'size' => 10],
    '#element_validate' => ['element_validate_integer_positive'],
    '#default_value' => $values['structure_id'] ?? '',
    '#required' => TRUE,
  ];
  $form['synonym']['attribute_id'] = [
    '#type' => 'textfield',
    '#title' => 'Attribute Id',
    '#attributes' => ['maxlength' => 10, 'size' => 10],
    '#element_validate' => ['element_validate_integer_positive'],
    '#default_value' => $values['attribute_id'] ?? '',
    '#required' => TRUE,
  ];
  $form['synonym']['cvterm_1_id'] = [
    '#type' => 'textfield',
    '#title' => 'CV Term 1 Id',
    '#attributes' => ['maxlength' => 10, 'size' => 10],
    '#element_validate' => ['element_validate_integer_positive'],
    '#default_value' => $values['cvterm_1_id'] ?? '',
  ];
  $form['synonym']['cvterm_2_id'] = [
    '#type' => 'textfield',
    '#title' => 'CV Term 2 Id',
    '#attributes' => ['maxlength' => 10, 'size' => 10],
    '#element_validate' => ['element_validate_integer_positive'],
    '#default_value' => $values['cvterm_2_id'] ?? '',
  ];
  $form['synonym']['is_common_phenotype'] = [
    '#type' => 'checkbox',
    '#title' => 'Is Common Phenotype?',
    '#default_value' => $values['is_common_phenotype'] ?? 0,
  ];

  // Note: not id but group name.
  $unit_list = db_select('chado.phenotype_unit_group', 't')
    ->fields('t', ['unit_group'])
    ->orderBy('unit_group')
    ->execute()
    ->fetchCol('unit_group');

  $form['synonym']['unit_group'] = [
    '#type' => 'select',
    '#title' => 'Unit Group',
    '#options' => array_combine($unit_list, $unit_list),
    '#default_value' => $values['unit_group'] ?? NULL,
    '#required' => TRUE,
  ];

  $form['synonym']['measurement_type'] = [
    '#type' => 'select',
    '#options' => [
      'environment' => t('Environment'),
      'phenotype' => t('Phenotype'),
      'Genotype' => t('Genotype'),
    ],
    '#title' => 'Measurement Type',
    '#default_value' => $values['measurement_type'] ?? '',
    '#required' => TRUE,
  ];
  $form['synonym']['is_environmental_phenotype'] = [
    '#type' => 'checkbox',
    '#title' => 'Is Environmental Phenotype?',
    '#default_value' => $values['is_environmental_phenotype'] ?? 0,
  ];
  $form['synonym']['phenotype_group'] = [
    '#type' => 'textfield',
    '#title' => 'Phenotype Group',
    '#default_value' => $values['phenotype_group'] ?? '',
  ];

  // Actions.
  $form['actions']['add'] = [
    '#type' => 'submit',
    '#value' => t('Add Synonym'),
  ];
  $form['#submit'][] = 'tpps_synonym_add_form_submit';
  return $form;
}

/**
 * Implements hook_form_submit().
 */
function tpps_synonym_add_form_submit($form, &$form_state) {
  // Convert strings to integers.
  foreach ($form_state['complete form']['synonym'] as $name => $item) {
    $condition = (
      !empty($item['#element_validate'])
      && in_array('element_validate_integer_positive', $item['#element_validate'])
    );
    if ($condition) {
      $form_state['values']['synonym'][$name]
        = (!empty($form_state['values']['synonym'][$name]))
        ? ((int) $form_state['values']['synonym'][$name]) : NULL;
    }
  }
  $id = PhenotypeSynonym::add($form_state['values']['synonym']);
  $message = t('New synonym was created. Id: @id.Data: <pre>@data</pre>',
    [
      '@id' => $id,
      '@data' => print_r($form_state['values']['synonym'], 1),
    ]
  );
  drupal_set_message($message);
  watchdog('tpps', $message);

  // Clear only cached list of synonyms.
  tpps_cache_clear_all(['tpps_synonym_list']);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/**
 * Creates form to rebuild synonym/unit lists.
 */
function tpps_synonym_list_rebuild_form(array $form, array &$form_state) {
  // Actions.
  $form['actions']['rebuld'] = [
    '#type' => 'submit',
    '#value' => t('Rebuld Synonym/Unit lists'),
  ];
  $form['#submit'][] = 'tpps_synonym_list_rebuild_form_submit';
  return $form;
}

/**
 * Implements hook_form_submit().
 */
function tpps_synonym_list_rebuild_form_submit($form, &$form_state) {
  tpps_unit_get_list('all', [], TRUE);
  $message = t('Synonym/Unit lists was rebuilt.');
  drupal_set_message($message);
}
