<?php

/**
 * @file
 * Shows list of studies without DOI attached.
 */

/**
 * Menu callback. Shows studies without DOI attached.
 */
function tpps_missing_doi_report() {
  $table = 'chado.project';
  $page_title = variable_get('tpps_report_missing_doi_title', 'Missing DOI');

/*
select d.accession, p.name, p.description
from chado.project p
  // Get missing DOI
  left join chado.project_dbxref pd on pd.project_id = p.project_id

  //* Get study number
  left join chado.project_dbxref pd2 on p.project_id = p.project_id
  join chado.dbxref d on pd2.dbxref_id = d.dbxref_id

where pd.dbxref_id IS NULL;
 */

  $query = db_select($table, 'p');
  $query->addTag('debug');
  // Get missing DOI.
  $query->leftJoin(
    'chado.project_dbxref',
    'pd',
    'p.project_id = pd.project_id'
  );
  // Get study number.
  //$query->leftJoin(
  //  'chado.project_dbxref',
  //  'pd2',
  //  'p.project_id = p.project_id'
  //);
  $query->leftJoin(
    'chado.dbxref',
    'd',
    'pd.dbxref_id = d.dbxref_id'
  );
  // Columns.
  $query->addField('d', 'accession');
  $query->addField('p', 'project_id');
  $query->addField('p', 'name');
  $query->addField('p', 'description');
  // Conditions.
  //$query->isNotNull('d.accession');
  //$query->isNull('pd.project_dbxref_id');
  // 95 = TreeGenes Database.
  $query->condition('d.db_id', 95);


  return '<h2>' . $page_title . '</h2>' . easy_report([
    'css_path' => drupal_get_path('module', 'tpps') . TPPS_CSS_PATH,
    //'formatter' => 'tpps_missing_doi_report_formatter',
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
function tpps_missing_doi_report_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
    return $formatted;
  $name = check_plain($name);
  switch ($name) {
    case 'study_name':
      if (!empty($value)) {
        $formatted = l($formatted, 'tpps-admin-panel/' . $formatted);
      }
      break;

    case 'publictpps_phenotype_unit_warning_study_name':
      $formatted = l(t('Remove'), $_GET['q'], ['query' => ['remove' => $formatted]]);
      break;
  }
  return $formatted;
}
