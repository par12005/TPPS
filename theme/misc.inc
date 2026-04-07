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
