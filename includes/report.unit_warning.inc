<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Shows simple report of 'tpps_phenotype_unit_warning' table.
 */
function tpps_admin_unit_warning_report(array $filter = []) {
  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  $table = 'tpps_phenotype_unit_warning';
  $meta = [
    // @DEBUG Shows SQL-query at page and table rows dump in watchdog.
    // 'tag' => 'debug',
    'attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => [
      ['data' => t('Phenotype Id'), 'field' => $table . '.phenotype_id'],
      ['data' => t('Phenotype Name'), 'field' => 'phenotype.name'],
    ],
    'tables' => [
      $table => [],
      'chado.phenotype' => [
        'join' => [
          'type' => 'left outer',
          'on' => $table . '.phenotype_id = phenotype.phenotype_id',
        ],
        'fields' => ['name'],
      ]
    ],
    'formatter' => 'tpps_admin_unit_warning_report_formatter',
    'items_per_page' => variable_get('tpps_report_items_per_page', 25),
    'refresh_time' => variable_get('tpps_report_refresh_time', 0),
    'primary_key' => 'phenotype_id',
    'select_all' => FALSE,
    'actions' => [
      // name --> callback.
      'Remove' => 'tpps_unit_warning_report_remove',
    ],
  ];
  return simple_report_tableselect($meta, $filter);
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
