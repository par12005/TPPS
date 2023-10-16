<?php

/**
 * @file
 * Defines functions which are common and shared by many features.
 */

/**
 * Gets number of entities (like studies, species) in database.
 *
 * Note: Custom caching bin is used. To reset: drush cc tpps.
 * Used in tpps_details() to show static message at the top of page.
 *
 * @param mixed $column
 *   Columns of 'chado.plusgeno_view' table to count unique items in.
 *   Possible values are:
 *   - 'accession' (to count studies),
 *   - 'species' (to count species).
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 *
 * @return mixed
 *   Returns number of entities (studies/species) in database.
 *   Returns FALSE if wrong column to count was used.
 */
function tpps_count_entities($column, $reset = FALSE) {
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $allowed_columns = ['accession', 'species'];
  if (!in_array($column, $allowed_columns)) {
    $message = 'Wrong column @name used in @function';
    $params = ['@function' => __FUNCTION__, '@name' => $type];
    drupal_set_message($message, $params, 'error');
    watchdog('tpps', $message, $params, WATCHDOG_ERROR);
    return FALSE;
  }
  if ($reset || empty($cache) || !isset($cache->data[$column])) {
    $query = db_select('chado.plusgeno_view', 'pg');
    // @TODO Minor. Use ->distinct() and count items in Drupal way to avoid
    // expressions.
    $query->addExpression('count(distinct(' . $column . '))', 'count');
    $cache->data[$column] = (int) $query->execute()->fetchObject()->count;
    cache_set($cid, $cache->data, $cache_bin);
  }
  return $cache->data[$column] ?? NULL;
}

/**
 * Adds CSS and JS files to page.
 *
 * We add both CSS and JS even if only one is required because it's better
 * for caching system to have less unique pages and more pages with the
 * same set of attached files.
 *
 * CSS/JS files naming rules:
 *   '/js/tpps_' . $page . '.js'
 *   '/css/tpps_' . $page . '.css'
 *
 * WARNING: If file not exist it wil not break page load.
 *
 * How to use?
 *
 * To add only TPPS Page main CSS/JS files:
 *   tpps_add_css_js($form);
 *
 * To add CSS and JS for arbitrary page:
 *   tpps_add_css_js($form, 'details');
 *
 * To add only TPPS Page specific CSS/JS files:
 *   tpps_add_css_js($form, (int) $page_number);
 *   tpps_add_css_js($form, 'page_' . (int) $page_number);
 *
 * To add both (main and page specific) CSS/JS files:
 *   tpps_add_css_js($form);
 *   tpps_add_css_js($form, (int) $page_number);
 *   tpps_add_css_js($form, 'page_' . (int) $page_number);
 *
 * To add CSS and JS using drupal_add_css/js():
 *   tpps_add_css_js();
 *
 * To force using Form API instead of use drupal_add_css/js():
 *   $form = $form ?? [];
 *   tpps_add_css_js($form);
 *   This could be useful if $form not yet defined at the beginning of the
 *   form generation callback.
 *
 * @param array $form
 *   Drupal Form API form array.
 * @param mixed $page
 *   For TPPS Form Page it will be an integer page number.
 *   Constants like TPPS_PAGE_1 and etc could be used to improve code readablility.
 *   Also full name could be used instead 'page_1'.
 *   For other pages name (string) could be used. For example 'details'.
 */
function tpps_add_css_js(array &$form = NULL, $page = NULL) {
  // CSS Theme.
  $theme_path = TPPS_MODULE_PATH . '/css/themes/'
    . variable_get('tpps_theme', 'default') . '.css';
  if ($form === NULL) {
    drupal_add_css(TPPS_MODULE_PATH . TPPS_CSS_PATH);
    drupal_add_js(TPPS_MODULE_PATH . TPPS_JS_PATH);
    drupal_add_css($theme_path);
  }
  else {
    if ($page) {
      if (is_numeric($page) && $page > 0) {
        // Add TPPS Form Page specific files.
        $page = 'page_' . $page;
      }
      $form['#attached']['js'][] = [
        'type' => 'file',
        'scope' => 'footer',
        'group' => JS_THEME,
        'data' => TPPS_MODULE_PATH . '/js/tpps_' . $page . '.js',
      ];
      $form['#attached']['css'][] = TPPS_MODULE_PATH . '/css/tpps_' . $page . '.css';
    }
    else {
      // Add main files.
      $form['#attached']['js'][] = [
        'type' => 'file',
        'scope' => 'footer',
        'group' => JS_THEME,
        'data' => TPPS_MODULE_PATH . TPPS_JS_PATH,
      ];
      $form['#attached']['css'][] = TPPS_MODULE_PATH . TPPS_CSS_PATH;
      $form['#attached']['css'][] = $theme_path;
    }
  }
}

