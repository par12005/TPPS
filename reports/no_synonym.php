<?php

/**
 * @file
 * Generates list of Phenotypes without Synonyms.
 */

/**
 * Menu callback. Shows list of all phenotypes without synonym.
 */
function tpps_admin_no_synonym_report() {
  //return simple_table_report('chado.phenotype_to_synonym');

  $table = 'chado.phenotype';
  //$filter[] = ['name' => 'phenotype_synonyms_id', 'value' => 0];
  return simple_table_report($table, $filter ?? []);
}

