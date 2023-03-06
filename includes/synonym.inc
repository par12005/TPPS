<?php

/**
 * @file
 *
 * Helper functions related to Phenotype Synonym.
 *
 * Phenotype Synonym is a predefined set of Phenotypes attributes which could
 * be used to make it easier to add new Phenotypes and speed-up form submission.
 */

/**
 * Gets list of Phenotype Synonyms.
 *
 * @return array
 *   Returns associative array where key is synonym_id and value is name.
 */
function tpps_synonym_get_list() {
  $matches = array();
  $results = chado_select_record(
    'phenotype_synonyms',
    array('phenotype_synonyms_id', 'synonym'),
    array('synonym' => array('data' => 0, 'op' => '>'))
  );
  foreach ($results as $row) {
    $matches[$row->phenotype_synonyms_id] = check_plain($row->synonym);
  }
  return $matches;
}

/**
 * Gets Synonym's data.
 *
 * To get full Phenotype data use tpps_synonym_get_phenotype();
 *
 * @param int $synonym_id
 *   Phenotype Synonym Id.
 *
 * @return object
 *   Returns all the Synonym's data.
 *   Returns FALSE if synonym wasn't found.
 */
function tpps_synonym_get($synonym_id) {
  $results = chado_select_record(
    'phenotype_synonyms',
    array('*'),
    array('phenotype_synonyms_id' => $synonym_id)
  );
  if (!empty($results)) {
    // We need only one row.
    return reset($results);
  }
  return FALSE;
}

/**
 * Gets all the Phenotype data by Synonym.
 *
 * @param int $synonym_id
 *   Phenotype Synonym Id.
 *
 * @return array
 *   Returns Penotype's data related to Synonym.
 *   Returns FALSE if synonym wasn't found.
 */
function tpps_synonym_get_phenotype($synonym_id) {
  $synonym = tpps_synonym_get($synonym_id);
  foreach ($synonym as $key => $value) {
    $skip_keys_list = array(
      'phenotype_synonyms_id',
      'synonym',
      // This field already has correct value and not linked to 'cvterm' table.
      'structure',
      // I don't know what for this item.
      // @TODO Ask Emily about it.
      'is_common_phenotype'
    );
    if (!empty($value) && !in_array($key, $skip_keys_list)) {
      // @TODO Seems we need only 'name' from 'cvterm' table so multiple
      // requests could be replaced with a set of JOINs with aliases
      // which mapped to real values.
      $synonym->$key = tpps_synonym_get_cvterm($value);
    }
  }
  return $synonym;
}

/**
 * Gets 'CVTerm' by it's Id.
 *
 * @param int $cvterm_id
 *   CVTerm Id.
 *
 * @return object
 *    Returns CVTerm.
 *
 * @TODO Minor. Move this function to better file/module.
 */
function tpps_synonym_get_cvterm($cvterm_id) {
  return db_select('chado.cvterm', 't')
    ->fields('t', array())
    ->condition('t.cvterm_id', $cvterm_id)
    ->execute()
    ->fetchObject();
}
