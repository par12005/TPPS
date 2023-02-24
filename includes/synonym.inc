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
