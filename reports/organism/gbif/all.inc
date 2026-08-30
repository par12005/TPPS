<?php

/**
 * @file
 * Shows list of the GBIF Taxonomy Ids.
 */

/**
 * Menu callback. Shows list of the all organisms and their NCBI Taxonomy Ids.
 */
function tpps_report_organism_gbif_all() {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $table = 'chado.organism';
  $table_alias = str_replace('.', '', $table);
  $query = db_select($table, $table_alias);

  $table2 = 'public.' . GbifSpeciesApi::TABLE;
  $table2_alias = str_replace('.', '', $table2);
  $query->leftJoin(
    $table2,
    $table2_alias,
    $table_alias . '.organism_id = ' . $table2_alias . '.organism_id'
  );
  // Organism Id must be from $table (not $table2) to see empty values from
  // $table2.
  $query->fields($table_alias, ['organism_id']);
  $query->fields($table2_alias, ['organism_name', 'taxonomy_id']);

  $header = [
    [
      'data' => t('Organism Id'),
      'field' => $table_alias . '.organism_id',
    ],
    [
      'data' => t('Organism Name'),
      // WARNING: $table2 (not $table).
      'field' => $table2_alias . '.organism_name',
    ],
    [
      'data' => t('Taxonomy Id'),
      // WARNING: $table2 (not $table).
      'field' => $table2_alias . '.taxonomy_id',
    ],
  ];

  return Report::print([
    'name' => 'organism_gbif_all',
    'header' => $header,
    'query' => $query,
    'pager' => 'none',
  ], $reset);

}
