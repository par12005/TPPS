<?php

/**
 * @file
 * Generates list of Phenotypes without Synonyms.
 */

/**
 * Menu callback. Shows list of new phenotypes without synonym.
 */
function tpps_admin_no_synonym_new_report() {
  $table = 'chado.phenotype_to_synonym';
  $filter[] = ['name' => 'phenotype_synonyms_id', 'value' => 0];
  return simple_table_report($table, $filter ?? []);
}

/**
 * Menu callback. Shows list of all phenotypes without synonym.
 */
function tpps_admin_no_synonym_all_report() {
  $table = 'chado.phenotype';
  //$filter[] = ['name' => 'phenotype_synonyms_id', 'value' => 0];
  return simple_table_report($table, $filter ?? []);
}

