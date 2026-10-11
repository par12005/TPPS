<?php

/**
 * @file
 * Shows list of all the organisms in db table 'chado.organism'.
 */

/**
 * Menu callback. Shows list of all organisms.
 */
function tpps_report_organism_taxonomy() {
  $report_name = 'organism_taxonomy';
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $query = db_select(Organism::TABLE, 'co');
  $query->leftJoin(
    'public.' . OrganismTaxonomy::TABLE, 'ot',
    'ot.organism_id = co.organism_id'
  );
  $query->fields('co', [
    'organism_id',
    'genus',
    'species',
    'type_id',
    'infraspecific_name',
  ]);
  $query->fields('ot', ['source', 'taxonomy_id']);

  // Header.
  $header = [
    ['data' => t('Organism Id'), 'field' => 'co.organism_id'],
    ['data' => t('Genus'), 'field' => 'co.genus'],
    ['data' => t('Species'), 'field' => 'co.species'],
    ['data' => t('Infraspecific Rank'), 'field' => 'co.type_id'],
    ['data' => t('Infraspecific Name'), 'field' => 'co.infraspecific_name'],
    ['data' => t('Source'), 'field' => 'ot.source'],
    ['data' => t('Taxonomy Id'), 'field' => 'ot.taxonomy_id'],
  ];

  return Report::print([
    'name' => $report_name,
    'header' => $header,
    'query' => $query,
    'pager' => 'none',
  ], $reset);
}

/**
 * Values formatter for tpps_report_organism_taxonomy().
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
function tpps_report_organism_taxonomy_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
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

    case 'type_id':
      if (!empty($formatted)) {
        $formatted = InfraspecificRank::getName($formatted);
      }
      break;

    case 'infraspecific_name':
      if (!empty($formatted)) {
        if (!empty($row['type_id'])) {
          // Get rank name.
          $pattern = '/' . InfraspecificRank::getRegex($row['type_id']) . '\s/';
          // Replace rank abbreviation in rank name.
          $formatted = trim(preg_replace($pattern, '', $formatted));
        }
        else {
          // Empty 'type_id' but 'name' could have rank.
          // Let's just leave it for now.
        }
      }
      break;

    case 'taxonomy_id':
      if (!empty($value)) {
        $formatted = l($formatted, NcbiTaxonomy::getUrl($row['taxonomy_id']));
        $formatted = '<em>' . $formatted . '</em>';
      }
      break;

  }
  return $formatted;
}
