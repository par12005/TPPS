<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Shows simple report of 'tpps_phenotype_unit_warning' table.
 */
function tpps_admin_unit_warning_report(array $filter = []) {
  if (!function_exists('simple_report')) {
    return '';
  }
  $table = 'tpps_phenotype_unit_warning';
  $meta = [
    // @DEBUG Shows SQL-query at page and table rows dump in watchdog.
    //'tag' => 'debug',
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'refresh_time' => variable_get('tpps_report_refresh_time', 0),
    // Manually set 'primary_key' because DB table has no PK
    // but 'unique keys' index.
    'primary_key' => 'study_name',
    'select_all' => FALSE,
    // To have common style.
    'attributes' => ['class' => ['view' , 'tpps_table']],
    'formatter' => 'tpps_admin_unit_warning_report_formatter',
    'header' => [
      [
        'data' => t("Study"),
        'field' => 'public.' . $table . '.study_name',
        'theme' => 'none',
      ],
    ],
    'main_table' => [
      'schema' => 'public',
      'name' => $table,
    ],
  ];

  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  $title = variable_set(
    'tpps_report_unit_warning_title',
    'Unit Warning (list of phenotypes which unit differs from Synonym)'
  );
  if ($title) {
    $page_title = '<h2>' . $title . '</h2>';
  }
  return $page_title . simple_report_tableselect($meta, $filter ?? []);
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
function tpps_admin_unit_warning_report_formatter(string $name, $value, array $row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'study_name':
      if (!empty($value)) {
        $formatted = l($formatted, 'tpps-admin-panel/' . $formatted);
      }
      break;
  }
  return $formatted;
}
