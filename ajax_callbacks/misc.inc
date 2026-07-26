<?php

/**
 * @file
 * Misc. AJAX-callbacks.
 *
 * See /tpps.ajax.inc.
 *
 * How to add new AJAX-callback?
 * 1. Add new function with 'tpps_ajax_' prefix.
 * 2. Use function name withtout prefix 'tpps_ajax_' at client side.
 *
 * Example:
 * function: tpps_ajax_save_google_scholar_serp()
 * URL: /ajax/tpps/save_google_scholar_serp
 */

/**
 * AJAX callback for 'Remember Fieldset Collapsed State' feature.
 *
 * Stores state of given fieldset.
 *
 * @param array $data
 *   POST data.
 *
 * @return bool
 *   Returns TRUE in case of success and FALSE otherwise.
 */
function tpps_ajax_remember_fieldset_collapsed_state(array $data) {
  if (
    empty($data['name'])
    && empty($data['form_id'])
    && !isset($data['value'])
  ) {
    return FALSE;
  }
  $_SESSION[TPPS_RFCS_KEY][$data['form_id']][$data['name']] = (int) $data['value'];
  return TRUE;
}
