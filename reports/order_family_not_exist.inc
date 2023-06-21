<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Report to find term (Order/Family) not present in DB.
 */
function tpps_admin_order_family_not_exist_report() {
  $table = 'chado.organism';
  $table_alias = str_replace('.', '', $table);
  $meta = [
    'table' => $table,
    'debug_mode' => FALSE,
    'css_path' => drupal_get_path('module', 'tpps') . TPPS_CSS_PATH,
    'page_title' => variable_get('tpps_report_order_family_not_exist_title'),
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => [
      [
        'data' => t('Organism Id'),
        // Note: $table_alias could be skipped here.
        'field' => $table_alias . '.organism_id',
      ],
      [
        'data' => t('Genus'),
        'field' => $table_alias . '.genus',
      ],
      [
        'data' => t('Species'),
        'field' => $table_alias . '.species',
      ],
      [
        'data' => t('Type Id'),
        'field' => $table_alias . '.type_id',
      ],
    ],
    'base_query' => '',
  ];
  // Ids to exclude.
  $list = db_select('chado.organismprop', 'op')
    ->fields('op', ['organism_id'])
    ->condition('type_id', [9, 10], 'IN')
    ->distinct()
    ->execute()
    ->fetchCol();
  // Build base query.
  $meta['base_query'] = db_select($table, $table_alias)
    ->fields($table_alias, ['organism_id', 'genus', 'species', 'type_id'])
    ->condition($table_alias . '.organism_id', $list, 'NOT IN');
  return tpps_report($meta);
}
