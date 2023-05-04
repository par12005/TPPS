<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Shows simple report of 'tpps_phenotype_unit_warning' table.
 */
function tpps_admin_unit_warning_report(array $filter = []) {
  $sub_query = db_select('chado.phenotype', 'cte')->fields('cte', []);
  $sub_query->addExpression(
    "substring(cte.uniquename, 0, strpos(cte.uniquename, '-'))",
    'study_name'
  );

  $table = 'tpps_phenotype_unit_warning';
  $meta = [
    // @DEBUG Shows SQL-query at page and table rows dump in watchdog.
    // 'tag' => 'debug',
    //'formatter' => 'tpps_admin_no_synonym_formatter',
    'items_per_page' => variable_get('tpps_report_items_per_page', 25),
    'refresh_time' => variable_get('tpps_report_refresh_time', 0),
    'primary_key' => 'phenotype_id',
    'select_all' => FALSE,
    'attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => [
      ['data' => t('Phenotype Id'), 'field' => $table . '.phenotype_id'],
      ['data' => t("Study"), 'field' => 'study_name'],
      ['data' => t('Phenotype Name'), 'field' => 'phenotype.name'],
    ],
    'tables' => [
      $table => [
        'schema' => 'public',

      ],
      'phenotype' => [
        'schema' => 'chado',
        'join' => [
          'type' => 'left outer',
          'on' => $table . '.phenotype_id = phenotype.phenotype_id',
        ],
        'fields' => ['name'],
      ],
      // Common Table Expression.
      'cte' => [
        'schema' => 'public',
        'query' => $sub_query,
        'fields' => ['study_name'],
        'join' => [
          // @TODO Use addJoin() instead of leftJoin().
          'type' => 'left outer',
          'on' => 'cte.phenotype_id = tpps_phenotype_unit_warning.phenotype_id',
        ],
      ],
    ],
    'filter' => [
      // Exclude all names without 'TGDR' in it.
      "strpos(cte.study_name, 'TGDR') > 0",
    ],
  ];

  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  if($title = variable_get('tpps_report_no_synonym_title')) {
    // @TODO Use theme templates.
    $page_title = '<h2>' . $title . '</h2>';
  }
  return $page_title . simple_report_tableselect($meta, $filter ?? []);
}
