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
      ->fields('t', ['organismprop_id', 'value'])
      ->condition('type_id', OrganismSynonym::CVTERM_ID)
      ->orderBy('t.value')
      //->condition('organismprop_id', 75905)
      //->range(0, 90)
      ->execute()
      ->fetchAllKeyed();

    $i = 1;
    foreach ($data as $id => $synonym_name) {
      if (strpos($synonym_name, '?') === 0) {
        continue;
      }
      $parts = tpps_organism_parse_name($synonym_name);
      if (1) {
        if (!empty($parts['genus'])) {
          continue;
        }
      }
      $rows[$synonym_name] = array_merge(
        [
          //'number' => $i++,
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
  $result = [
    'genus' => NULL,
    'species' => NULL,
    'infraspecific_rank' => NULL,
    'infraspecific_name' => NULL,
    'authority_in_parentheses' => NULL,
    'authority' => NULL,
    'year' => NULL,
  ];

  // Handle cases where there's no species (just genus).
  if (preg_match('/^([A-Z][a-z]+)$/', $input, $matches)) {
    $result['genus'] = $matches[1];
    return $result;
  }

  // Get genus+species.
  $pattern = '/^
    ([A-Z][a-z]+)                     # 1. Genus
    \s+
    ([a-z-]+(?:\s*[a-z-]+)?)          # 2. Species (including ×)
    $/xu';
  if (preg_match($pattern, $input, $matches)) {
    $result['genus'] = $matches[1] ?? NULL;
    $result['species'] = $matches[2] ?? NULL;
    return $result;
  }

  // Genus + authority with parentheses + optional outside authority.
  $pattern = '/^
    ([A-Z][a-z]+)                          # 1. Genus.
    \s+\(([^)]+)\)\s*                      # 2. Authority in parentheses.
    ([A-Za-z0-9\.\x{00E0}-\x{00FC}\&\s]+?) # 3. Authority
    (?:,\s*(\d{4}))?                       # 4. Optional year.
    $/xu';
  if (preg_match($pattern, $input, $matches)) {
    $result['genus'] = $matches[1];
    $result['authority_in_parentheses'] = trim($matches[2]);
    $result['authority'] = trim($matches[3] ?? NULL);
    $result['year'] = $matches[4] ?? NULL;
    return $result;
  }

  // Handle cases with just genus + authority + optional year.
  // Authority must start with uppercase letter to distinguish from species.
  $pattern = '/^([A-Z][a-z]+)\s+([A-Z][A-Za-z0-9\.
    \x{00E0}-\x{00FC}\x{0100}-\x{017F}\&\s]+?)(?:,\s*(\d{4}))?
    $/xu';
  if (preg_match($pattern, $input, $matches)) {
    $result['genus'] = $matches[1];
    $result['authority'] = trim($matches[2]);
    $result['year'] = $matches[3] ?? NULL;
    return $result;
  }


  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Allow optional dot after rank
  $pattern = '/\s
    (subsp|subspecies|var|variety|f|forma|subvar|subf|proles|nothosubsp|nothovar|spp)
    \.*\s/x';
  preg_match($pattern, $input, $matches);
  $infraspecific_rank = $matches[1] ?? NULL;
  if (!empty($infraspecific_rank)) {
    $result['infraspecific_rank'] = $infraspecific_rank;
    // Split by rank.
    $parts = explode($matches[0], $input, 2);
  }
  else {
    // No infraspecific rank.
    if (strpos($input, '(') !== FALSE) {
      $parts = preg_split('/(?=[\(])/', $input, 2);
      //$parts = explode('(', $input, 2);
    }
    // Check for uppercase word (split before first uppercase letter).
    elseif (
      // Find all uppercase letters with positions
      preg_match_all('/[A-Z]/', $input, $matches, PREG_OFFSET_CAPTURE) >= 2
    ) {
      $second_uppercase_char_position = $matches[0][1][1];
      $parts = [
        substr($input, 0, $second_uppercase_char_position),
        substr($input, $second_uppercase_char_position)
      ];
    }
    else {
      // @TODO Check if there only 2 words.
      $parts = [$input];
    }
  }

  if (1) {
    // Get genus+species.
    $pattern = '/^
      ([A-Z][a-z-]+)                         # 1. Genus
      \s+
      ([\×a-z-]+(?:\s*[\×a-z-]+)?)          # 2. Species (including ×)
      /xu';
    if (preg_match($pattern, trim($parts[0]), $matches)) {

      $result['genus'] = $matches[1] ?? NULL;
      $result['species'] = $matches[2] ?? NULL;
    }

    if (empty($parts[1])) {
      return $result;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get infraspecific name.
    // Marker: first char is lowercase.
    $pattern = '/^
        ([\×a-z-]+(?:\s*[\×a-z-]+)?)        # Infraspecific name (including ×)
        # Not to the end of string.
      /xu';
    preg_match($pattern, trim($parts[1]), $matches);
    if (!empty($matches[1])) {
      $result['infraspecific_name'] = $matches[1];
      $parts[1] = str_replace($result['infraspecific_name'], '', $parts[1]);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get Year.
    $pattern = '/,*\s*(\d{4})$/';
    preg_match($pattern, trim($parts[1]), $matches);
    if (!empty($matches[1])) {
      $result['year'] = $matches[1];
      // Note: replace with leading comma and space.
      $parts[1] = trim(str_replace($matches[0], '', $parts[1]));
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Authority in parentheses (optional).
    $pattern = '/\(([^)]+)\)/';
    preg_match($pattern, trim($parts[1]), $matches);
    if (!empty($matches[1])) {
      $result['authority_in_parentheses'] = $matches[1];
      $parts[1] = trim(str_replace($matches[0], '', $parts[1]));
    }

    if (!empty($parts[1])) {
      $result['authority'] = trim($parts[1]);
    }

  }
  else {
    // No infraspecific rank.
    // Pattern to match the full structure - more permissive for authors.
    $pattern = '/^
      ([A-Z][a-z]+)                         # 1. Genus
      \s+
      ([\×a-z-]+(?:\s*[\×a-z-]+)?)          # 2. Species (including ×)
      (?:
        \s+
        (subsp|subspecies|var|variety|f|forma|subvar|subf|proles|nothosubsp|nothovar|spp) # 3.
        \.*                                  # Allow optional dot after rank
        \s+
        ([\×a-z-]+(?:\s*[\×a-z-]+)?)        # Infraspecific name (including ×)
      )?
      (?:
        \s*
        ,?                                  # Optional comma before authority
        \s*
        (?:                                 # 4. Authority in parentheses (optional)
          \(
            ([^)]+)                         # Authority inside parentheses
          \)
          \s*
        )?
        (?:                                 # 5. Authority outside parentheses
          ([A-Z][\p{L}0-9\.\&\s\,]+?)       # Authority (must start with uppercase, Unicode support)
        )?
        (?:                                 # 6. Year (optional)
          ,\s*
          (\d{4})
        )?
      )?
      $/xu';
    preg_match($pattern, $input, $matches);

    $result = [
      'genus' => $matches[1] ?? NULL,
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
      // Authority positions shift when infraspecific is present.
      $parenAuthIndex = 5;
      $authIndex = 6;
      $yearIndex = 7;
    }
    else {
      $parenAuthIndex = 4;
      $authIndex = 5;
      $yearIndex = 6;
    }

    // Extract parenthetical authority.
    if (isset($matches[$parenAuthIndex]) && !empty($matches[$parenAuthIndex])) {
      $result['authority_in_parentheses'] = trim($matches[$parenAuthIndex]);
    }

    // Extract authority.
    if (isset($matches[$authIndex]) && !empty($matches[$authIndex])) {
      $potential = trim($matches[$authIndex]);
      // Skip if it looks like an infraspecific rank.
      if (!preg_match('/^(subsp|var|f|forma|subspecies|variety)/', $potential)) {
        // Remove trailing year if it's attached.
        $potential = preg_replace('/,\s*\d{4}$/', '', $potential);
        // Remove trailing "nom. illeg." etc.
        $potential = preg_replace('/\s*,\s*[a-z. ]+$/', '', $potential);
        $result['authority'] = $potential;
      }
    }

    // Extract year.
    if (isset($matches[$yearIndex])) {
      $result['year'] = trim($matches[$yearIndex]);
    }
  }

  return $result;
}
