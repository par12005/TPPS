<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Shows simple report of 'tpps_phenotype_unit_warning' table.
 */
function tpps_admin_unit_warning_report() {
  $table = 'public.tpps_phenotype_unit_warning';
  $table_alias = str_replace('.', '', $table);
  $page_title = variable_get(
    'tpps_report_unit_warning_title',
    'Unit Warning (list of phenotypes which unit differs from Synonym)'
  );

  // Remove selected item.
  if (!empty($_GET['remove'])) {
    db_delete($table)
      ->condition('study_name', check_plain($_GET['remove']))
      ->execute();
    drupal_set_message(t(
      'Removed record for study @study_name.',
      ['@study_name' => $_GET['remove']]
    ));
    drupal_goto($_GET['q']);
  }

  $query = db_select($table, $table_alias)
    ->fields($table_alias, [
      'study_name',
      // 2nd column is required to emulate 'Actions' column.
      // See formatter-function.
      'study_name',
    ]);

  return '<h2>' . $page_title . '</h2>' . easy_report([
    'css_path' => drupal_get_path('module', 'tpps') . TPPS_CSS_PATH,
    'formatter' => 'tpps_admin_unit_warning_report_formatter',
    'page_title' => $page_title,
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'header' => [
      ['data' => t('Study'), 'field' => $table_alias . '.study_name'],
      ['data' => t('Actions'), 'field' => 'actions'],
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
function tpps_admin_unit_warning_report_formatter(string $name, $value, array &$row) {
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

    case 'publictpps_phenotype_unit_warning_study_name':
      $formatted = l(t('Remove'), $_GET['q'], ['query' => ['remove' => $formatted]]);
      break;
  }
  return $formatted;
}
