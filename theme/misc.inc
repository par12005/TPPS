
<?php

/**
 * @file
 * Contains common theme-functions.
 */

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
