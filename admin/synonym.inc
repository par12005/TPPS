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
  $form['synonym']['unit_group'] = [
    '#type' => 'textfield',
    '#title' => 'Unit Group',
    '#default_value' => $values['unit_group'] ?? '',
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
  $id = tpps_synonym_add($form_state['values']['synonym']);
  $message = t('New synonym was created. Id: @id.Data: <pre>@data</pre>',
    [
      '@id' => $id,
      '@data' => print_r($form_state['values']['synonym'], 1),
    ]
  );
  drupal_set_message($message);
  watchdog('tpps', $message);
  cache_clear_all('*', TPPS_CACHE_BIN ?? 'cache', TRUE);
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
