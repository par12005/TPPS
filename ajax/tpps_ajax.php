<?php

/**
 * @file
 * Defines some ajax callback functions that TPPS uses often.
 */

/**
 * Call the correct autocomplete function based on the type provided.
 *
 * @param string $type
 *   The type of autocomplete function needed.
 * @param string $string
 *   The string to be autocompleted.
 */
function tpps_autocomplete($type, $string = "") {
  $function = "tpps_{$type}_autocomplete";
  if (function_exists($function)) {
    $string = preg_replace('/\\\\/', '\\\\\\\\', $string);
    $function($string);
  }
}

/**
 * Author auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_author_autocomplete($string) {
  $matches = array();

  $results = chado_select_record('contact', array('name'), array(
    'name' => $string,
    'type_id' => tpps_load_cvterm('person')->cvterm_id,
  ), array(
    'regex_columns' => array('name'),
  ));

  foreach ($results as $row) {
    $matches[$row->name] = check_plain($row->name);
  }

  drupal_json_output($matches);
}

/**
 * Project title auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_project_title_autocomplete($string) {
  $matches = array();

  $query = db_select('chado.plusgeno_view', 'p')
    ->fields('p', array('title'))
    ->condition('title', $string, '~*')
    ->execute();

  while (($result = $query->fetchObject())) {
    $matches[$result->title] = check_plain($result->title);
  }

  drupal_json_output($matches);
}

/**
 * Project accession number auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_project_accession_autocomplete($string) {
  $matches = array();

  $query = db_select('chado.plusgeno_view', 'p')
    ->fields('p', array('accession'))
    ->condition('accession', $string, '~*')
    ->execute();

  while (($result = $query->fetchObject())) {
    $matches[$result->accession] = check_plain($result->accession);
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

  if (empty($matches)) {
    $matches = tpps_ncbi_species_autocomplete($string);
  }

  drupal_json_output($matches);
}

/**
 * NCBI species auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_ncbi_species_autocomplete($string) {
  $matches = array();

  $taxons = tpps_ncbi_get_taxon_id("$string*", TRUE);
  $taxons = json_decode(json_encode($taxons))->Id;

  $fetch = new EFetch('taxonomy');
  $fetch->addParam('id', implode(',', $taxons));
  $response = $fetch->get()->xml();
  $species = json_decode(json_encode($response))->Taxon;

  foreach ($species as $info) {
    $name = $info->ScientificName;
    if (!empty($name) and $info->Rank == 'species') {
      $matches[$name] = "$name (autocomplete from NCBI)";
    }
  }
  return $matches;
}

/**
 * Phenotype name auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_phenotype_autocomplete($string) {
  $matches = array();

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
 * Phenotype ontology name auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_phenotype_ontology_autocomplete($string) {
  $matches = array();

  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.cvterm', 'cvt', 'cvt.cvterm_id = p.attr_id');
  $query->join('chado.cv', 'cv', 'cv.cv_id = cvt.cv_id');
  $query->fields('cv', array('name'));
  $query->condition('cv.name', $string, '~*');
  $query = $query->execute();

  while (($result = $query->fetchObject())) {
    $matches[$result->name] = check_plain($result->name);
  }

  drupal_json_output($matches);
}

/**
 * Genotype name auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_genotype_autocomplete($string) {
  $matches = array();

  $query = db_select('chado.tpps_search_genotype_name', 'g')
    ->fields('g', array('name'))
    ->condition('g.name', $string, '~*')
    ->execute();

  while (($result = $query->fetchObject())) {
    $matches[$result->name] = check_plain($result->name);
  }

  drupal_json_output($matches);
}

/**
 * Genotype marker name auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_genotype_marker_autocomplete($string) {
  $matches = array();

  $query = db_select('chado.tpps_search_genotype_marker', 'g')
    ->fields('g', array('name'))
    ->condition('g.name', $string, '~*')
    ->execute();

  while (($result = $query->fetchObject())) {
    $matches[$result->name] = check_plain($result->name);
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
 * Phenotype unit auto-complete matching.
 *
 * Used for 'Custom Unit' field to avoid duplicates.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_unit_autocomplete($string) {
  $matches = array_map('check_plain',
    db_select('chado.phenotype_units', 'cpu')
      ->fields('cpu', ['unit_name'])
      ->condition('unit_name', '%' . db_like($string) . '%', 'LIKE')
      ->execute()->fetchCol()
  );
  drupal_json_output(array_combine($matches, $matches));
}

/**
 * Phenotype structure auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_structure_autocomplete($string) {
  $matches = array();
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

  if (empty($results)) {
    $results = chado_select_record('cvterm', array('name'), array(
      'name' => array(
        'data' => $string,
        'op' => '~*',
      ),
    ));
  }

  foreach ($results as $row) {
    $matches[$row->name] = check_plain($row->name);
    if (!empty($row->definition)) {
      $matches[$row->name] = check_plain($row->name . ': ' . $row->definition);
    }
  }

  drupal_json_output($matches);
}

/**
 * User account auto-complete matching.
 *
 * @param string $string
 *   The string the user has already entered into the text field.
 */
function tpps_user_autocomplete($string) {
  $matches = array();
  $or = db_or()
    ->condition('mail', $string, '~*')
    ->condition('name', $string, '~*');
  $result = db_select('users')
    ->fields('users', array('uid', 'name'))
    ->condition($or)
    ->execute();
  foreach ($result as $user) {
    $user = user_load($user->uid);
    $matches["$user->mail"] = check_plain($user->name);
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
