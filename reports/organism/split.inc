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
      if (0 && !empty($parts['genus'])) {
        continue;
      }
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
  $input = trim($input);

  // Handle cases where there's no species (just genus).
  if (preg_match('/^([A-Z][a-z]+)(?:\s+[A-Z][a-z]+)?$/', $input, $genusOnly)) {
    return [
      'genus' => $genusOnly[1],
      'species' => NULL,
      'infraspecific_rank' => NULL,
      'infraspecific_name' => NULL,
      'authority_in_parentheses' => NULL,
      'authority' => NULL,
      'year' => NULL,
    ];
  }

  // Pattern to match the full structure - more permissive for authors.
  $pattern = '/^
    ([A-Z][a-z]+)                         # 1. Genus
    \s+
    ([a-z-]+(?:\s*[a-z-]+)?)              # 2. Species
    (?:
      \s+
      (?:                                 # 3. Infraspecific rank + name (optional)
        (subsp|subspecies|var|variety|f|forma|subvar|subf|proles|nothosubsp|nothovar|spp)
        \.*
        \s+
        ([a-z-]+(?:\s*[a-z-]+)?)
      )
    )?
    (?:
      \s+
      (?:                                 # 4. Authority in parentheses (optional)
        \(
          ([^)]+)                         # Authority inside parentheses
        \)
        \s*
      )?
      (?:                                 # 5. Authority outside parentheses
        ([^,]+?)                          # Authority without year (non-greedy)
        (?:\s*,\s*[a-z. ]+)?              # Allow "nom. illeg." etc.
      )?
      (?:                                 # 6. Year (optional)
        ,\s*
        (\d{4})
      )?
    )?
    $/x';

  preg_match($pattern, $input, $matches);

  $result = [
    'genus' => $matches[1] ?? null,
    'species' => $matches[2] ?? null,
    'infraspecific_rank' => null,
    'infraspecific_name' => null,
    'authority_in_parentheses' => null,
    'authority' => null,
    'year' => null,
  ];

  // Extract infraspecific if present.
  if (isset($matches[3]) && isset($matches[4])) {
    $result['infraspecific_rank'] = $matches[3];
    $result['infraspecific_name'] = $matches[4];
    // Authority positions shift when infraspecific is present.
    $authIndex = 5;
    $parenAuthIndex = 5;
    $yearIndex = 7;
  }
  else {
    $authIndex = 5;
    $parenAuthIndex = 5;
    $yearIndex = 6;
  }

  // Extract parenthetical authority.
  if (isset($matches[$parenAuthIndex]) && !empty($matches[$parenAuthIndex])) {
    // Check if it's actually a parenthetical authority or a false match.
    $potential = trim($matches[$parenAuthIndex]);
    if (strpos($potential, '(') === FALSE && !preg_match('/^(subsp|var|f|forma)/', $potential)) {
      $result['authority_in_parentheses'] = $potential;
    }
  }

  // Extract authority
  if (isset($matches[$authIndex]) && !empty($matches[$authIndex])) {
    $potential = trim($matches[$authIndex]);
    // Skip if it looks like an infraspecific rank
    if (!preg_match('/^(subsp|var|f|forma|subspecies|variety)/', $potential)) {
      // Remove trailing "nom. illeg." etc.
      $potential = preg_replace('/\s*,\s*[a-z. ]+$/', '', $potential);
      $result['authority'] = $potential;
    }
  }

  // Extract year.
  if (isset($matches[$yearIndex])) {
    $result['year'] = trim($matches[$yearIndex]);
  }

  return $result;
}
