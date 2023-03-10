<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */


// @TODO Remove selected items from db table.
function tpps_admin_unit_warning_report() {
  $table_name = 'tpps_phenotype_unit_warning';
  return simple_table_report($table_name);
}

