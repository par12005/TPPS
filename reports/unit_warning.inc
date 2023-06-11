<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Shows simple report of 'tpps_phenotype_unit_warning' table.
 */
function tpps_admin_unit_warning_report(array $filter = []) {
  $table = 'tpps_phenotype_unit_warning';
  $meta = [
    // @DEBUG Shows SQL-query at page and table rows dump in watchdog.
    //'tag' => 'debug',
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'refresh_time' => variable_get('tpps_report_refresh_time', 0),
    // Manually set 'primary_key' because DB table has no PK
    // but 'unique keys' index.
    'primary_key' => 'study_name',
    'select_all' => FALSE,
    // To have common style.
    'attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => [
      [
        'data' => t("Study"),
        'field' => 'public.' . $table . '.study_name',
        'theme' => 'none',
      ],
    ],
    'main_table' => [
      'schema' => 'public',
      'name' => $table,
    ],
  ];

  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  $title = variable_set(
    'tpps_report_unit_warning_title',
    'Unit Warning (list of phenotypes which unit differs from Synonym)'
  );
  if ($title) {
    $page_title = '<h2>' . $title . '</h2>';
  }
  return $page_title . simple_report_tableselect($meta, $filter ?? []);
}
