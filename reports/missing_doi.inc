<?php

/**
 * @file
 * Shows list of studies without DOI attached.
 */

/**
 * Menu callback. Shows studies without DOI attached.
 */
function tpps_admin_missing_doi_report() {
  $page_title = variable_get('tpps_report_missing_doi_title', 'Missing DOI');

  // Original query from Emily.
  //  select
  //    p.project_id,
  //    p.name,
  //    p.description,
  //    d.accession
  //  from chado.project p
  //  join chado.project_dbxref pd on p.project_id = pd.project_id
  //  join chado.dbxref d on pd.dbxref_id = d.dbxref_id
  //  where p.project_id in (
  //    select project_id
  //    from chado.project_dbxref
  //    where dbxref_id in (
  //      select dbxref_id from chado.dbxref where accession ilike 'tgdr%'
  //    )
  //  )
  //  and p.project_id not in (
  //    select project_id
  //    from chado.project_dbxref
  //    where dbxref_id in (
  //      select dbxref_id from chado.dbxref where accession ilike '%\/%'
  //    )
  //  )
  //  order by project_id;

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

  return '<h2>' . $page_title . '</h2>' . easy_report([
    'css_path' => TPPS_MODULE_PATH . TPPS_CSS_PATH,
    'formatter' => 'tpps_admin_missing_doi_report_formatter',
    'page_title' => $page_title,
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'header' => [
      ['data' => t('Study'), 'field' => 'accession'],
      ['data' => t('Project Id'), 'field' => 'project_id'],
      ['data' => t('Name'), 'field' => 'name'],
      ['data' => t('Description'), 'field' => 'chadoproject.description'],
    ],
    'query' => $query,
  ]);
}

/**
 * Values formatter for 'tpps_admin_unit_warning_report'.
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
function tpps_admin_missing_doi_report_formatter(string $name, $value, array &$row) {
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
