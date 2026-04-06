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
    ->fields('t', [
      'id',
      'request_id',
      'code',
      'body',
    ])
    ->condition('id', $id);
  //$query->join('public.' . RestAPILog::REQUEST_TABLE, 't2', 't2.request_id = t.request_id');
  //$query->addField('t2', '.tag');

  $result = $query->execute()->fetchAssoc();

  // Request.
  $output .= theme('item_list', [
    'title' => t('Request'),
    'items' => [
      l(
        t('View Request "@hash"', ['@hash' => $result['request_id']]),
        '/admin/reports/tpps/rest_api/request/' . $result['request_id']
      ),
      'Code: ' . $result['code'] . ' ' . HttpCode::getMessage($result['code']),
      'Response Id: ' . $result['id'],
    ]
  ]);

  if ($result['code'] != HttpCode::OK) {
    dpm($result['body']);
    $output .= ' ';
  }
  else {

    //$data = json_decode($result, TRUE, 512, JSON_OBJECT_AS_ARRAY);
    $data = json_decode($result['body'], 1);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $title = t('Response Body');
    if (is_array($data)) {
      $output .= TppsArray::print($data, $title);
    }
    elseif (is_string($data)) {

// @TODO When $data contains HTML it will be shown in page and broke layout.

      $output .= '<h3>' . $title . '</h3><pre>' . var_export($data, 1) . '</pre>';
    }
    else {
      dpm(var_export($data, 1));
      $output .= 'Unknown format';
    }

  }

  return $output;
}
