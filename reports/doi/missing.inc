<?php

/**
 * @file
 * Shows list of studies without DOI attached.
 */

/**
 * Menu callback. Shows studies without DOI attached.
 */
function tpps_report_doi_missing() {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $sub_query_1 = db_select('chado.dbxref', 't1')
    ->fields('t1', ['dbxref_id'])
    ->condition('accession', 'TGDR%', 'LIKE');
  $sub_query_2 = db_select('chado.project_dbxref', 't2')
    ->fields('t2', ['project_id'])
    ->condition('dbxref_id', $sub_query_1, 'IN');
  $sub_query_3 = db_select('chado.dbxref', 't3')
    ->fields('t3', ['dbxref_id'])
    ->condition('accession', '%\/%', 'LIKE');
  $sub_query_4 = db_select('chado.project_dbxref', 't4')
    ->fields('t4', ['project_id'])
    ->condition('dbxref_id', $sub_query_3, 'IN');

  $query = db_select('chado.project', 'p');
  $query->addField('d', 'accession');
  $query->addField('p', 'project_id');
  $query->addField('p', 'name');
  $query->addField('p', 'description');
  $query->leftJoin('chado.project_dbxref', 'pd', 'p.project_id = pd.project_id');
  $query->leftJoin('chado.dbxref', 'd', 'pd.dbxref_id = d.dbxref_id');
  $query->condition('p.project_id', $sub_query_2, 'IN');
  $query->condition('p.project_id', $sub_query_4, 'NOT IN');

  $header = [
    ['data' => t('Study'), 'field' => 'accession'],
    ['data' => t('Project Id'), 'field' => 'project_id'],
    ['data' => t('Name'), 'field' => 'name'],
    ['data' => t('Description'), 'field' => 'chadoproject.description'],
  ];
  return Report::print([
    'name' => 'doi_missing',
    'header' => $header,
    'query' => $query,
    'pager' => 'none',
  ], $reset);
}

/**
 * Values formatter for 'tpps_report_doi_missing' report.
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
function tpps_report_doi_missing_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'accession':
      if (!empty($value)) {
        $formatted = l($formatted, 'tpps-admin-panel/' . $formatted);
      }
      break;
  }
  return $formatted;
}
