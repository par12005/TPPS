<?php


//ini_set('display_errors', 1); ini_set('display_startup_errors', 1); error_reporting(E_ALL);



/**
 * @file
 * Generates list of Phenotypes without Synonyms.
 */

/**
 * Menu callback. Shows list of all phenotypes without synonym.
 */
function tpps_admin_no_synonym_report() {
  //return simple_table_report('chado.phenotype_to_synonym');
        //"
        //  SELECT
        //    phenotype_id,
        //    substring(uniquename, 0, strpos(uniquename, '-')) AS study_name
        //  FROM chado.phenotype",
  $sub_query = db_select('chado.phenotype', 'cte')
    ->fields('cte');
  $sub_query->addExpression(
    "substring(cte.uniquename, 0, strpos(cte.uniquename, '-'))",
    'study_name'
  );



  $table = 'phenotype';
  $meta = [
    'header' => [
      ['data' => t("#"), 'field' => 'countStudies'],
      ['data' => t("Study"), 'field' => 'study_name'],
    ],
    'tables' => [
      'chado.' . $table => [
        'fields' => [
          'phenotype_id',
          'uniquename'
        ],
        'full_table_name' => 'chado.phenotype',
        'schema' => 'chado',
      ],
      // Common Table Expression.
      'cte' => [
        'join' => [
          // @TODO Use addJoin() instead of leftJoin().
          'type' => 'left outer',
          'on' => 'cte.phenotype_id = phenotype.phenotype_id',
        ],
        'query' => $sub_query,
        'fields' => ['study_name'],
      ],
      'chado.phenotype_to_synonym' => [
        'join' => [
          'type' => 'left outer',
          'on' => 'phenotype.phenotype_id = phenotype_to_synonym.phenotype_id',
        ],
        'fields' => ['phenotype_synonyms_id'],
      ],
    ],
    'addExpression' => [
    //  'study_name' => "substring(uniquename, 0, strpos(uniquename, '-'))",
      'countStudies' => "count(phenotype.uniquename)",
    ],
    'filter' => [
      // Exclude all names without 'TGDR' in it.
      "strpos(phenotype.uniquename, 'TGDR') > 0",

      // Show only phenotypes which do not have synonyms.
      "COALESCE(phenotype_to_synonym.phenotype_synonyms_id, 0) = 0",

      // @DEBUG This phenotype has a synomym and must be excluded.
      //'phenotype.phenotype_id = 2531888',
    ],
    'groupBy' => [
      //"phenotype.phenotype_id",
      'study_name',
      'phenotype.uniquename',
      'phenotype_to_synonym.phenotype_synonyms_id',
      //"substring(uniquename, 0, strpos(uniquename, '-'))",
    ],
    'formatter' => 'tpps_admin_no_synonym_formatter',
  ];

  if (count($_GET)) {
    foreach ($_GET as $name => $value) {
      if (!in_array($name, ['q', 'page', 'sort', 'order', 'count'])) {
        $filter[] = ['name' => $name, 'value' => $value];
      }
    }
  }
  return simple_report($meta, $filter ?? []);
}

/**
 * Simple Report Formatter for 'tpps_admin_no_synonym' report.
 *
 * @param string $name
 * @param mixed $value
 * @param array $row
 * @access public
 *
 * @return void
 */
function tpps_admin_no_synonym_formatter(string $name, $value, array $row) {
  $formatted = check_plain($value);
  if (empty($name)) {
    return $formatted;
  }
  switch ($name) {
    // @TODO Update to use for Count Report.
    case 'phenotype_id':
      if (!empty($value)) {
        $path = 'wholeplant/' . check_plain($value);
        $path = 'tpps-admin-panel/TGDR465';
        $formatted = l(check_plain($value), $path);
      }
      break;

    case 'name':
      if (!empty($value)) {
        $path = url('admin/reports/no-synonym/name/' . $formatted, [
          'absolute' => TRUE,
        ]);
        $formatted = l(check_plain($value), $path);
      }
      break;
  }
  return $formatted;
}

/**
 * Menu callback. Shows specific phenotype report which has no synonym.
 */
function tpps_admin_no_synonym_name_report($phenotype_name = '') {
  if (empty($phenotype_name)) {
    return;
  }

  $filter[] = ['name' => 'name', 'value' => $phenotype_name];
  $table = 'phenotype';
  $meta = [
    'tables' => [
      'chado.' . $table => [
        'fields' => [],
        'full_table_name' => 'chado.phenotype',
        'schema' => 'chado',
      ],
      'chado.phenotype_to_synonym' => [
        'join' => [
          'type' => 'leftJoin',
          'on' => 'phenotype.phenotype_id = phenotype_to_synonym.phenotype_id',
        ],
        'fields' => ['phenotype_synonyms_id'],
      ],
    ],
    'formatter' => 'tpps_admin_no_synonym_formatter',
    'pager' => 'both',
  ];
  // @TODO Check how to use 'op' => '>'.
  //$filter[] = ['name' => 'phenotype_synonyms_id', 'value' => 0, 'operator' => '='];
  if (count($_GET)) {
    foreach ($_GET as $name => $value) {
      if (!in_array($name, ['q', 'page', 'sort', 'order', 'count'])) {
        $filter[] = ['name' => $name, 'value' => $value];
      }
    }
  }
  return simple_report($meta, $filter ?? []);
}

