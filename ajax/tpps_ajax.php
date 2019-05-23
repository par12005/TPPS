<?php

/**
 * @file
 * Defines some ajax callback functions that TPPS uses often.
 */

/**
 * Author auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_author_autocomplete($string) {
  // TODO: load chado cvterm_ids with tripal api functions.
  $matches = array();
  $result = db_select('chado.contact', 'contact')
    ->fields('contact', array('name', 'type_id'))
    ->condition('name', db_like($string) . '%', 'LIKE')
    ->condition('type_id', '71', 'LIKE')
    ->execute();

  foreach ($result as $row) {
    $matches[$row->name] = check_plain($row->name);
  }

  drupal_json_output($matches);
}

/**
 * Organization auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_organization_autocomplete($string) {
  $matches = array();
  $result = db_select('chado.contact', 'contact')
    ->fields('contact', array('name', 'type_id'))
    ->condition('name', db_like($string) . '%', 'LIKE')
    ->condition('type_id', '72', 'LIKE')
    ->execute();

  foreach ($result as $row) {
    $matches[$row->name] = check_plain($row->name);
  }

  drupal_json_output($matches);
}

/**
 * Journal auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_journal_autocomplete($string) {
  $matches = array();
  $result = db_select('chado.pub', 'pub')
    ->fields('pub', array('series_name'))
    ->condition('series_name', db_like($string) . '%', 'LIKE')
    ->execute();

  foreach ($result as $row) {
    $matches[$row->series_name] = check_plain($row->series_name);
  }

  drupal_json_output($matches);
}

/**
 * Species auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_species_autocomplete($string) {
  $matches = array();

  $parts = explode(" ", $string);
  if (!isset($parts[1])) {
    $parts[1] = "";
  }
  // var_dump($parts);
  $result = db_select('chado.organism', 'organism')
    ->fields('organism', array('genus', 'species'))
    ->condition('genus', db_like($parts[0]) . '%', 'LIKE')
    ->condition('species', db_like($parts[1]) . '%', 'LIKE')
    ->orderBy('genus')
    ->orderBy('species')
    ->execute();

  foreach ($result as $row) {
    $matches[$row->genus . " " . $row->species] = check_plain($row->genus . " " . $row->species);
  }

  drupal_json_output($matches);
}

/**
 * Phenotype name auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_phenotype_autocomplete($string) {
  $matches = array();

  $result = db_select('chado.phenotype', 'phenotype')
    ->fields('phenotype', array('name'))
    ->condition('name', db_like($string) . '%', 'LIKE')
    ->execute();

  foreach ($result as $row) {
    $matches[$row->name] = check_plain($row->name);
  }

  drupal_json_output($matches);
}

/**
 * Phenotype attribute auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_attribute_autocomplete($string) {
  $matches = array();

  $attributes = db_select('chado.phenotype', 'p')
    ->distinct()
    ->fields('p', array('attr_id'));

  $and = db_and()
    ->condition('c.cvterm_id', $attributes, 'IN')
    ->condition('c.name', db_like($string) . '%', 'LIKE');

  $result = db_select('chado.cvterm', 'c')
    ->fields('c', array('name'))
    ->condition($and)
    ->execute();

  foreach ($result as $row) {
    $matches[$row->name] = check_plain($row->name);
  }

  drupal_json_output($matches);
}

/**
 * Phenotype units auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_units_autocomplete($string) {
  $matches = array();

  $and = db_and()
    ->condition('type_id', '2842')
    ->condition('value', db_like($string) . '%', 'LIKE');

  $result = db_select('chado.phenotypeprop', 'p')
    ->distinct()
    ->fields('p', array('value'))
    ->condition($and)
    ->execute();

  foreach ($result as $row) {
    $matches[$row->value] = check_plain($row->value);
  }

  drupal_json_output($matches);
}

/**
 * Phenotype structure auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_structure_autocomplete($string) {
  $matches = array();

  $structures = db_select('chado.phenotype', 'p')
    ->distinct()
    ->fields('p', array('observable_id'));

  $and = db_and()
    ->condition('c.cvterm_id', $structures, 'IN')
    ->condition('c.name', db_like($string) . '%', 'LIKE');

  $result = db_select('chado.cvterm', 'c')
    ->fields('c', array('name', 'definition'))
    ->condition($and)
    ->execute();

  foreach ($result as $row) {
    $matches[$row->name] = check_plain($row->name . ': ' . $row->definition);
  }

  drupal_json_output($matches);
}

/**
 * Indicate the managed_file element to be updated.
 *
 * This function is called after a no_header element is changed, triggering an
 * update of the managed_file element.
 *
 * @param array $form
 *   The form that needs to be updated.
 * @param array $form_state
 *   The state of the form that needs to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_no_header_callback(array $form, array &$form_state) {

  $parents = $form_state['triggering_element']['#parents'];
  array_pop($parents);

  $element = drupal_array_get_nested_value($form, $parents);
  return $element;
}
