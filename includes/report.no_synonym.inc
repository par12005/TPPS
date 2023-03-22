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

  $sql = "
SELECT
  study_name, count(phenotype.phenotype_id) AS count_studies
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
    ) as cte
    ON cte.phenotype_id = phenotype.phenotype_id

GROUP BY
  cte.study_name,
  phenotype_to_synonym.phenotype_synonyms_id
HAVING
  (strpos(cte.study_name, 'TGDR') > 0)
  AND
    (COALESCE(phenotype_to_synonym.phenotype_synonyms_id, 0) = 0)
ORDER BY study_name DESC
";

  $meta = [
    'header' => [
      ['data' => t("Study"), 'field' => 'study_name'],
      ['data' => t("Count"), 'field' => 'count_studies'],
    ],
    'sql' => $sql,
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
