<?php

/**
 * @file
 * Shows list of studies without DOI attached.
 */

/**
 * Menu callback. Shows Rest API Response Body.
 */
function tpps_report_rest_api_response_body() {

  $id = arg(6);
  if (empty($id)) {
    $message = t('Empty Rest API Response Id.');
    drupal_set_message($message, 'error');
    drupal_goto('admin/reports/tpps/rest_api/response/list');
  }

  $output = '';
  // WARNING: Report::print() requires db_select() not db_query().
  $query = db_select(RestAPIResponse::TABLE, 't')
    ->fields('t', [])
    ->condition('id', $id);
  $query->join(RestAPIRequest::TABLE, 't2', 't2.request_id = t.request_id');
  $query->addField('t2', 'tag', 'tag');

  $result = $query->execute()->fetchAssoc();

  // Request.
  $output .= theme('item_list', [
    'title' => t('Request'),
    'items' => [
      l(
        t('View Request "@hash"', ['@hash' => $result['request_id']]),
        '/admin/reports/tpps/rest_api/request/single/' . $result['request_id']
      ),
      check_plain($result['tag']),
    ],
  ]);
  $output .= theme('item_list', [
    'title' => t('Response'),
    'items' => [
      t('Code: !value', [
        '!value' => theme('http_code', ['code' => $result['code']]),
      ]),
      t('Duration (msec): @value', ['@value' => $result['duration']]),
      t('Started: @value', [
        '@value' => format_date($result['created'], 'custom', "Y/m/d H:i:s O")
          . ' (' . $result['created'] . ')',
      ]),
      'Response Id: ' . check_plain($result['id']),
    ]
  ]);

  $output .= theme('json', [
    'title' => t('Response Body'),
    'json' => $result['body'] ?? '',
  ]);
  return $output;
}
