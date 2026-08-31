<?php

/**
 * @file
 * Shows list of synonyms for each organism.
 */

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
  if ($reset || empty($cache) || empty($cache->data[$key])) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $rows = $default_value;
    $data = db_select(OrganismSynonym::TABLE, 't')
      ->fields('t', ['value'])
      ->condition('type_id', OrganismSynonym::CVTERM_ID)
      ->orderBy('t.value')
      //->range(0, 19)
      ->execute()
      ->fetchCol();

    foreach ($data as $synonym_name) {
      $parts = tpps_organism_parse_name($synonym_name);
      $rows[$synonym_name] = array_merge(['synonym_name' => $synonym_name], $parts);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $cache->data[$key] = $rows;
    cache_set($cid, $cache->data, $cache_bin);
  }
  $rows = $cache->data[$key] ?? $default_value;
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $header = [
    ['data' => t('Synonym Name'), 'field' => 'synonym_name'],
    ['data' => t('Genus'), 'field' => 'genus'],
    ['data' => t('Species'), 'field' => 'species'],
    [
      'data' => t('Infraspecific Rank'),
      'field' => 'infraspecific_rank',
    ],
    ['data' => t('Infraspecific Name'), 'field' => 'infraspecific_name'],
    [
      'data' => t('Authority_in_parentheses'),
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

/**
 * Parses organism name.
 *
 * @param string $input
 *   Organism name.
 *
 * @return array
 *   Returns parts of the organism name.
 */
function tpps_organism_parse_name($input) {
  // Pattern to match the full structure.
  $pattern = '/^
    ([A-Z][a-z]+)                         # 1. Genus (capitalized)
    \s+
    ([a-z-]+(?:\s*[a-z-]+)?)              # 2. Species (lowercase, may have hyphen)
    (?:
      \s+
      (?:                                 # 3. Infraspecific rank + name (optional)
        (subsp|var|f|forma|subvar|subf|proles|nothosubsp|nothovar|spp|subspecies|var|varieta|variety|form)
        \.*
        \s+
        ([a-z-]+(?:\s*[a-z-]+)?)
      )
    )?
    (?:
      \s+
      (?:                                 # 4. Authority (optional, in parentheses first)
        \(
          ([^)]+)                         # Authority inside parentheses
        \)
        \s*
      )?
      (?:                                 # 5. Authority (outside parentheses)
        ([^,]+?)                          # Authority without year
      )?
      (?:                                 # 6. Year (optional)
        ,\s*
        (\d{4})
      )?
    )?
    $/x';
  preg_match($pattern, trim($input), $matches);
  $result = [
    'genus' => $matches[1] ?? 'NULL',
    'species' => $matches[2] ?? NULL,
    'infraspecific_rank' => NULL,
    'infraspecific_name' => NULL,
    'authority_in_parentheses' => NULL,
    'authority' => NULL,
    'year' => NULL,
  ];

  // Extract infraspecific if present.
  if (isset($matches[3]) && isset($matches[4])) {
    $result['infraspecific_rank'] = $matches[3];
    $result['infraspecific_name'] = $matches[4];
  }

  // Extract authority and year.
  if (isset($matches[5])) {
    $result['authority_in_parentheses'] = trim($matches[5]);
  }

  if (isset($matches[6])) {
    $result['authority'] = trim($matches[6]);
  }

  if (isset($matches[7])) {
    $result['year'] = trim($matches[7]);
  }

  return $result;
}
