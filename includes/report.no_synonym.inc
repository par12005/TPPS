<?php

/**
 * @file
 * Generates list of Phenotypes without Synonyms.
 */

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
    //    'tag' => 'debug',
    'attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => [
      ['data' => t("Study"), 'field' => 'study_name'],
      ['data' => t("Phenotype Name"), 'field' => $table . '.name'],
    ],
    'tables' => [
      'chado.' . $table => [
        'fields' => ['name'],
      ],
      'chado.phenotype_to_synonym' => [
        'join' => [
          'type' => 'left outer',
          'on' => 'phenotype.phenotype_id = phenotype_to_synonym.phenotype_id',
        ],
        'fields' => ['phenotype_synonyms_id'],
      ],
      // Common Table Expression.
      'cte' => [
        'query' => $sub_query,
        'fields' => ['study_name'],
        'join' => [
          // @TODO Use addJoin() instead of leftJoin().
          'type' => 'left outer',
          'on' => 'cte.phenotype_id = phenotype.phenotype_id',
        ],
      ],
    ],
    'filter' => [
      // Exclude all names without 'TGDR' in it.
      "strpos(cte.study_name, 'TGDR') > 0",

      // Show only phenotypes which do not have synonyms.
      "COALESCE(phenotype_to_synonym.phenotype_synonyms_id, 0) = 0",

      // @DEBUG This phenotype has a synomym and must be excluded.
      //'phenotype.phenotype_id = 2531888', // Empty table.
      //'phenotype.phenotype_id = 2815843', // Must be shown.
    ],
    'groupBy' => [
      // WARNING:
      // Do not add extra fields because grouping by 'study_name' won't work.
      'study_name',
      'phenotype.name',
      'phenotype_to_synonym.phenotype_synonyms_id',
    ],
    'formatter' => 'tpps_admin_no_synonym_formatter',
  ];
  
  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  if($title = variable_get('tpps_report_no_synonym_title')) {
    // @TODO Use theme templates.
    $page_title = '<h2>' . $title . '</h2>';
  }
  return $page_title . simple_report($meta, $filter ?? []);
}

/**
 * Simple Report Formatter for 'tpps_admin_no_synonym' report.
 *
 * @param string $name
 *   Field name.
 * @param mixed $value
 *   Field value.
 * @param array $row
 *   Whole table's row.
 *
 * @return string
 *   Returns formatted field value.
 */
function tpps_admin_no_synonym_formatter(string $name, $value, array $row) {
  $formatted = check_plain($value);
  if (empty($name)) {
    return $formatted;
  }
  switch ($name) {
    case 'study_name':
      if (!empty($value)) {
        $path = 'tpps-admin-panel/' . $value;
        $formatted = l(check_plain($value), $path);
      }
      break;
  }
  return $formatted;
}
