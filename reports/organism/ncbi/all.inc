<?php

/**
 * @file
 * Shows list of the NCBI Taxonomy Ids.
 */

/**
 * Menu callback. Shows list of the all organisms and their NCBI Taxonomy Ids.
 */
function tpps_report_organism_ncbi_all() {
  $page_title = variable_get('tpps_report_organism_ncbi_all_title');

  $table = 'chado.organism';
  $table_alias = str_replace('.', '', $table);
  $query = db_select($table, $table_alias);

  $table2 = 'public.' . Organism::TABLE_NCBI_TAXONOMY;
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

  return '<h2>' . $page_title . '</h2>' . Report::print([
    'debug_mode' => FALSE,
    'css_path' => TPPS_MODULE_PATH . TPPS_CSS_PATH,
    'page_title' => $page_title,
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => $header,
    'query' => $query,
  ]);

}
