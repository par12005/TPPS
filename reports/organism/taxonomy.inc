<?php

/**
 * @file
 * Shows list of all the organisms in db table 'chado.organism'.
 */

/**
 * Menu callback. Shows list of all organisms.
 */
function tpps_report_organism_taxonomy() {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $query = db_select('public.' . OrganismTaxonomy::TABLE, 'ot');
  $query->leftJoin('chado.organism', 'o', 'ot.organism_id = o.organism_id');
  $query->fields('o', [
    'organism_id',
    'genus',
    'species',
    'type_id',
    'infraspecific_name',
  ]);
  $query->fields('ot', ['source', 'taxonomy_id']);

  // Header.
  $header = [
    ['data' => t('Organism Id'), 'field' => 'o.organism_id'],
    ['data' => t('Genus'), 'field' => 'o.genus'],
    ['data' => t('Species'), 'field' => 'o.species'],
    ['data' => t('Type Id'), 'field' => 'o.type_id'],
    ['data' => t('Infraspecific Name'), 'field' => 'o.infraspecific_name'],
    ['data' => t('Source'), 'field' => 'ot.source'],
    ['data' => t('Taxonomy Id'), 'field' => 'ot.taxonomy_id'],
  ];

  return Report::print([
    'name' => 'organism_taxonomy',
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
    case 'type_id':
      if (!empty($value)) {
        $formatted = $formatted . ' {' . InfraspecificRank::getName($formatted) . '}';
      }
      break;

    case 'taxonomy_id':
      if (!empty($value)) {
        $formatted = l($formatted, NcbiTaxonomy::getUrl($row['taxonomy_id']));
        $formatted = '<em>' . $formatted . '</em>';
      }
      break;

    case 'organism_id':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/db/chado.organism',
          ['query' => ['chado.organism.organism_id' => $formatted]]
        );
      }
      break;
  }
  return $formatted;
}
