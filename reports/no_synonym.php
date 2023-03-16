<?php

/**
 * @file
 * Generates list of Phenotypes without Synonyms.
 */

/**
 * Menu callback. Shows list of all phenotypes without synonym.
 */
function tpps_admin_no_synonym_report() {
  //return simple_table_report('chado.phenotype_to_synonym');

  $table = 'phenotype';
      //['data' => t("Name"), 'field' => $table . '.name', 'op' => 'count'],
      //['data' => t("Attribute Id"), 'field' => $table . '.attr_id', 'op' => 'count'],
  $meta = [
    'header' => [
      ['data' => t("Phenotype Id"), 'field' => $table . '.phenotype_id', 'sort' => 'DESC'],
      ['data' => t("Unique Name"), 'field' => $table . '.uniquename'],
      //['data' => t("Name"), 'field' => $table . '.name'],
      ['data' => t("Name"), 'field' => $table . '.name', 'op' => 'count'],
      ['data' => t("Attribute Id"), 'field' => $table . '.attr_id'],
      ['data' => t("Value"), 'field' => $table . '.value'],
      ['data' => t("CValue"), 'field' => $table . '.cvalue_id'],
      ['data' => t("Assay Id"), 'field' => $table . '.assay_id'],
    ],
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

