<?php

/**
 * @file
 * Shows list of the all Rest API Requests.
 */

/**
 * Menu callback. Shows list of the all Rest API Requests.
 */
function tpps_report_rest_api_request_list() {
  $name = 'rest_api_request_list';
  $page_title = variable_get('tpps_report_' . $name . '_title', 'Rest API Request List');

  // WARNING: Report::print() requires db_select() not db_query().
  $query = db_select(RestAPILog::REQUEST_TABLE, 't')
    ->fields('t', [
      'request_id',
      'created',
      'method',
      'url',
      'header',
      'payload',
    ]);

  return '<h2>' . $page_title . '</h2>' . Report::print([
    'css_path' => TPPS_MODULE_PATH . TPPS_CSS_PATH,
    'page_title' => $page_title,
    'formatter' => 'tpps_report_' . $name . '_formatter',
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'header' => [
      ['data' => t('Request Id'), 'field' => 't.request_id'],
      ['data' => t('Created'), 'field' => 't.created'],
      ['data' => t('Method'), 'field' => 't.method'],
      ['data' => t('URL'), 'field' => 't.url'],
      ['data' => t('Header'), 'field' => 't.header'],
      ['data' => t('Payload'), 'field' => 't.payload'],
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
function tpps_report_rest_api_request_list_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {

    case 'request_id':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/tpps/rest_api/request/'
          . $formatted);
      }
      break;

    case 'header':
      $formatted = json_decode($value, 1);
      if (!empty($formatted)) {
        if (is_array($formatted)) {
          $formatted = TppsArray::print($formatted);
        }
      }
      else {
        $formatted = '';
      }
      break;

    case 'payload':
      $formatted = json_decode($value, 1);
      if (!empty($formatted)) {
        $url = 'admin/reports/tpps/rest_api/request/single/' . $row['request_id'];
        $formatted = check_plain(substr(var_export($formatted, 1), 0, 50)) . '... ' . l(t('View more'), $url);
      }
      else {
        $formatted = 'Empty';
      }
      break;

    case 'created':
      if (!empty($value)) {
        $formatted = format_date($formatted, 'custom', "Y/m/d H:i:s O");
      }
      break;

  }
  return $formatted;
}
