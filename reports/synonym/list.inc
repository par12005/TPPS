<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Shows simple report of 'tpps_phenotype_unit_warning' table.
 */
function tpps_report_synonym_list() {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $table = 'chado.phenotype_synonyms';
  $table_alias = str_replace('.', '', $table);
  $query = db_select($table, $table_alias)
    ->fields($table_alias, []);
  $header = [
    ['data' => '#', 'field' => $table_alias . '.phenotype_synonyms_id'],
    ['data' => t('Synonym Name'), 'field' => $table_alias . '.synonym'],
    ['data' => t('Structure'), 'field' => $table_alias . '.structure'],
    ['data' => t('Structure Id'), 'field' => $table_alias . '.structure_id'],
    ['data' => t('Attribute Id'), 'field' => $table_alias . '.attribute_id'],
    ['data' => t('CVTerm1 Id'), 'field' => $table_alias . '.cvterm_1_id'],
    ['data' => t('CVTerm2 Id'), 'field' => $table_alias . '.cvterm_2_id'],
    [
      'data' => t('Is common phenotype?'),
      'field' => $table_alias . '.is_common_phenotype',
    ],
    ['data' => t('Unit Group'), 'field' => $table_alias . '.unit_group'],
    [
      'data' => t('Measurement Type'),
      'field' => $table_alias . '.measurement_type',
    ],
    [
      'data' => t('Is environmental phenotype?'),
      'field' => $table_alias . '.is_environmental_phenotype',
    ],
    [
      'data' => t('Phenotype Group'),
      'field' => $table_alias . '.phenotype_group',
    ],
  ];
  return Report::print([
    'name' => 'synonym_list',
    'header' => $header,
    'query' => $query,
  ], $reset);
}
