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

/**
 * Puts message using both watchdog() and drupal_set_message().
 *
 * WARNING: Not for CLI (messages will be ignored).
 *
 * @param string $message
 *   Not localized message text.
 * @param array $params
 *   List of the tokens which will be replaces in $message.
 * @param string $type
 *   Predefined message type. Possible values are:
 *   'error', 'notice', 'info' (default), 'warning' and 'debug'
 *   Note: 'debug' messages will be shown at page only when 'Debug Mode' enabled.
 *   To enable 'Debug Mode': variable_set('tpps_debug_mode', TRUE)
 *   or use admin settings page.
 */
function tpps_message($message, array $params = [], $type = 'info') {
  if (php_sapi_name() == 'cli') {
    // Do nothing in CLI mode.
    return;
  }
  $watchdog_type = constant('WATCHDOG_' . strtoupper($type)) ?? WATCHDOG_NOTICE;
  watchdog('tpps', $message, $params, $watchdog_type);
  if ($type != 'debug' || (tpps_is_debug_mode() && $type == 'debug')) {
    $type = ($type == 'info') ? 'status' : $type;
    drupal_set_message(t($message, $params), $type);
  }
}

/**
 * Checks if curation team member or administrator.
 *
 * @return bool
 *   Returns TRUE if current user belongs to 'Curation' or 'administrator'
 *   groups and FALSE otherwise.
 */
function tpps_is_admin_or_curation() {
  global $user;
  return (
    in_array('administrator', $user->roles)
    || in_array('Curation', $user->roles)
  );
}

/**
 * Resolves the absolute filepath of a local URI or filepath.
 *
 * Function drupal_realpath() (and PHP's realpath()) can't resolve path if
 * there is a symlink in path but this function works fine with symlinks.
 *
 * @param string $uri
 *   A stream wrapper URI or a filepath, possibly including one or
 *   more symbolic links.
 *
 * @return string|false
 *   The absolute local filepath (with no symbolic links), or FALSE on failure.
 */
function tpps_realpath($uri) {
  $path = drupal_realpath($uri);
  if ($path === FALSE) {
    return str_replace('public://', drupal_realpath('public://') . '/', $uri);
  }
  return $path;
}
