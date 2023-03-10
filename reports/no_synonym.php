<?php

/**
 * @file
 * Generates list of Phenotypes without Synonyms.
 */

function tpps_admin_no_synonym_report() {
  $table_name = 'tpps_phenotype_unit_warning';
  return simple_table_report($table_name);
}

