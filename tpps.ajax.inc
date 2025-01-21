<?php

/**
 * @file
 * Standard AJAX-requests processing.
 *
 * WARNING: Only POST. See hook_menu() for GET.
 */

// Local AJAX-callbacks.
module_load_include('inc', 'tpps', 'ajax_callbacks/misc');
module_load_include('inc', 'tpps', 'ajax_callbacks/study_details');

/**
 * Main AJAX callback which calls other functions.
 *
 * Use POST to send data to callback.
 * Use GET to just get response.
 * Note: To send data using GET you need to implement custom processing of
 * data from URL in callback and those data will appear in server logs.
 *
 * Callback must return:
 *  in case of errors: ['errors' => ['list', 'of', 'errors']],
 *  in case of success: ['success' => $data],
 *
 * @param string $callback
 *   Part or full function name to be used as a callback.
 *   Priorities:
 *   1. Part of function name (without 'tpps_ajax_' prefix).
 *   2. Name of the existing function. Non-empty response treated as success
 *      in this case.
 *   List of functions for search purpose:
 *   - tpps_ncbi_get_taxon_id().
 *
 * @return array
 *   Returns data ready to be sent to browser.
 */
function tpps_ajax_callback($callback = '') {
  $function = 'tpps_ajax_' . check_plain($callback);
  if (function_exists($function)) {
    $response = call_user_func($function, $_POST);
    if (empty($response)) {
      $response['errors'][] = t('Empty response from AJAX callback.');
    }
  }
  elseif (function_exists($callback)) {
    // Note order of elements in $_POST probably will be the same as
    // in AJAX-request's 'data' element and callback could get more then
    // 1 argument in correct order.
    $response = call_user_func($callback, $_POST);
    if (empty($response)) {
      $response['errors'][] = t('Empty response from function @callback().', ['@callback' => $callback]);
      $response['errors'][] = print_r(array_values($_POST));
    }
    else {
      $response['success'] = TRUE;
    }
  }
  else {
    $response['errors'][] = t('Callback not found.');
  }
  // If we never succed we fail.
  $response['success'] = $response['success'] ?? FALSE;
  return $response;
}
