<?php

/**
 * @file
 * Helper functions to build reports based on db tables.
 */

/**
 * Generates report based on given DB Query.
 *
 * @param array $meta
 *   Metadata for report. See README.md for more details.
 *
 * @return string
 *   Returns themed table with pagers.
 */
function easy_report(array $meta) {
  if (empty($meta['header']) || empty($meta['query'])) {
    drupal_set_message('No metadata to build report.');
    return '';
  }
  extract($meta);
  if (!empty($debug_mode)) {
    $query->addTag('debug');
  }
  // Pager and sorting.
  $report_query = $query->extend('PagerDefault')
    ->limit(($items_per_page ? $items_per_page : 25))
    ->extend('TableSort')
    ->orderByHeader($header);
  $data = $report_query->execute()->fetchAll();
  // Formatter.
  if (!isset($meta['formatter'])) {
    // When $meta['formatter'] was just wasn't set then use default.
    // But it it was set to NULL, FALSE or empty string just skip formatting.
    $formatter = 'easy_report_formatter';
  }
  $is_formatter = (!empty($formatter) && function_exists($formatter));

  // Prepare rows for report.
  $rows = [];
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
  if (!empty($page_title)) {
    drupal_set_title($page_title);
  }
  return theme('pager') . theme('table',
    [
      'header' => $header,
      'empty' => $empty_message ?? t('No data found.'),
      'attributes' => $table_attributes,
      'rows' => $rows,
    ]
  ) . theme('pager');
}

/**
 * Values formatter for 'easy_report()'.
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
function example_easy_report_formatter(string $name, $value, array $row) {
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
