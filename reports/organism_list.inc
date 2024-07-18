<?php

/**
 * @file
 * Shows list of organisms per study.
 */

/**
 * Menu callback. Shows list of organisms per study.
 */
function tpps_admin_organism_list_report() {
  $name = 'organism_list';
  $page_title = variable_get('tpps_report_' . $name . '_title',
        'Organism List');

  // WARNING: easy_report() requires db_select() not db_query().
  $query = db_select('public.tpps_submission', 'ts');
  $query->leftJoin('chado.project_dbxref', 'pdbxref', 'ts.dbxref_id = pdbxref.dbxref_id');
  $query->join('chado.project_organism', 'po', 'po.project_id = pdbxref.project_id');
  $query->leftJoin('chado.project', 'p', 'p.project_id = po.project_id');
  $query->addField('ts', 'accession');
  $query->addField('pdbxref', 'project_id');
  $query->addField('po', 'organism_id');
  $query->addField('p', 'name');
  $query->condition('ts.status', '%Approved%', 'LIKE');

  return '<h2>' . $page_title . '</h2>' . easy_report([
    'css_path' => TPPS_MODULE_PATH . TPPS_CSS_PATH,
    'page_title' => $page_title,
    'formatter' => 'tpps_admin_' . $name . '_report_formatter',
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'header' => [
      ['data' => t('Accession'), 'field' => 'ts.accession'],
      ['data' => t('Project Id'), 'field' => 'pdbxref.project_id'],
      ['data' => t('Organism Id'), 'field' => 'organism_id'],
      ['data' => t('Project Name'), 'field' => 'p.name'],
    ],
    'query' => $query,
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
function tpps_admin_organism_list_report_formatter(string $name, $value, array &$row) {
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
