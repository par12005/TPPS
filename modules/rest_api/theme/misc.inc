<?php

/**
 * @file
 * Contains common theme functions for the REST API module.
 */

/**
 * Adds color to the HTTP Method.
 *
 * @param array $variables
 *   Data for theming:
 *   - 'method': (string) Required. HTTP Method.
 *
 * @return string
 *   Returns rendered HTML.
 */
function theme_http_method(array $variables) {
  $method = $variables['method'] ?? NULL;
  $output = '';
  if (!empty($method)) {
    $method = strtoupper($method);
    $color = NULL;
    switch ($method) {
      case 'GET':
        $color = 'green';
        break;

      case 'DELETE':
        $color = 'red';
        break;

      case 'POST':
        $color = 'blue';
        break;

      case 'PUT':
        $color = 'orange';
        break;
    }
    $output = check_plain($method);
    if ($color) {
      $output = '<span style="font-weight: bold; color:' . $color . ';">'
        . check_plain($method) . '</span>';
    }
  }
  return $output;
}

/**
 * Adds color and related message text to the HTTP Code.
 *
 * @param array $variables
 *   Data for theming:
 *   - 'code': (int) Required. HTTP Code.
 *
 * @return string
 *   Returns rendered HTML.
 */
function theme_http_code(array $variables) {
  $code = $variables['code'] ?? NULL;
  $output = '';
  if (!empty($code)) {
    $color = ($code >= 200 && $code < 300) ? 'green' : 'red';
    $msg = check_plain(HttpCode::getMessage($code));
    $output = '<span style="color:' . $color . '"><strong>'
      . (int) $code . '</strong> ' . $msg . '</span>';
  }
  return $output;
}

/**
 * Shows response body as formatted XML, JSON, HTML iframe, or plain text.
 *
 * @param array $variables
 *   Data for theming:
 *   - 'body': (mixed) Required. Response body.
 *   - 'title': (string) Optional. Title heading to display above the body.
 *   - 'content_type': (string) Optional. Content type of the body.
 *
 * @return string
 *   Returns rendered HTML.
 */
function theme_response_body(array $variables) {
  $output = '';
  if (!empty($variables['title'])) {
    $output .= '<h3>' . check_plain($variables['title']) . '</h3>';
  }
  if (!isset($variables['body'])) {
    return $output;
  }

  $content_type = $variables['content_type'] ?? '';
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (
    stripos($content_type, 'application/xml') !== FALSE
    || stripos($content_type, 'text/xml') !== FALSE
  ) {
    // XML.
    $body = $variables['body'];
    if (class_exists('NcbiTaxonomy')) {
      $body = NcbiTaxonomy::printXml($body);
    }
    $output .= '<pre style="border: 2px solid silver; padding: 20px">'
      . htmlspecialchars(trim((string) $body), ENT_QUOTES, 'UTF-8') . '</pre>';
    return $output;
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  elseif (stripos($content_type, 'application/json') !== FALSE) {
    if (
      is_string($variables['body'])
      && strpos($variables['body'], '"{\"') !== FALSE
    ) {
      // Handle double-encoded JSON strings.
      $body = json_decode($variables['body'], TRUE);
    }
    else {
      $body = $variables['body'];
    }
    // JSON.
    $decoded = is_string($body) ? json_decode($body, TRUE) : $body;
    if (is_array($decoded)) {
      if (class_exists('TppsArray')) {
        return $output . TppsArray::print($decoded);
      }
      $pretty_json = json_encode(
        $decoded,
        JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES
      );
      $output .= '<pre style="border: 2px solid silver; padding: 20px">'
        . check_plain($pretty_json) . '</pre>';
      return $output;
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Other (neither JSON, nor XML).
  $raw_body = is_string($variables['body'])
    ? $variables['body']
    : print_r($variables['body'], TRUE);
  $decoded = json_decode($raw_body, TRUE);
  $body = is_string($decoded) ? $decoded : $raw_body;

  if ((stripos($body, '<html') !== FALSE) && $body !== strip_tags($body)) {
    // HTML rendered inside a sandboxed iframe with escaped srcdoc.
    $escaped = htmlspecialchars($body, ENT_QUOTES, 'UTF-8');
    $output .= '<iframe sandbox="" srcdoc="' . $escaped . '" '
      . 'style="width: 100%; height: 500px; border: 2px solid red;"></iframe>';
  }
  else {
    $output .= '<pre style="border: 2px solid green; padding: 20px">'
      . check_plain(print_r($variables['body'], TRUE)) . '</pre>';
  }
  return $output;
}

/**
 * Link to single request's report.
 *
 * @param array $variables
 *   Data for theming:
 *   - 'request_id': (string|int) Request Id.
 *   - 'title': (string) Optional link text.
 *
 * @return string
 *   Returns rendered HTML link (or escaped text if route is not registered).
 */
function theme_request_report_link(array $variables) {
  $request_id = trim((string) ($variables['request_id'] ?? ''));
  if ($request_id === '') {
    return '';
  }
  $url = 'admin/reports/tpps/rest_api/request/single/' . $request_id;
  $title = (string) ($variables['title'] ?? $request_id);
  if (function_exists('drupal_valid_path') && drupal_valid_path($url)) {
    return l($title, $url);
  }
  return check_plain($title);
}

/**
 * Link to response body report.
 *
 * @param array $variables
 *   Data for theming:
 *   - 'response_id': (string|int) Response Id.
 *   - 'title': (string) Optional link text.
 *
 * @return string
 *   Returns rendered HTML link (or escaped text if route is not registered).
 */
function theme_response_report_link(array $variables) {
  $response_id = trim((string) ($variables['response_id'] ?? ''));
  if ($response_id === '') {
    return '';
  }
  $url = 'admin/reports/tpps/rest_api/response/body/' . $response_id;
  $title = (string) ($variables['title'] ?? $response_id);
  if (function_exists('drupal_valid_path') && drupal_valid_path($url)) {
    return l($title, $url);
  }
  return check_plain($title);
}
