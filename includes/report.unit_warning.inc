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
    //'tag' => 'debug',
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'refresh_time' => variable_get('tpps_report_refresh_time', 0),
    // Manually set 'primary_key' because DB table has no PK.
    'primary_key' => 'phenotype_id',
    'select_all' => FALSE,
    // To have common style.
    'attributes' => ['class' => ['view' , 'tpps_table']],

    'header' => [
      ['data' => t('Phenotype Id'), 'field' => 'public.' . $table . '.phenotype_id'],
      [
        'data' => t("Study"),
        'field' => 'public.cte.study_name',
        'theme' => 'none',
      ],
      ['data' => t('Phenotype Name'), 'field' => 'chado.phenotype.name'],
    ],
    'main_table' => [
      'schema' => 'public',
      'name' => $table,
    ],
    'tables' => [
      'chadophenotype' => [
        'schema' => 'chado',
        'name' => 'phenotype',
        'join' => [
          'type' => 'left outer',
          'on' => 'public' . $table . '.phenotype_id = chadophenotype.phenotype_id',
        ],
        'fields' => ['name'],
      ],
      // Common Table Expression.
      'publiccte' => [
        'schema' => 'public',
        'name' => 'cte',
        'query' => $sub_query,
        // Specify list of fields to avoid automatic generation.
        'fields' => ['study_name'],
        'join' => [
          'type' => 'left outer',
          'on' => 'publiccte.phenotype_id = publictpps_phenotype_unit_warning.phenotype_id',
        ],
      ],
    ],
    'filter' => [
      // Exclude all names without 'TGDR' in it.
      "strpos(publiccte.study_name, 'TGDR') > 0",
    ],
  ];

  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  if ($title = variable_get('tpps_report_no_synonym_title')) {
    $page_title = '<h2>' . $title . '</h2>';
  }
  return $page_title . simple_report_tableselect($meta, $filter ?? []);
}
