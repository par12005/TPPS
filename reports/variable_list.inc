<?php

/**
 * @file
 * Shows list of module's variables and their values.
 */

/**
 * Menu callback. Shows list of module's variables and their values.
 */
function tpps_admin_variable_list_report() {
  $report_name = 'variable_list';
  $report = tpps_report_get_list()[$report_name];

  $table = 'public.variable';
  $table_alias = str_replace('.', '', $table);
  $query = db_select($table, $table_alias)
    ->fields($table_alias, ['name', 'value'])
    ->condition('name', 'tpps%', 'LIKE');
  $query->addField($table_alias, 'value', 'serialized_value');

  return '<h2>' . $report['title'] . '</h2>' . easy_report([
    'css_path' => TPPS_MODULE_PATH . TPPS_CSS_PATH,
    'formatter' => is_live_site() ? ''
    : 'tpps_admin_' . $report_name . '_report_formatter',
    'page_title' => $report['title'],
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'items_per_page' => 1000,
    'header' => [
      ['data' => t('Name'), 'field' => $table_alias . '.name'],
      ['data' => t('Value'), 'field' => $table_alias . '.value'],
      [
        'data' => t('Serializes Value'),
        'field' => $table_alias . '.serialized_value'
      ],
    ],
    'query' => $query,
  ]);
}

/**
 * Values formatter for 'tpps_admin_variable_name_report'.
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
function tpps_admin_variable_list_report_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'name':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/db/public.variable',
          ['query' => ['public.variable.name' => $formatted]]
        );
      }
      break;

    case 'value':
      if (!empty($value)) {
        $formatted = var_export(@unserialize($value), 1);
      }
      break;
  }
  return $formatted;
}
