<?php

/**
 * @file
 * Helper functions to build reports based on db tables.
 */

/**
 * Generates report based on given DB Query.
 *
 * @param array $meta
 *   Metadata for report. Associative array. Example:
 *   $meta = [
 *     'formatter' => '',
 *     'debug_mode' => TRUE,
 *     // Optional. Default: 25.
 *     'items_per_page' => 25,
 *     'css_path' => drupal_get_path('module', 'tpps') . TPPS_CSS_PATH,
 *     'title' => variable_get('tpps_report_order_family_not_exist_title'),
 *     'table_attributes' => ['class' => ['view' , 'tpps_table']],
 *     // Optional. Default: 'No data found.'.
 *     'empty_message' => t('No data found.'),
 *     'header' => [
 *       [
 *         'data' => t('Organism Id'),
 *         // Note: $table_alias could be skipped here.
 *         'field' => $table_alias . '.organism_id',
 *       ],
 *     ],
 *     // Drupal DB query. Pager and sorting will be added.
 *     'base_query' => '',
 *   ]; //.
 *
 * @return string
 *   Returns themed table with pagers.
 */
function easy_report(array $meta) {
  extract($meta);
  if (!empty($debug_mode)) {
    $base_query->addTag('debug');
  }
  $query = $base_query->extend('PagerDefault')
    ->limit(($items_per_page ? $items_per_page : 25))
    ->extend('TableSort')
    ->orderByHeader($header);
  $data = $query->execute()->fetchAll();
  // Format.
  $rows = [];
  $is_formatter = (!empty($formatter) && function_exists($formatter));
  foreach ($data as $row_id => $row) {
    foreach ($row as $field_id => $field_value) {
      $rows[$row_id][$field_id] = [
        'data' => (!$is_formatter ? check_plain($field_value)
          : call_user_func($formatter, $field_id, $field_value, (array) $row)
        ),
        'style' => 'vertical-align:top;min-width:10px;',
      ];
    }
  }
  if (!empty($css_path)) {
    drupal_add_css($css_path);
  }
  $output = '';
  if (!empty($page_title)) {
    drupal_set_title($page_title);
  }
  $output .= theme('pager');
  $output .= theme('table',
    [
      'header' => $header,
      'empty' => ($empty_message ? $empty_message : t('No data found.')),
      'attributes' => $table_attributes,
      'rows' => $rows,
    ]
  );
  $output .= theme('pager');
  return $output;
}

/**
 * Values formatter for 'tpps_report()'.
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
function easy_report_formatter(string $name, $value, array $row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'publicctestudy_name':
      if (!empty($value)) {
        $formatted = l($formatted, 'tpps-admin-panel/' . $formatted);
      }
      break;
  }
  return $formatted;
}
