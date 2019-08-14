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
  $matches = array();
  $string = preg_replace('/\\\\/', '\\\\\\\\', $string);

  $results = chado_select_record('contact', array('name'), array(
    'name' => $string,
    'type_id' => array(
      'name' => 'Person',
      'cv_id' => array(
        'name' => 'tripal_contact',
      ),
    ),
  ), array(
    'regex_columns' => array('name'),
  ));

  foreach ($results as $row) {
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
  $string = preg_replace('/\\\\/', '\\\\\\\\', $string);

  $results = chado_select_record('contact', array('name'), array(
    'name' => $string,
    'type_id' => array(
      'name' => 'Organization',
      'cv_id' => array(
        'name' => 'tripal_contact',
      ),
    ),
  ), array(
    'regex_columns' => array('name'),
  ));

  foreach ($results as $row) {
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
  $string = preg_replace('/\\\\/', '\\\\\\\\', $string);

  $results = chado_select_record('pub', array('series_name'), array(
    'series_name' => $string,
  ), array(
    'regex_columns' => array('series_name'),
  ));

  foreach ($results as $row) {
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
  $string = preg_replace('/\\\\/', '\\\\\\\\', $string);

  $parts = explode(" ", $string);
  if (!isset($parts[1])) {
    $parts[1] = ".";
  }

  $results = chado_select_record('organism', array('genus', 'species'), array(
    'genus' => array(
      'data' => $parts[0],
      'op' => '~*',
    ),
    'species' => array(
      'data' => $parts[1],
      'op' => '~*',
    ),
  ));

  foreach ($results as $row) {
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
  $string = preg_replace('/\\\\/', '\\\\\\\\', $string);

  $results = chado_select_record('phenotype', array('name'), array(
    'name' => array(
      'data' => $string,
      'op' => '~*',
    ),
  ));

  foreach ($results as $row) {
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
  $string = preg_replace('/\\\\/', '\\\\\\\\', $string);
  $attributes = array();
  $attr_results = chado_select_record('phenotype', array('distinct attr_id'), array());

  foreach ($attr_results as $result) {
    $attributes[] = $result->attr_id;
  }

  $results = chado_select_record('cvterm', array('name'), array(
    'name' => array(
      'data' => $string,
      'op' => '~*',
    ),
    'cvterm_id' => $attributes,
  ));

  foreach ($results as $row) {
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
  $string = preg_replace('/\\\\/', '\\\\\\\\', $string);

  $results = chado_select_record('phenotypeprop', array('value'), array(
    'value' => array(
      'data' => $string,
      'op' => '~*',
    ),
    'type_id' => array(
      'name' => 'unit',
      'cv_id' => array(
        'name' => 'uo',
      ),
    ),
  ));

  foreach ($results as $row) {
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
  $string = preg_replace('/\\\\/', '\\\\\\\\', $string);
  $structures = array();
  $struct_results = chado_select_record('phenotype', array('distinct observable_id'), array());

  foreach ($struct_results as $result) {
    $structures[] = $result->observable_id;
  }

  $results = chado_select_record('cvterm', array('name'), array(
    'name' => array(
      'data' => $string,
      'op' => '~*',
    ),
    'cvterm_id' => $structures,
  ));

  foreach ($results as $row) {
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
