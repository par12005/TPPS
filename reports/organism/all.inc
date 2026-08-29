<?php

/**
 * @file
 * Shows list of all the organisms in db table 'chado.organism'.
 */

/**
 * Menu callback. Shows list of all organisms.
 */
function tpps_report_organism_all() {
  // WARNING: Report::print() requires db_select() not db_query().
  $query = db_select('chado.organism', 'o')->fields('o', ['organism_id']);
  $query->orderBy('organism_id');

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $add_accession = 0;
  // Get study accession.
  if ($add_accession ?? FALSE) {
    $query->leftJoin(
      'chado.project_organism',
      'cpo',
      'o.organism_id = cpo.organism_id'
    );
    $query->addField('cpo', 'project_id');
    $query->leftJoin(
      'chado.project_dbxref',
      'cpd',
      'cpo.project_id = cpd.project_id'
    );

// @BUG: Multiple 'dbxref_id' in 'project_dbxref' for the same 'project_id'.

    $query->leftJoin(
      'public.tpps_submission',
      'pts',
      'cpd.dbxref_id = pts.dbxref_id'
    );
    $query->addField('pts', 'accession');
  }
  $query->addExpression("CONCAT(o.genus, ' ', o.species)", 'organism_name');

  // Header.
  $header = [
    ['data' => t('Organism Id'), 'field' => 'o.organism_id'],
  ];
  if ($add_accession ?? FALSE) {
    $header[] = [
      'data' => t('Project Id'),
      'field' => 'cpo.project_id',
    ];
    $header[] = [
      'data' => t('Accession'),
      'field' => 'pts.accession',
    ];
  }
  $header = array_merge($header, [
    [
      'data' => t('Organism Name'),
      'field' => 'organism_name',
    ]
  ]);

  return Report::print([
    'name' => 'organism_all',
    'header' => $header,
    'query' => $query,
    'pager' => 'both',
  ]);
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
function tpps_report_organism_all_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'accession':
      if (!empty($value)) {
        $formatted = l($formatted, 'api/submission/' . $formatted . '/view/state');
      }
      break;

    case 'project_id':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/db/chado.project_organism',
          ['query' => ['chado.project_organism.project_id' => $formatted]]
        );
      }
      break;

    case 'organism_id':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/db/chado.project_organism',
          ['query' => ['chado.project_organism.organism_id' => $formatted]]
        );
      }
      break;
  }
  return $formatted;
}
