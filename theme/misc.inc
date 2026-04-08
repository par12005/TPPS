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
 *   'json' string
 *     Tequired. JSON string to be shown.
 *   'title' string
 *     Optional. Name of the data which will be shown as a title..
 *
 * @return string
 *   Returns rendered HTML.
 */
function theme_json(array $variables) {
  $output = '';
  $json = $variables['json'] ?? NULL;
  if (!empty($json)) {
    if (!empty($variables['title'])) {
      $output .= '<h3>' . $variables['title'] . '</h3>';
    }

    $data = json_decode($json, 1);
    if (is_array($data)) {
      $output .= TppsArray::print($data);
    }
    elseif (is_string($data)) {
      if (
        (strpos($data, '<html') !== FALSE)
        && $data !== strip_tags($data)
      ) {
        // HTML.
        $output .= "<iframe srcdoc='" . $data . "' " . '
          style="width: 100%; height: 500px; border: 2px solid silver;">
          </iframe>';
      }
      else {
        // @TODO Responses from dryad server was twice encoded into JSON.
        // Check why this happens.
        $data2 = json_decode($data, 1);
        if (is_array($data2)) {
          $output .= TppsArray::print($data2);
        }
        else {
          $output .= '<pre style="border: 2px solid red; padding: 20px">'
            . var_export($data, 1) . '</pre>';
        }
      }
    }
    else {
      dpm(var_export($data, 1));
      $output .= 'Unknown format';
    }

  }
  return $output;
}
