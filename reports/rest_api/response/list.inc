<?php

/**
 * @file
 * Shows list of the all Rest API Requests.
 */

/**
 * Menu callback. Shows list of the all Rest API Requests.
 */
function tpps_report_rest_api_response_list() {
  $report_name = 'rest_api_response_list';
  $page_title = variable_get('tpps_report_' . $report_name . '_title', 'Rest API Response List');

  // WARNING: Report::print() requires db_select() not db_query().
  $query = db_select(RestAPILog::RESPONSE_TABLE, 't')
    ->fields('t', [
      'id',
      'request_id',
      'started',
      'code',
      'duration',
      'body',
    ]);
  $query->join(RestAPILog::REQUEST_TABLE, 't2', 't2.request_id = t.request_id');
  $query->addField('t2', 'tag');

  return '<h2>' . $page_title . '</h2>' . Report::print([
    'css_path' => TPPS_MODULE_PATH . TPPS_CSS_PATH,
    'page_title' => $page_title,
    'formatter' => 'tpps_report_' . $report_name . '_formatter',
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'items_per_page' => variable_get('tpps_report_items_per_page', 15),
    'header' => [
      ['data' => t('Id'), 'field' => 't.id', 'sort' => 'desc'],
      ['data' => t('Request Id'), 'field' => 't.request_id'],
      ['data' => t('Started'), 'field' => 't.started'],
      ['data' => t('HTTP Code'), 'field' => 't.code'],
      ['data' => t('Duration, msec'), 'field' => 't.duration'],
      ['data' => t('Body'), 'field' => 't.body'],
      ['data' => t('Request'), 'field' => 't.tag'],
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
function tpps_report_rest_api_response_list_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'id':
      if (!empty($value)) {
        $url = 'admin/reports/tpps/rest_api/response/body/' . $formatted;
        $formatted = l($formatted, $url);
      }
      else {
        $formatted = 'Empty';
      }
      break;

    case 'request_id':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/tpps/rest_api/request/'
          . $formatted);
      }
      break;

    case 'body':
      if (!empty($value)) {
        $url = 'admin/reports/tpps/rest_api/response/body/' . $row['id'];
        $formatted = substr($formatted, 0, 70)
          . '... ' . l(t('View more'), $url);
      }
      else {
        $formatted = 'Empty';
      }
      break;

    case 'started':
      if (!empty($value)) {
        $formatted = format_date($formatted, 'custom', "Y/m/d H:i:s O");
          //. ' (' . $formatted . ')';
      }
      break;

    case 'code':
      $formatted = theme('http_code', ['code' => $formatted]);
      break;

  }
  return $formatted;
}
