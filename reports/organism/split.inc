<?php

/**
 * @file
 * Shows list of synonyms for each organism.
 *
 * How to test?
 * $input = 'Amygdalus gessleriana G.Kirchn., 1864';
 * module_load_include('inc', 'tpps', 'reports/organism/split');
 * dpm(var_export(tpps_organism_parse_name($input), 1), 'output');
 */

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
function tpps_report_organism_split_formatter(string $name, $value, array &$row) {
  $formatted = is_array($value) ? $value : check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'id':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/tpps/organism/split',
          ['query' => ['id' => $formatted]]
        );
      }
      break;
  }
  return $formatted;
}

/**
 * Menu callback. Shows how synonym names are splitted into parts.
 */
function tpps_report_organism_split() {
  $default_value = [];
  // When TRUE organismprop_id will be appended to synonym's name.
  $add_id_to_name = FALSE;

  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';

  $cache = cache_get($cid, $cache_bin);
  $key = $cid;
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  if (
    $reset
    || empty($cache)
    || empty($cache->data[$key])
    || (!empty($_GET['id']) && empty($cache->data[$key][$_GET['id']]))
  ) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $rows = $default_value;
    $data = db_select(OrganismSynonym::TABLE, 't')
      ->fields('t', ['organismprop_id', 'value'])
      ->condition('type_id', OrganismSynonym::CVTERM_ID)
      ->orderBy('t.value')
      //->range(0, 90)
      //->condition('organismprop_id', $_GET['id'])
      ->execute()
      ->fetchAllKeyed();

    foreach ($data as $id => $synonym_name) {
      if (strpos($synonym_name, '?') === 0) {
        continue;
      }
      $parts = Organism::parseName($synonym_name);
      $rows[$id] = array_merge(
        [
          'id' => $id,
          'synonym_name' => $synonym_name,
        ],
        $parts
      );
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $cache->data[$key] = $rows;
    cache_set($cid, $cache->data, $cache_bin);
  }
  $rows = $cache->data[$key] ?? $default_value;
  if (!empty($id = $_GET['id'] ?? NULL) && !empty($value = $rows[$id] ?? NULL)) {
    $rows = [$id => $value];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $header = [
    ['data' => t('Id'), 'field' => 'id'],
    ['data' => t('Synonym Name'), 'field' => 'synonym_name'],
    ['data' => t('Genus'), 'field' => 'genus'],
    ['data' => t('Species'), 'field' => 'species'],
    [
      'data' => t('Infraspecific Rank'),
      'field' => 'infraspecific_rank',
    ],
    ['data' => t('Infraspecific Name'), 'field' => 'infraspecific_name'],
    [
      'data' => t('(Authority)'),
      'field' => 'authority_in_parentheses',
    ],
    ['data' => t('Authority'), 'field' => 'authority'],
    ['data' => t('Year'), 'field' => 'year'],
  ];

  return Report::print([
    'name' => 'organism_split',
    'header' => $header,
    'rows' => $rows,
    'pager' => 'none',
  ], $reset);
}
