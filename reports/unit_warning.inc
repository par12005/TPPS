<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Shows simple report of 'tpps_phenotype_unit_warning' table.
 */
function tpps_admin_unit_warning_report(array $filter = array()) {
  // Note: Unique fields: .
  $table = 'tpps_phenotype_unit_warning';
  //return simple_table_report($table);
  $meta_data = array(
    'header' => [
      ['data' => t('Phenotype Id'), 'field' => $table . '.phenotype_id'],
      //['data' => t('Phenotype Name'), 'field' => 'chado.phenotype.name'],
    ],
    'tables' => [
      $table => [],
      //'chado.phenotype' => [
      //  'join' => [
      //    'type' => 'leftJoin',
      //    'on' => $table . '.phenotype_id = chado.phenotype.phenotype_id',
      //  ],
      //  'fields' => ['name'],
      //]
    ],
    'formatter' => 'tpps_report_formatter',
    'items_per_page' => variable_get('tpps_report_items_per_page', 25),
    'refresh_time' => variable_get('tpps_report_refresh_time', 0),
    'tableselect' => [
      'primary_key' => 'phenotype_id',
      'select_all' => TRUE,
      'actions' => [
        // name => callback.
        'Remove' => 'tpps_unit_warning_report_remove',
      ],
    ],
  );
  return simple_report($meta_data, $filter);
}

/**
 * Values formatter for 'simple_report'.
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
function tpps_report_formatter(string $name, $value, array $row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'phenotype_id':
      if (!empty($value)) {
        $path = 'wholeplant/' . check_plain($value);
        $formatted = l(check_plain($value), $path);
      }
      break;

  }
  return $formatted;
}


/**
 * Removes items selected at 'Unit Warning Report' page.
 *
 * @param array $form_state
 *   Form API submitted data.
 */
function tpps_unit_warning_report_remove(array $form_state) {
  $values = $form_state['values'];
  $condition = (
    empty($table_name = $values['table_name'])
    || empty($primary_key = $values['primary_key'])
    || empty($id_list = array_filter(array_values($values['table'])))
  );
  if ($condition) {
    return;
  }
  // @TODO Should try-catch be uses here?
  db_delete($table_name)->condition($primary_key, $id_list, 'IN')->execute();
  $message = 'Removed unit warning for phenotypes: @list.';
  $args = ['@list' => implode(', ', $id_list)];
  drupal_set_message($message, $args);
  watchdog('tpps', $message, $args);
}
