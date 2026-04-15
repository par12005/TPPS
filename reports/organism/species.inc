<?php

/**
 * @file
 * Shows list of organism's species.
 */

/**
 * Menu callback. Shows list of organisms's species.
 */
function tpps_report_organism_species() {
  if (empty($genus = arg(5))) {
    drupal_goto('admin/reports/tpps/organism/genus');
  }

  $name = 'organism_species';
  $page_title = variable_get('tpps_report_' . $name . '_title', 'Organism Species List');

  $query = db_select('chado.organism', 'o')
    ->fields('o', ['species'])
    ->groupBy('o.species')
    ->condition('o.genus', $genus);
  $query->addExpression('COUNT(o.species)', 'count');
  $query->orderBy('count', 'DESC');

  $result = $query->execute()->fetchAllAssoc('species', PDO::FETCH_ASSOC);

  $list = [];
  foreach ($result as $item) {
    $name = check_plain($item['species']);
    $url = 'api/organism/' . $genus . '/' . $name;
    $list[] = l($name, $url) . ' [' . check_plain($item['count']) . ']';
  }
  return theme('item_list', [
    'title' => l($genus, $genus),
    'items' => $list,
  ]);
}
