<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Shows simple report of 'tpps_phenotype_unit_warning' table.
 */
function tpps_admin_unit_warning_report(array $filter = []) {
  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  $title = 'Phenotypes which units differs from Synonym';
  drupal_set_title($title);
  $table = 'tpps_phenotype_unit_warning';
  $meta = [
    // @DEBUG Shows SQL-query at page and table rows dump in watchdog.
    // 'tag' => 'debug',
    'attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => [
      ['data' => t('Phenotype Id'), 'field' => $table . '.phenotype_id'],
      ['data' => t('Phenotype Name'), 'field' => 'phenotype.name'],
    ],
    'tables' => [
      $table => [],
      'chado.phenotype' => [
        'join' => [
          'type' => 'left outer',
          'on' => $table . '.phenotype_id = phenotype.phenotype_id',
        ],
        'fields' => ['name'],
      ]
    ],
    'formatter' => 'tpps_admin_unit_warning_report_formatter',
    'items_per_page' => variable_get('tpps_report_items_per_page', 25),
    'refresh_time' => variable_get('tpps_report_refresh_time', 0),
    'primary_key' => 'phenotype_id',
    'select_all' => FALSE,
  ];
  return '<h2>' . $title . '</h2>' . simple_report_tableselect($meta, $filter);
}
