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
