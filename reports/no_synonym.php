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

  //$table_name = 'public.users';
  //$table_name = 'users';
  $table_name = 'chado.phenotype';
  return simple_table_report($table_name);
}

