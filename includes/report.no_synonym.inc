<?php

/**
 * @file
 * Generates list of Phenotypes without Synonyms.
 */

//ini_set('display_errors', 1);
//ini_set('display_startup_errors', 1);
//error_reporting(E_ALL);
/**
 * Menu callback. Shows list of all phenotypes without synonym.
 */
function tpps_admin_no_synonym_report() {
  // Query to get Study name.
  $sub_query = db_select('chado.phenotype', 'cte')->fields('cte', []);
  $sub_query->addExpression(
    "substring(cte.uniquename, 0, strpos(cte.uniquename, '-'))",
    'study_name'
  );

  $table = 'phenotype';
  $meta = [
    // @DEBUG Shows SQL-query at page and table rows dump in watchdog.
    //'tag' => 'debug',
    // To have common style.
    'attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => [
      ['data' => t('Study'), 'field' => 'public.cte.study_name'],
      ['data' => t('Phenotype Name'), 'field' => 'chado.' . $table . '.name'],
    ],
    'main_table' => [
      'schema' => 'chado',
      'name' => $table,
      'fields' => ['name'],
    ],
    'tables' => [
      'phenotype_to_synonym' => [
        'schema' => 'chado',
        'name' => 'phenotype_to_synonym',
        'join' => [
          'type' => 'left outer',
          'on' => 'chado' . $table . '.phenotype_id '
            . '= chadophenotype_to_synonym.phenotype_id',
        ],
        'fields' => ['phenotype_synonyms_id'],
      ],
      // Common Table Expression.
      'cte' => [
        'schema' => 'public',
        'name' => 'cte',
        'query' => $sub_query,
        'fields' => ['study_name'],
        'join' => [
          'type' => 'left outer',
          'on' => 'chado' . $table . '.phenotype_id'
            . ' = publiccte.phenotype_id',
        ],
      ],
    ],
    'filter' => [
      // Exclude all names without 'TGDR' in it.
      "strpos(publiccte.study_name, 'TGDR') > 0",
      // Show only phenotypes which do not have synonyms.
      "COALESCE(chadophenotype_to_synonym.phenotype_synonyms_id, 0) = 0",
    ],
    'groupBy' => [
      // WARNING:
      // Do not add extra fields because grouping by 'study_name' won't work.
      'publiccte.study_name',
      'chadophenotype.name',
      'chadophenotype_to_synonym.phenotype_synonyms_id',
    ],
  ];

  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  if($title = variable_get('tpps_report_no_synonym_title')) {
    $page_title = '<h2>' . $title . '</h2>';
  }
  return $page_title . simple_report($meta, $filter ?? []);
}
