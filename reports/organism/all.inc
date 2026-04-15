<?php

/**
 * @file
 * Shows list of all the organisms in db table 'chado.organism'.
 */

/**
 * Menu callback. Shows list of all organisms.
 */
function tpps_report_organism_all_list() {
  $name = 'organism_all';
  $page_title = variable_get('tpps_report_' . $name . '_title', 'All Organism List');
  $table = 'chado.organism';

  // WARNING: Report::print() requires db_select() not db_query().
  $query = db_select($table, 'o')->fields('o', []);
  $query->addExpression("CONCAT(o.genus, ' ', o.species)", 'organism_name');
  $query->orderBy('organism_id');

  return '<h2>' . $page_title . '</h2>' . Report::print([
    'css_path' => TPPS_MODULE_PATH . TPPS_CSS_PATH,
    'page_title' => $page_title,
    'formatter' => 'tpps_report_' . $name . '_formatter',
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'header' => [
      ['data' => t('Organism Id'), 'field' => 'o.organism_id'],
      ['data' => t('Organism Name'), 'field' => 'organism_name'],
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
function tpps_report_organism_list_formatter(string $name, $value, array &$row) {
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
