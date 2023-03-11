<?php

/**
 * @file
 * Generates list of Phenotypes without Synonyms.
 */

/**
 * Menu callback. Shows list of phenotypes without synonym.
 */
function tpps_admin_no_synonym_report() {
  // @TODO Build sql to get list of phenotypes without synonyms.

  $table_name = 'tpps_phenotype_unit_warning';
  return simple_table_report($table_name);
}

