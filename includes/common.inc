<?php

/**
 * @file
 * Helper functions use at any step of TPPS form.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/common');
 */

/**
 * Gets domain name.
 *
 * @return string
 *   Returns domain name.
 */
function tpps_get_hostname() {
  global $base_url;
  return explode('://', $base_url)[1];
}

/**
 * Checks if it's a live site.
 *
 * @return bool
 *   Returns TRUE code runs at production server and FALSE otherwise.
 */
function is_live_site() {
  // @TODO Minor. Move value of the live domain to the settings page.
  $live_domain = 'treegenesdb.org';
  return (bool) (tpps_get_hostname() == $live_domain);
}

/**
 * Dumps variable using dpm to browser.
 *
 * @param mixed $dump
 *   Variable which must be dumped.
 * @param string $message
 *   Optional helper message.
 */
function dpv($dump, $message = '') {
  dpm(print_r($dump, 1), $message ?? NULL);
}

/**
 * Replaces $old with $new in array's values and keys.
 *
 * @param array $array
 *   Array to be updated.
 * @param mixed $old
 *   Old token.
 * @param mixed $new
 *   New token.
 *
 * @return array
 *   Returns updated array.
 */
function tpps_array_replace(array $array, $old, $new) {
  // Source: tpps_change_tgdr_number();
  $state_json = json_encode($array);
  $state_json = str_replace($old, $new, $state_json);
  return json_decode($state_json, TRUE);
}

/**
 * Highlights indents with gray vertical bars.
 *
 * @param string $string
 *   String to be processed.
 *
 * @return string
 *   Returns formatted HTML string.
 */
function tpps_string_format_indent($string) {
  $line_clean = ltrim($string);
  $spaces = str_replace($line_clean, '', $string);
  return '<span style="color:silver;">'
    . str_replace(' ', '&#124;&nbsp;&nbsp;&nbsp;&nbsp;', $spaces)
    . '</span>' . $line_clean;
}

/**
 * Parses huge array to show a well-formatted dump.
 *
 * Good for huge arrays. For small use dpm() or similar.
 *
 * @param array $array
 *   Array to be dumped.
 * @param bool $sticky
 *   Should table has sticky header or not.
 *
 * @return array
 *   Returns array with keys:
 *    'plain_items' and names of sub-array items.
 */
function tpps_array_dump(array $array, $sticky = TRUE) {
  $output = [];
  // Get list of sub-arrays ($part list)
  foreach (array_keys($array) as $key) {
    if (is_array($array[$key])) {
      $part_list[$key] = count($array[$key]);
    }
    else {
      $plain[] = $key;
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Plain Items.;
  if (!empty($plain ?? NULL)) {
    ksort($plain);
    foreach ($plain as $line => $key) {
      $plain_rows[] = [
        [
          'data' => $line + 1,
          'style' => 'padding: 0 10px; font-weight: bold;',
        ],
        [
          'data' => check_plain($key),
          'style' => 'padding: 0 10px; font-weight: bold;',
        ],
        [
          'data' => check_plain(var_export($array[$key], 1)),
          'style' => 'padding: 0 10px; width:100%;',
        ],
      ];
    }
    $output['plain_items'] = [
      'title' => t('Plain Items'),
      'data' => theme('table', ['rows' => $plain_rows, 'sticky' => $sticky]),
      'count' => count($plain),
    ];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Sub-arrays.
  if (!empty($part_list ?? NULL)) {
    ksort($part_list);
    foreach ($part_list as $key => $count) {
      $rows = [];
      $state_part = $array[$key] ?? [];
      $state_array = preg_split("/\r\n|\n|\r/", var_export($state_part, 1));
      if ($state_part) {
        foreach ($state_array as $line => $content) {
          $rows[] = [
            [
              'data' => $line + 1,
              'style' => 'padding: 0 10px; font-weight: bold;',
            ],
            [
              'data' => tpps_string_format_indent($content),
              'style' => 'padding: 0 10px; width:100%;',
            ],
          ];
        }
        $output[$key] = [
          'title' => $key,
          'data' => theme('table', ['rows' => $rows, 'sticky' => $sticky]),
          'count' => count($array[$key]),
        ];
      }
    }
  }
  return $output;
}

/**
 * Gets Drupal User Acount Email-address.
 *
 * @param int $uid
 *   Drupal User Id.
 * @param bool $reset
 *   Flag to reset cache.
 *
 * @return string
 *   Returns email-address or empty string if user wasn't found.
 */
function tpps_get_user_email($uid, $reset = FALSE) {
  $key = $uid;
  $static_cache = &drupal_static(__FUNCTION__);
  if ($reset || !isset($static_cache[$key])) {
    $static_cache[$key] = (string) db_select('users', 'u')
      ->fields('u', ['mail'])
      ->condition('uid', $uid)
      ->range(0, 1)
      ->execute()
      ->fetchField();
  }
  return $static_cache[$key];
}
