<?php

/**
 * @file
 * Shows list of synonyms for each organism.
 */

/**
 * Menu callback. Shows list of all organisms.
 *
 * Reset: 52 msec,
 * Cached: 1 msec.
 *
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 */
function tpps_report_organism_synonym_list($reset = FALSE) {
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $key = $cid;
  if ($reset || empty($cache) || empty($cache->data[$key])) {
    $name = 'organism_synonym_list';
    $page_title = variable_get(
      'tpps_report_' . $name . '_title',
      'Organism Synonym List'
    );
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $synonym_cvterm_id = Organism::SYNONYM_CVTERM_ID;
    $query = db_select('chado.organismprop', 'op')
      ->fields('op', ['organismprop_id'])
      ->condition('op.type_id', $synonym_cvterm_id);
    $query->leftJoin('chado.organism', 'o', 'op.organism_id = o.organism_id');
    $query->addField('o', 'organism_id');
    $query->addField('o', 'genus');
    $query->addField('o', 'species');
    $query->addField('o', 'infraspecific_name');
    $query->addField('op', 'value', 'synonym_list');
    $query->orderBy('o.organism_id');
    $query->orderBy('op.value');
    $result = $query->execute()->fetchAll();

    // Group by organism_id to have a list of synonyms.
    $rows = [];
    $prev_oid = NULL;
    foreach ($result as $item) {
      $oid = $item->organism_id;
      if ($oid != $prev_oid) {
        $rows[$oid] = (array) $item;
        $rows[$oid]['synonym_list'] = [$item->synonym_list];
      }
      else {
        $rows[$oid]['synonym_list'][] = $item->synonym_list;
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
      ['data' => t('Synonym List'), 'field' => 'synonym_list'],
    ];

    $content = Report::printSortableTable([
      'report_name' => $name,
      'header' => $header,
      'rows' => $rows,
      'formatter' => 'tpps_report_' . $name . '_formatter',
    ]);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $cache->data[$key] = $content;
    cache_set($cid, $cache->data, $cache_bin);
  }
  $content = $cache->data[$key] ?? '';
  return $content;
}

/**
 * Values formatter for tpps_report_organism_synonym_list().
 *
 * @param string $name
 *   Required. Field name.
 *   Machine table field name (column name) without table name and dot.
 * @param mixed $value
 *   Field value.
 * @param array $row
 *   Data of the whole table row with hidden fields.
 *   This data could be used to format other fields. For example,
 *   hidden field 'uid' or 'entity_id' could be used to build a link to
 *   user profile page.
 *
 * @return mixed
 *   Returns formatted value.
 *   Empty value will not be formatted and returned as is.
 */
function tpps_report_organism_synonym_list_formatter(string $name, $value, array &$row) {
  $formatted = is_array($value) ? $value : check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'organism_id':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/db/chado.organism',
          ['query' => ['chado.organism.organism_id' => $formatted]]
        );
      }
      break;

    case 'synonym_list':
      $formatted = theme('item_list', ['items' => $formatted, 'type' => 'ol']);
      break;
  }
  return $formatted;
}
