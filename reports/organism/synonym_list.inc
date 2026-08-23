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
  $name = 'organism_synonym_list';

  $rows = [];
  $prev_oid = NULL;
  foreach (OrganismSynonym::get() as $item) {
    $oid = $item['organism_id'];
    // Group by organism_id to have a list of synonyms.
    if ($oid != $prev_oid) {
      $rows[$oid] = (array) $item;
      $rows[$oid]['synonym_list'] = [$item['synonym_name']];
    }
    else {
      $rows[$oid]['synonym_list'][] = $item['synonym_name'];
    }
    unset($rows[$oid]['synonym_name']);
    $prev_oid = $oid;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $header = [
    ['data' => t('Id'), 'field' => 'organismprop_id'],
    ['data' => t('Organism Id'), 'field' => 'organism_id'],
    ['data' => t('Genus'), 'field' => 'genus'],
    ['data' => t('Species'), 'field' => 'species'],
    ['data' => t('Infraspecific Name'), 'field' => 'infraspecific_name'],
    ['data' => t('Synonym List'), 'field' => 'synonym_list'],
  ];
  return Report::printTable([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
    'formatter' => 'tpps_report_' . $name . '_formatter',
    'pager' => 'none',
  ]);
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
