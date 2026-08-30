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
  $query = db_select('chado.organism', 'o')
    ->fields('o', [
      'organism_id',
      'genus',
      'species',
      'infraspecific_name',
      'type_id',
    ]);
  // NCBI.
  $query->leftJoin(
    'public.' . NcbiTaxonomy::TABLE,
    'nt',
    'nt.organism_id = o.organism_id'
  );
  $query->addField('nt', 'taxonomy_id', 'ncbi');
  $query->addField('nt', 'name', 'ncbi_name');
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // GBIF.
  $query->leftJoin(
    'public.' . GbifSpeciesApi::TABLE,
    'gsa',
    'gsa.organism_id = o.organism_id'
  );
  $query->addField('gsa', 'taxonomy_id', 'gbif');
  $query->addField('gsa', 'name', 'gbif_name');

  // Header.
  $header = [
    ['data' => t('Organism Id'), 'field' => 'o.organism_id'],
    ['data' => t('Genus'), 'field' => 'o.genus'],
    ['data' => t('Species'), 'field' => 'o.species'],
    ['data' => t('Infraspecific Name'), 'field' => 'o.infraspecific_name'],
    ['data' => t('Type Id'), 'field' => 'o.type_id'],
    ['data' => t('NCBI'), 'field' => 'ncbi'],
    ['data' => t('NCBI Name'), 'field' => 'ncbi_name'],
    ['data' => t('GBIF'), 'field' => 'gbif'],
    ['data' => t('GBIF Name'), 'field' => 'gbif_name'],
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
    case 'ncbi':
      if (!empty($value)) {
        //$formatted = l($formatted, NcbiTaxonomy::getUrl($formatted));
        $organism_name = implode(' ', [
          $row['genus'],
          $row['species'],
          $row['infraspecific_name'],
        ]);
        $url = NcbiTaxonomy::getApiUrl($organism_name);
        $formatted = l($formatted, $url);
        $formatted = '<strong>' . $formatted . '</strong>';
      }
      break;

    case 'ncbi_name':
    case 'ncbi_full_name':
      if (!empty($value)) {
        $formatted = l($formatted, NcbiTaxonomy::getUrl($row['ncbi']));
        $formatted = '<em>' . $formatted . '</em>';
      }
      break;

    case 'gbif':
      if (!empty($value)) {
        $organism_name = implode(' ', [
          $row['genus'],
          $row['species'],
          $row['infraspecific_name'],
        ]);
        $url = GbifSpeciesApi::getApiUrl($organism_name);
        $formatted = l($formatted, $url);
        $formatted = '<strong>' . $formatted . '</strong>';
      }
      break;

    case 'gbif_name':
    case 'gbif_full_name':
      if (!empty($value)) {
        $formatted = l($formatted, GbifSpeciesApi::getUrl($row['gbif']));
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
