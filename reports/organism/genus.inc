<?php

/**
 * @file
 * Shows list of organisms per study.
 */

/**
 * Menu callback. Shows list of organisms per study.
 */
function tpps_report_organism_genus() {
  $name = 'organism_genus_list';
  $page_title = variable_get('tpps_report_' . $name . '_title', 'Organism Genus List');

  // WARNING: Report::print() requires db_select() not db_query().
  $query = db_select('chado.organism', 'o')
    ->fields('o', ['genus'])
    ->groupBy('o.genus');
  $query->addExpression('COUNT(o.genus)', 'count');
  $query->orderBy('count', 'DESC');

  $result = $query->execute()->fetchAllAssoc('genus', PDO::FETCH_ASSOC);

  $list = [];
  foreach ($result as $item) {
    $name = check_plain($item['genus']);
    $url = 'admin/reports/tpps/organism/species/' . $name;
    $list[] = l($name, $url) . ' [' . check_plain($item['count']) . ']';
  }
  return theme('item_list', ['items' => $list]);
}
