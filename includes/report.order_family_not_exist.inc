<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Shows simple report to identify if a term (Order/Family)
 * is not already present in database.
 */
function tpps_admin_order_family_not_exist_report(array $filter = []) {
  $list = db_select('chado.organismprop', 'op')
    ->fields('op', ['organism_id'])
    ->condition('type_id', [9, 10], 'IN')
    ->distinct()
    ->execute()
    ->fetchCol();

  $table = 'chado.organism';
  $meta = [
    // @DEBUG Shows SQL-query at page and table rows dump in watchdog.
    //'tag' => 'debug',
    'attributes' => ['class' => ['view' , 'tpps_table']],
    'tables' => [
      $table => ['fields'  => ['organism_id', 'genus', 'species', 'type_id']],
    ],
    'filter' => [
      [
        'name' => 'organism_id',
        'value' => $list,
        'operator' => 'NOT IN',
      ],
    ],
    'items_per_page' => variable_get('tpps_report_items_per_page', 25),
    'refresh_time' => variable_get('tpps_report_refresh_time', 0),
  ];

  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  if($title = variable_get('tpps_report_order_family_not_exist_title')) {
    // @TODO Use theme templates.
    $page_title = '<h2>' . $title . '</h2>';
  }
  return $page_title . simple_report($meta, $filter ?? []);
}
