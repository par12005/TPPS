<?php

/**
 * @file
 * Shows list of the organism which are missing NCBI Taxonomy Id.
 */

/**
 * Menu callback.
 */
function tpps_report_organism_ncbi_missing() {
  $page_title = variable_get('tpps_report_organism_ncbi_missing_title');

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

  return '<h2>' . $page_title . '</h2>' . Report::print([
    'debug_mode' => FALSE,
    'css_path' => TPPS_MODULE_PATH . TPPS_CSS_PATH,
    'page_title' => $page_title,
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => [
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
    ],
    'query' => $query,
  ]);
}
