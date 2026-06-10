<?php

/**
 * @file
 * Contains common theme-functions.
 */

/**
 * Adds color to the HTTP Method.
 *
 * @param array $variables
 *   Data for theming.
 *   'method' int
 *     Required. HTTP Method.
 *
 * @return string
 *   Returns rendered HTML.
 */
function theme_http_method(array $variables) {
  $method = $variables['method'] ?? NULL;
  $output = '';
  if (!empty($method)) {
    $method = strtoupper($method);
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
    $output = $method;
    if ($color) {
      $output = '<span style="font-weight: bold; color:' . $color . ';">'
        . $method . '</span>';
    }
  }
  return $output;
}

/**
 * Adds color and related message text to the HTTP Code.
 *
 * @param array $variables
 *   Data for theming.
 *   'code' int
 *     Required. HTTP Code.
 *
 * @return string
 *   Returns rendered HTML.
 */
function theme_http_code(array $variables) {
  $code = $variables['code'] ?? NULL;
  $output = '';
  if (!empty($code)) {
    $color = ($code == HttpCode::OK) ? 'green' : 'red';
    $output = '<span style="color:' . $color . '"><strong>'
      . $code . '</strong> ' . HttpCode::getMessage($code) . '</span>';
  }
  return $output;
}

/**
 * Shows JSON as a structured list with the title.
 *
 * @param array $variables
 *   Data for theming.
 *   'body' string
 *     Required. Response body.
 *   'title' string
 *     Optional. Name of the data which will be shown as a title..
 *   'content_type' string
 *     Optional. Content type of the body.
 *     Posible values: 'json', 'xml' or empty.
 *
 * @return string
 *   Returns rendered HTML.
 */
function theme_response_body(array $variables) {
  $output = '';
  if (!empty($variables['title'])) {
    $output .= '<h3>' . $variables['title'] . '</h3>';
  }
  if (!isset($variables['body'])) {
    // error.
  }

  $content_type = $variables['content_type'] ?? '';

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (strpos($content_type, 'application/xml') !== FALSE) {
    // XML.
    $body = RestAPIResponse::restoreXml($variables['body']);
    $output .= '<pre style="border: 2px solid silver; padding: 20px">'
     . htmlspecialchars(trim($body)) . '</pre>';
    return $output;
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  elseif (strpos($content_type, 'application/json') !== FALSE) {
    if (strpos($variables['body'], '"{\"') !== FALSE) {
      // Responses from dryad server was twice encoded into JSON by excident.
      // This bug was already fixed.
      $body = json_decode($variables['body'], 1);
    }
    else {
      $body = $variables['body'];
    }
    // JSON.
    if (is_array($body = json_decode($body, 1))) {
      // Some errors could be just strings so we show only arrays.
      return $output . TppsArray::print($body);
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Other (neither JSON, nor XML).
  // Note: decode to restore escaped tags.
  $body = json_decode($variables['body'], 1);

  if ((strpos($body, '<html') !== FALSE) && $body !== strip_tags($body)) {
    // HTML.
    $output .= "<iframe srcdoc='" . $body . "' " . '
      style="width: 100%; height: 500px; border: 2px solid red;">
      </iframe>';
  }
  else {
    $output .= '<pre style="border: 2px solid green; padding: 20px">'
      . check_plain(print_r($variables['body'], 1)) . '</pre>';
  }
  return $output;
}
