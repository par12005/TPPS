<?php

/**
 * @file
 * Shows list of the organism which are missing NCBI Taxonomy Id.
 */

/**
 * Menu callback.
 */
function tpps_report_organism_ncbi_missing() {
  $table = 'chado.organism';
  $table_alias = str_replace('.', '', $table);

  $table2 = 'public.' . NcbiTaxonomy::TABLE;
  $table2_alias = str_replace('.', '', $table2);

  $query = db_select($table, $table_alias);
  $query->leftJoin(
    $table2,
    $table2_alias,
    $table_alias . '.organism_id = ' . $table2_alias . '.organism_id'
  );
  // Organism Id must be from $table (not $table2) to see empty values from
  // $table2.
  $query->fields($table_alias, ['organism_id', 'genus', 'species']);
  $query->isNull($table2_alias . '.taxonomy_id');

  $header = [
    [
      'data' => t('Organism Id'),
      'field' => $table_alias . '.organism_id',
    ],
    [
      'data' => t('Organism Genus'),
      // WARNING: $table2 (not $table).
      'field' => $table_alias . '.genus',
    ],
    [
      'data' => t('Organism Species'),
      // WARNING: $table2 (not $table).
      'field' => $table_alias . '.species',
    ],
  ];
  return Report::print([
    'name' => 'organism_ncbi_missing',
    'header' => $header,
    'query' => $query,
  ]);
}
