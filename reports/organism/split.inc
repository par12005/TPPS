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

  if (!empty($_GET['id'])) {
    $reset = TRUE;
  }
  if ($reset || empty($cache) || empty($cache->data[$key])) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $rows = $default_value;
    $query = db_select(OrganismSynonym::TABLE, 't')
      ->fields('t', ['organismprop_id', 'value'])
      ->condition('type_id', OrganismSynonym::CVTERM_ID)
      //->range(0, 90)
      ->orderBy('t.value');
    if (!empty($_GET['id'])) {
      $query->condition('organismprop_id', $_GET['id']);
    }
    $data = $query->execute()->fetchAllKeyed();

    foreach ($data as $id => $synonym_name) {
      if (strpos($synonym_name, '?') === 0) {
        continue;
      }
      $parts = tpps_organism_parse_name($synonym_name);
      $rows[$synonym_name] = array_merge(
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
    \s+([a-z-]+(?:\s*[a-z-]+)?)       # 2. Species (including ×)
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
  elseif (strpos($input, '(') !== FALSE) {
    $parts = preg_split('/(?=[\(])/', $input, 2);
  }
  elseif (
    // Find all uppercase letters with positions.
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get genus+species.
  $pattern = '/^
    ([A-Z][a-z-]+)                         # 1. Genus
    \s+
    ([\×a-z-]+(?:\s*[\×a-z-.]+)?)          # 2. Species (including ×)
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
      ([\×a-z-]+(?:\s*[\×a-z-.]+)?)        # Infraspecific name (including ×)
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

  return $result;
}
