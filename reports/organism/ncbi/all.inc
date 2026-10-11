<?php

/**
 * @file
 * Shows list of the NCBI Taxonomy Ids.
 */

/**
 * Menu callback. Shows list of the all organisms and their NCBI Taxonomy Ids.
 */
function tpps_report_organism_ncbi_all() {
  $report_name = 'organism_ncbi_all';
  $table = 'public.' . OrganismTaxonomy::TABLE;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $query = db_select('chado.organism', 'co');
  // Taxonomy Id.
  $query->leftJoin($table, 'ot', 'co.organism_id = ot.organism_id');
  $query->condition('ot.source', NcbiTaxonomy::KEY);
  // Infraspecific Rank.

  if (0) {
    $query->leftJoin(
      'chado.cvterm',
      'cvt',
      'cvt.cvterm_id = co.type_id AND cvt.cv_id = :cv_id',
      [':cv_id' => InfraspecificRank::CV_ID]
    );
    // Infraspecific Rank.
    $query->fields('cvt', ['name']);
  }

  $query->fields('co', [
    'organism_id',
    'genus',
    'species',
    'type_id',
    'infraspecific_name',
  ]);
  $query->fields('ot', ['taxonomy_id']);

  $header = [
    ['data' => t('Organism Id'), 'field' => 'co.organism_id'],
    ['data' => t('Genus'), 'field' => 'co.genus'],
    ['data' => t('Species'), 'field' => 'co.species'],
    ['data' => t('Infraspecific Rank'), 'field' => 'co.type_id'],
    ['data' => t('Infraspecific Name'), 'field' => 'co.infraspecific_name'],
    ['data' => t('Taxonomy Id'), 'field' => 'ot.taxonomy_id'],
  ];

  return Report::print([
    'name' => $report_name,
    'header' => $header,
    'query' => $query,
    'pager' => 'both',
  ], $reset);
}

/**
 * Values formatter for tpps_admin_organism_list_report().
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
function tpps_report_organism_ncbi_all_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {

    case 'organism_id':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/db/chado.project_organism',
          ['query' => ['chado.project_organism.organism_id' => $formatted]]
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
  }
  return $formatted;
}
