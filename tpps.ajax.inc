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
 * @param array $payload
 *   Optional payload. Could be used for testing to emulate POST data.
 *
 * @return array
 *   Returns data ready to be sent to browser.
 *   Keys are:
 *   - 'success' - bool. TRUE when request proccessed sucessfully.
 *   - 'errors' - array. Optional. List of localized messages.
 *   - 'warnings' - array. Optional. List of localized messages.
 *   - Other data which could be proccessed by JS code in browser.
 */
function tpps_ajax_callback($callback = '', $payload = NULL) {
  $function = 'tpps_ajax_' . check_plain($callback);
  // If we never succed we fail.
  $response['success'] = FALSE;
  $payload = $payload ?? $_POST;
  if (function_exists($function)) {
    $result = call_user_func($function, $payload);
    if (empty($result)) {
      $response['errors'][] = t('Empty response from AJAX callback.');
    }
    else {
      $response = $result;
      $response['success'] = TRUE;
    }
  }
  elseif (function_exists($callback)) {
    // Note order of elements in $_POST probably will be the same as
    // in AJAX-request's 'data' element and callback could get more then
    // 1 argument in correct order.
    $result = call_user_func($callback, $payload);
    if (empty($result)) {
      $response['errors'][] = t('Empty response from function @callback().', ['@callback' => $callback]);
      $response['errors'][] = print_r(array_values($payload));
    }
    else {
      $response = $result;
      $response['success'] = TRUE;
    }
  }
  else {
    $response['errors'][] = t('Callback not found.');
  }
  return $response;
}
