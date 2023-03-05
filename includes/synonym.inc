<?php

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
 * Gets Phenotype Synonym's data.
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
