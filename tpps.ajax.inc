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
function tpps_ajax_callback($callback = '', array $payload = NULL) {
  $callback = check_plain($callback);
  $function = 'tpps_ajax_' . $callback;

  // If we never succed we fail.
  $response['success'] = FALSE;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get payload.
  $content_type = $_SERVER['CONTENT_TYPE'] ?? NULL;
  if (!empty($content_type) && strpos($content_type, 'application/json') !== FALSE) {
    // This is a JSON request.
    $content = file_get_contents('php://input');
    if (empty($content)) {
      $payload = [];
    }
    else {
      $payload = json_decode($content, TRUE);
    }
  }
  else {
    $payload = $payload ?? $_POST ?? [];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // @TODO Since URL is case insensitive and to avoid errors it's better to
  // use payload to send callback.
  if (!empty($payload['callback']) && $payload['callback'] != $callback) {
    $callback = $payload['callback'];
  }
  $is_method = method_exists('TppsAjax', $callback);

  // New callbacks.
  if ($is_method || function_exists($function)) {
    // Method of the 'TppsAjax' class.
    if ($is_method) {
      $result = call_user_func(['TppsAjax', $callback], $payload);
    }
    // New callback-function.
    elseif (function_exists($function)) {
      $result = call_user_func($function, $payload);
    }

    // Process result.
    if (empty($result)) {
      $response['errors'][] = t('Empty response from AJAX callback.');
    }
    else {
      if (!is_bool($result)) {
        $response = $result;
      }
      $response['success'] = TRUE;
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Outdated callbacks.
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
      if (!is_bool($result)) {
        $response = $result;
      }
      $response['success'] = TRUE;
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Unknown callback type.
  else {
    $response['errors'][] = t('Callback not found.');
  }
  return $response;
}
