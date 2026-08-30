<?php

/**
 * @file
 * Shows list of the all Rest API Requests.
 */

/**
 * Menu callback. Shows list of the all Rest API Requests.
 */
function tpps_report_rest_api_response_list() {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $query = db_select(RestApiResponse::TABLE, 't')
    ->fields('t', [
      'response_id',
      'request_id',
      'created',
      'code',
      'duration',
      'body',
    ]);
  $query->join(RestApiRequest::TABLE, 't2', 't2.request_id = t.request_id');
  $query->addField('t2', 'tag');

  $header = [
    ['data' => t('Response Id'), 'field' => 't.response_id', 'sort' => 'desc'],
    ['data' => t('Request Id'), 'field' => 't.request_id'],
    ['data' => t('Started'), 'field' => 't.created'],
    ['data' => t('HTTP Code'), 'field' => 't.code'],
    ['data' => t('Duration, msec'), 'field' => 't.duration'],
    ['data' => t('Body'), 'field' => 't.body'],
    ['data' => t('Tag'), 'field' => 't.tag'],
  ];
  return Report::print([
    'name' => 'rest_api_response_list',
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
function tpps_report_rest_api_response_list_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'response_id':
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
        $formatted = l($formatted, 'admin/reports/tpps/rest_api/request/single/'
          . $formatted);
      }
      break;

    case 'body':
      if (!empty($value)) {
        $url = 'admin/reports/tpps/rest_api/response/body/' . $row['response_id'];
        $formatted = substr($formatted, 0, 70)
          . '... ' . l(t('View more'), $url);
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

    case 'code':
      $formatted = theme('http_code', ['code' => $formatted]);
      break;

  }
  return $formatted;
}
