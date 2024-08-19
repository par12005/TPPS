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
 * Function drupal_realpath() (which uses PHP's realpath()) can't resolve path
 * if there is a symlink in path.
 * This function works fine with symlinks.
 * For more details: https://app.clickup.com/t/86b0ztv8g.
 *
 * @param string $uri
 *   A stream wrapper URI or a filepath, possibly including one or
 *   more symbolic links.
 *
 * @return string|false
 *   The absolute local filepath (with no symbolic links), or FALSE on failure.
 */
function tpps_realpath($uri) {
  global $conf;
  $path = drupal_realpath($uri);
  // If path to special folder (like 'private://')  wasn't set then $path
  // will be non-empty and custom processing will be skipped.
  // If $uri contains symlink then drupal_realpath() will return FALSE and
  // special processing will be required.
  if ($path === FALSE) {
    $path = $uri;
    // Order is important. First must be more often used to speed-up processing.
    // Note: ofter folders overwritten in settings.php which uses $conf and
    // there is no variable in DB or it's empty.
    $folder_list = [
      'public://' => variable_get('file_public_path')
      ?? $conf['file_public_path'] ?? 'sites/default/files',
      'temporary://' => variable_get('file_temporary_path')
      ?? $conf['file_temporary_path'] ?? '/tmp',
      'private://' => variable_get('file_private_path')
      ?? $conf['file_private_path'] ?? '',
    ];
    $count = NULL;
    foreach ($folder_list as $name => $folder_path) {
      if (!$count && $folder_path) {
        $path = str_replace($name, "$folder_path/", $path, $count);
      }
    }
  }
  return $path;
}

/**
 * Shows report as a table with pager.
 *
 * WARNING:
 * Specific to reports settings are used! DON'T USE for regular table with pager.
 *
 * @param array $report
 *   Required keys:
 *   - 'report_name' - string. Machine report name (with underscores).
 *   - 'header' - array. List of the table's header labels.
 *   - 'rows' - array. Table data to be shown.
 *   Optional keys:
 *   - 'empty' - string. Localized string which will be shown when 'rows' is empty.
 *
 * @return string
 *   Returns rendered HTML table with pager.
 */
function tpps_report_show_table(array $report) {
  $report_name = $report['report_name'];
  $rows = $report['rows'] ?? [];

  $per_page = variable_get('tpps_report_items_per_page', 20);
  $current_page = pager_default_initialize(count($rows), $per_page);
  $chunks = array_chunk($rows, $per_page, TRUE);
  $pager = theme('pager', ['quantity', count($rows)]);
  return $pager . theme('table', [
    'caption' => tpps_report_get_list()[$report_name]['description'],
    'header' => $report['header'] ?? [],
    'rows' => $chunks[$current_page],
    'empty' => $report['empty'] ?? NULL,
  ]) . $pager;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Google Maps.

/**
 * Builds JS code for Google Maps JavaScript MarkerClusterer.
 *
 * The library creates and manages per-zoom-level clusters for large amounts
 * of markers. Project page: https://github.com/googlemaps/js-markerclusterer .
 *
 * Helps to show plants on Google Maps.
 *
 * @return string
 *   Returns inline JavaScript code with wrapper HTML-tag.
 */
function tpps_get_markercluster_code($use_wrapper = TRUE) {
  if (empty($map_api_key = variable_get('tpps_maps_api_key', NULL))) {
    return '';
  }
  $query = [
    'key' => $map_api_key,
    'callback' => 'initMap',
    'v' => 'weekly',
    'libraries' => 'marker',
    'loading' => 'async',
  ];
  $url = url('https://maps.googleapis.com/maps/api/js', ['query' => $query]);
  $url_type = variable_get('tpps_markercluster_url_type', 'new');
  // WARNING: Be sure to set 'async' and 'defer' HTML attributes.
  $output = '<script src="' . tpps_markercluster_get_url($url_type)
    . '"></script><script src="' . $url . '" async defer></script>';
  if ($use_wrapper) {
    $output .= '<div id="_map_wrapper"></div>';
  }
  $js_settings = [
    'markerClusterUrlType' => $url_type,
    'googleMapId' => variable_get('tpps_google_map_id'),
  ];
  drupal_add_js(['tpps' => $js_settings], 'setting');
  return $output;
}

/**
 * Gets Google Map MarkerCluster library URL.
 *
 * @param string $key
 *   Possible values are: 'new', 'old'.
 *
 * @return mixed
 *   Returns URL (string) if $key was specified.
 *   Returns array with all URLs when $key wasn't specified.
 */
function tpps_markercluster_get_url($key = '') {
  $options = [
    'old' => 'https://developers.google.com/maps/documentation/javascript'
      . '/examples/markerclusterer/markerclusterer.js',
    'new' => 'https://unpkg.com/@googlemaps/markerclusterer/dist/index.min.js',
  ];
  return $options[($key)] ?? $options;
}