/**
 * Updated DB query with tag 'tpps_admin_no_synonym_report'.
 *
 * WARNING:
 * This function uses format hook_query_TAG_alter() but will not be called
 * because Drupal expects hooks to be in *.module file.
 *
 * @param QueryAlterableInterface $query
 */
function tpps_query_tpps_admin_no_synonym_report_alter(QueryAlterableInterface $query) {

  //dpm(debug_backtrace());

  //$query->groupBy('phenotype.uniquename');
//$query->groupBy('study_name');
//
  //$query->havingCondition('study_name', 'TGDR737');
  //$query->isNull('phenotype_to_synonym.phenotype_id');
  //$query->distinct();
}




/*
 * Both works.
============================
WITH
============================

WITH
cte1 as (
  SELECT
    phenotype_id,
    substring(uniquename, 0, strpos(uniquename, '-')) AS study_name
  FROM chado.phenotype
)
SELECT
 study_name,
  count(cte1.phenotype_id)
FROM cte1
LEFT OUTER JOIN
  chado.phenotype_to_synonym phenotype_to_synonym
  ON cte1.phenotype_id = phenotype_to_synonym.phenotype_id

GROUP BY
  cte1.study_name,
  phenotype_to_synonym.phenotype_synonyms_id
HAVING
  (strpos(cte1.study_name, 'TGDR') > 0)
  AND
    (COALESCE(phenotype_to_synonym.phenotype_synonyms_id, 0) = 0)
ORDER BY cte1.study_name DESC
LIMIT 25;



============================
JOIN
============================

SELECT
 study_name,
  count(cte1.phenotype_id)
FROM (
  SELECT
    phenotype_id,
    substring(uniquename, 0, strpos(uniquename, '-')) AS study_name
  FROM chado.phenotype
  ) as cte1

  LEFT OUTER JOIN
    chado.phenotype_to_synonym phenotype_to_synonym
    ON cte1.phenotype_id = phenotype_to_synonym.phenotype_id

GROUP BY
  cte1.study_name,
  phenotype_to_synonym.phenotype_synonyms_id
HAVING
  (strpos(cte1.study_name, 'TGDR') > 0)
  AND
    (COALESCE(phenotype_to_synonym.phenotype_synonyms_id, 0) = 0)
ORDER BY cte1.study_name DESC
LIMIT 25;


=========================
2 JOINS
=========================

SELECT
  study_name,
  count(cte1.phenotype_id)
FROM
  chado.phenotype phenotype
  LEFT OUTER JOIN
    chado.phenotype_to_synonym phenotype_to_synonym
    ON phenotype.phenotype_id = phenotype_to_synonym.phenotype_id
  LEFT OUTER JOIN
    (
    SELECT
      phenotype_id,
      substring(uniquename, 0, strpos(uniquename, '-')) AS study_name
    FROM chado.phenotype
    ) as cte1
    ON cte1.phenotype_id = phenotype.phenotype_id

GROUP BY
  cte1.study_name,
  phenotype_to_synonym.phenotype_synonyms_id
HAVING
  (strpos(cte1.study_name, 'TGDR') > 0)
  AND
    (COALESCE(phenotype_to_synonym.phenotype_synonyms_id, 0) = 0)
ORDER BY cte1.study_name DESC
LIMIT 25;

===================================
  From page:

SELECT
  cte.study_name AS study_name,
  phenotype_to_synonym.phenotype_synonyms_id AS phenotype_synonyms_id
FROM
  chado.phenotype phenotype
  left outer JOIN (
    SELECT t.phenotype_id AS phenotype_id, substring(t.uniquename, 0, strpos(t.uniquename, '-')) AS study_name
    FROM chado.phenotype t) cte
    ON cte.phenotype_id = phenotype.phenotype_id
  left outer JOIN chado.phenotype_to_synonym phenotype_to_synonym
    ON phenotype.phenotype_id = phenotype_to_synonym.phenotype_id
GROUP BY
  phenotype_to_synonym.phenotype_synonyms_id, phenotype.uniquename, cte.study_name, study_name
HAVING
  (strpos(uniquename, 'TGDR') > 0)
  AND (COALESCE(phenotype_to_synonym.phenotype_synonyms_id, 0) = 0)
ORDER BY cte.study_name ASC
LIMIT 25;


 */
