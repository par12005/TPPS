<?php

/**
 * @file
 * Shows list of studies without DOI attached.
 */

/**
 * Menu callback. Shows Rest API Response Body.
 */
function tpps_report_rest_api_request_single() {

  $id = arg(6);
  if (empty($id)) {
    $message = t('Empty Rest API Response Id.');
    drupal_set_message($message, 'error');
    drupal_goto('admin/reports/tpps/rest_api/response/list');
  }

  $output = '';
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
  $query->condition('t.request_id', $id);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get related responses.
  $query->addExpression("string_agg(CAST(res.response_id AS VARCHAR), ',')", 'response_list');
  $query->leftJoin(RestApiResponse::TABLE, 'res', 'res.request_id = t.request_id');

  // Group by ALL columns from the request table that you're selecting.
  foreach ($field_list as $field_name) {
    $query->groupBy('t.' . $field_name);
  }

  $result = $query->execute()->fetchAssoc();

  // Special care about some data.
  $header = json_decode($result['header'], TRUE);
  $payload = json_decode($result['payload'], TRUE);
  $response_list = $result['response_list'] ?? NULL;
  unset($result['header']);
  unset($result['payload']);
  unset($result['response_list']);

  // Plain data.
  $output .= TppsArray::print($result, t('Metadata'));

  // Response list.
  if (!empty($response_list)) {
    $list = explode(',', $response_list);
    $items = [];
    foreach ($list as $response_id) {
      $url = 'admin/reports/tpps/rest_api/response/body/' . $response_id;
      $items[] = l($response_id, $url);
    }
    $output .= theme('item_list', [
      'items' => $items,
      'title' => t('Response List'),
    ]);
  }

  if ($header) {
    $output .= TppsArray::print($header, t('Header')) . '<hr />';
  }
  if ($payload) {
    $output .= TppsArray::print($payload, t('Payload')) . '<hr />';
  }
  return $output;
}
