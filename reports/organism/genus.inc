<?php

/**
 * @file
 * Shows list of organisms per study.
 */

/**
 * Menu callback. Shows list of organisms per study.
 */
function tpps_report_organism_genus() {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $query = db_select('chado.organism', 'o')
    ->fields('o', ['genus'])
    ->groupBy('o.genus');
  $query->addExpression('COUNT(o.genus)', 'count');
  $result = $query->execute()->fetchAllAssoc('genus', PDO::FETCH_ASSOC);

  $rows = [];
  foreach ($result as $item) {
    $name = check_plain($item['genus']);
    $rows[] = [
      'genus' => l($name, 'admin/reports/tpps/organism/species/' . $name),
      'count' => check_plain($item['count']),
    ];
  }
  $header = [
    ['data' => t('Genus'), 'field' => 'genus'],
    ['data' => t('Count'), 'field' => 'count', 'sort' => 'desc'],
  ];
  return Report::print([
    'name' => 'organism_genus_list',
    'header' => $header,
    'rows' => $rows,
    'pager' => 'none',
  ], $reset);
}
