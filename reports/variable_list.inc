<?php

/**
 * @file
 * Shows list of module's variables and their values.
 */

/**
 * Menu callback. Shows list of module's variables and their values.
 */
function tpps_admin_variable_list_report() {
  $table = 'public.variable';
  $table_alias = str_replace('.', '', $table);
  $query = db_select($table, 't')
    ->fields('t', ['name', 'value'])
    ->condition('name', 'tpps%', 'LIKE');

  $header = [
    ['data' => t('Name'), 'field' => 'name'],
    ['data' => t('Value'), 'field' => 'value'],
  ];
  return Report::print([
    'name' => 'variable_list',
    'header' => $header,
    'query' => $query,
    'pager' => 'none',
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
function tpps_report_variable_list_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'name':
      if (!empty($value)) {
        $formatted = l(
          $formatted,
          'admin/reports/db/public.variable',
          ['query' => ['public.variable.name' => $formatted]]
        );
      }
      break;
  }
  return $formatted;
}
