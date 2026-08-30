<?php

/**
 * @file
 * Shows list of the all Rest API Requests.
 */

/**
 * Menu callback. Shows list of the all Rest API Requests.
 */
function tpps_report_rest_api_request_list() {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $field_list = [
    'request_id',
    'created',
    'method',
    'url',
    'header',
    'payload',
  ];
  $query = db_select(RestApiRequest::TABLE, 't')
    ->fields('t', $field_list);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get related responses.
  $query->addExpression("string_agg(CAST(res.response_id AS VARCHAR), ',')", 'response_list');
  $query->leftJoin(RestApiResponse::TABLE, 'res', 'res.request_id = t.request_id');

  // Group by ALL columns from the request table that you're selecting.
  foreach ($field_list as $field_name) {
    $query->groupBy('t.' . $field_name);
  }

  $header = [
    ['data' => t('Request Id'), 'field' => 't.request_id'],
    ['data' => t('Created'), 'field' => 't.created'],
    ['data' => t('Method'), 'field' => 't.method'],
    ['data' => t('URL'), 'field' => 't.url'],
    ['data' => t('Header'), 'field' => 't.header'],
    ['data' => t('Payload'), 'field' => 't.payload'],
    ['data' => t('Response List'), 'field' => 'response_list'],
  ];
  return Report::print([
    'name' => 'rest_api_request_list',
    'header' => $header,
    'query' => $query,
    'pager' => 'both',
  ], $reset);
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
  $url = 'admin/reports/tpps/rest_api/request/single/' . $row['request_id'];
  switch ($name) {

    case 'request_id':
      if (!empty($value)) {
        $formatted = l($formatted, 'admin/reports/tpps/rest_api/request/single/'
          . $formatted);
      }
      break;

    case 'method':
      $formatted = theme('http_method', ['method' => $formatted]);
      break;

    case 'header':
    case 'payload':
      $formatted = json_decode($value, 1);
      if (!empty($formatted)) {
        $formatted = l(t('View'), $url);
      }
      else {
        $formatted = '';
      }
      break;

    case 'created':
      if (!empty($value)) {
        $formatted = format_date($formatted, 'custom', "Y/m/d H:i:s O");
      }
      break;

    case 'response_list':
      if (!empty($value)) {
        $list = explode(',', $value);
        $items = [];
        foreach ($list as $response_id) {
          $url = 'admin/reports/tpps/rest_api/response/body/' . $response_id;
          $items[] = l($response_id, $url);
        }
        $formatted = theme('item_list', ['items' => $items]);
      }
      break;

  }
  return $formatted;
}
