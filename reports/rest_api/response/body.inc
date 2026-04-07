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
  $query = db_select('public.tpps_rest_api_response', 't')
    ->fields('t', [])
    ->condition('id', $id);
  $query->join('public.' . RestAPILog::REQUEST_TABLE, 't2', 't2.request_id = t.request_id');
  $query->addField('t2', 'tag', 'tag');

  $result = $query->execute()->fetchAssoc();


  // Request.
  $output .= theme('item_list', [
    'title' => t('Request'),
    'items' => [
      l(
        t('View Request "@hash"', ['@hash' => $result['request_id']]),
        '/admin/reports/tpps/rest_api/request/' . $result['request_id']
      ),
      check_plain($result['tag']),
    ]
  ]);
  $output .= theme('item_list', [
    'title' => t('Response'),
    'items' => [
      t('Code: !value', [
        '!value' => theme('http_code', ['code' => $result['code']]),
      ]),
      t('Duration (msec): @value', ['@value' => $result['duration']]),
      t('Started: @value', [
        '@value' => format_date($result['started'], 'custom', "Y/m/d H:i:s O")
          . ' (' . $result['started'] . ')',
      ]),
      'Response Id: ' . check_plain($result['id']),
    ]
  ]);

  $title = t('Response Body');
  $body = $result['body'] ?? '';
  $body = json_decode($body, 1);

  $output .= '<h3>' . $title . '</h3>';
  if (is_array($body)) {
    $output .= TppsArray::print($body);
  }
  elseif (is_string($body)) {
    if ($body !== strip_tags($body)) {
      // HTML.
      $output .= "<iframe srcdoc='" . $body . "' " . '
        style="width: 100%; height: 500px; border: 2px solid silver;">
        </iframe>';
    }
    else {

// @TODO Responses from dryad server was twice encoded into JSON. Check why it happens.
      $body2 = json_decode($body, 1);
      if (is_array($body2)) {
        $output .= TppsArray::print($body2);
      }
      else {
        $output .= '<pre style="border: 2px solid red; padding: 20px">'
          . var_export($body, 1) . '</pre>';
      }
    }
  }
  else {
    dpm(var_export($body, 1));
    $output .= 'Unknown format';
  }

  return $output;
}
