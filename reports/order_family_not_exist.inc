<?php

/**
 * @file
 * Generates list of Phenotypes which Unit differ from Synonym's Unit.
 */

/**
 * Menu callback. Report to find term (Order/Family) not present in DB.
 */
function tpps_admin_order_family_not_exist_report() {
  $formatter = '';
  $debug_mode = FALSE;
  $table = 'chado.organism';
  $items_per_page = 25;
  $css_path = drupal_get_path('module', 'tpps') . TPPS_CSS_PATH;
  $title = variable_get('tpps_report_order_family_not_exist_title');
  $table_attributes = ['class' => ['view' , 'tpps_table']];
  $empty_message = t('No data found.');

  $table_alias = str_replace('.', '', $table);
  $output = '';
  $header = [
    [
      'data' => t('Organism Id'),
      'field' => $table_alias . '.organism_id',
    ],
    [
      'data' => t('Genus'),
      'field' => $table_alias . '.genus',
    ],
    [
      'data' => t('Species'),
      'field' => $table_alias . '.species',
    ],
    [
      'data' => t('Type Id'),
      'field' => $table_alias . '.type_id',
    ],
  ];
  // Ids to exclude.
  $list = db_select('chado.organismprop', 'op')
    ->fields('op', ['organism_id'])
    ->condition('type_id', [9, 10], 'IN')
    ->distinct()
    ->execute()
    ->fetchCol();
  // Get data.
  $base_query = db_select($table, $table_alias)
    ->fields($table_alias, ['organism_id', 'genus', 'species', 'type_id'])
    ->condition($table_alias . '.organism_id', $list, 'NOT IN');
  if ($debug_mode) {
    $base_query->addTag('debug');
  }
  $query = $base_query->extend('PagerDefault')
    ->limit($items_per_page)
    ->extend('TableSort')
    ->orderByHeader($header);
  $data = $query->execute()->fetchAll();

  // Format.
  foreach ($data as $row_id => $row) {
    foreach ($row as $field_id => $field_value) {
      // @todo add formatter.
      $rows[$row_id][$field_id] = [
        'data' => (
          empty($formatter)
          ? check_plain($field_value)
          : call_user_func($formatter, $field_id, $field_value, $row)
        ),
        'style' => 'vertical-align:top;min-width:10px;',
      ];
    }
  }

  drupal_add_css($css_path);
  if (!empty($title)) {
    $output = '<h2>' . $title . '</h2>';
    drupal_set_title($title);
  }
  $output .= theme('pager');
  $output .= theme('table',
    [
      'header' => $header,
      'empty' => $empty_message,
      'attributes' => $table_attributes,
      'rows' => $rows,
    ]
  );
  $output .= theme('pager');
  return $output;
}
