<?php

/**
 * @file
 * Shows list of synonyms for each organism.
 */

/**
 * Menu callback. Shows list of all organisms.
 */
function tpps_report_organism_synonym_list() {
  $name = 'organism_synonym_list';
  $page_title = variable_get('tpps_report_' . $name . '_title', "Organism Synonym List");

  $synonym_cvterm_id = Organism::SYNONYM_CVTERM_ID;

  $query = db_select('chado.organismprop', 'op')
    ->fields('op', ['organismprop_id'])
    ->condition('op.type_id', $synonym_cvterm_id);
  $query->leftJoin('chado.organism', 'o', 'op.organism_id = o.organism_id');
  $query->addField('o', 'organism_id');
  $query->addField('o', 'genus');
  $query->addField('o', 'species');
  $query->addField('o', 'infraspecific_name');
  $query->addField('op', 'value', 'synonym_name');
  $query->orderBy('o.organism_id');
  $query->orderBy('op.value');
  $result = $query->execute()->fetchAll();

  // @todo Add caching.
  $rows = [];
  $prev_oid = NULL;
  foreach ($result as $item) {
    $oid = $item->organism_id;
    if ($oid != $prev_oid) {
      if (!empty($rows[$prev_oid]['synonym_name'])) {
        $rows[$prev_oid]['synonym_name'] = theme('item_list', [
          'items' => $rows[$prev_oid]['synonym_name'],
          'type' => 'ol',
        ]);
      }
      $rows[$oid] = (array) $item;
      $rows[$oid]['synonym_name'] = [$item->synonym_name];
    }
    else {
      $rows[$oid]['synonym_name'][] = $item->synonym_name;
    }
    $prev_oid = $oid;
  }

  // Header.
  $header = [
    ['data' => t('Id'), 'field' => 'organismprop_id'],
    ['data' => t('Organism Id'), 'field' => 'organism_id'],
    ['data' => t('Genus'), 'field' => 'genus'],
    ['data' => t('Species'), 'field' => 'species'],
    ['data' => t('Infraspecific Name'), 'field' => 'infraspecific_name'],
    ['data' => t('Synonyms'), 'field' => 'synonym_name'],
  ];

  return Report::printSortableTable([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No data found.'),
  ]);
}
