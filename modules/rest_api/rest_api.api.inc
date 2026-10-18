<?php

/**
 * @file
 * This file contains miscellaneous API functions and Drupal hooks.
 */

/**
 * Implements hook 'rest_api_request_started'.
 *
 * @param array $context
 *   Request's context.
 *   Keys:
 *   'request_id' - int|null. Existing request ID if known.
 *   'hash' - string. MD5 hash of request metadata.
 *   'method' - string. HTTP method in uppercase.
 *   'url' - string. API Endpoint URL.
 *   'payload' - mixed. Request payload.
 *   'header' - array. Request headers.
 *   'tag' - string|null. Human-readable request tag.
 *   'cache' - bool. Whether caching is enabled for this request.
 *   'log' - bool. Whether DB logging is enabled for this request.
 */
function hook_rest_api_request_started(array $context = []) {
}

/**
 * Implements hook 'rest_api_response_received'.
 *
 * @param array $context
 *   Request's context.
 *   Keys:
 *   'code' - int. HTTP code.
 *   'duration' - float. Request execution time in milliseconds.
 *   'request' - array. Request metadata.
 *   'body' - mixed. Remote server's response.
 */
function hook_rest_api_response_received(array $context = []) {
}

/**
 * Implements hook 'rest_api_response_loaded'.
 *
 * @param array $context
 *   Keys: 'request_id' and 'response_id'.
 */
function hook_rest_api_response_loaded(array $context = []) {
  if (
    !empty($context['request_id'])
    && !empty($context['response_id'])
  ) {
    watchdog(
      'rest_api',
      "Loaded response '@id' for request '@request_id'.",
      [
        '@id' => $context['response_id'],
        '@request_id' => $context['request_id'],
      ]
    );
  }
}

/**
 * Implements alter-hook 'rest_api_exception_alter'.
 *
 * Called from RestApi->validateResponseStatus() when API responded with HTTP
 * code outside 200..299.
 *
 * @param string $message
 *   Exception message which could be altered.
 * @param array $exception_context
 *   Request context data. Keys are:
 *    'payload' - mixed.
 *    'url' - string. API Endpoint URL.
 *    'method' - string. HTTP-method in uppercase.
 *    'code' - int. HTTP response code.
 *    'header' - array. List of request headers. Each element is a string.
 *    'response' - mixed. API server response.
 *    'curl_info' - array. Data provided by curl_getinfo().
 *    'backtrace' - array. Stack of the callers.
 *    'message' - string. Clone of the $message to always have original string.
 */
function hook_rest_api_exception_alter(&$message, array $exception_context) {
}

/**
 * Implements alter-hook 'rest_api_response_hash_generation'.
 *
 * @param array $context
 *   Response meta data for hash generation.
 *   See RestApiResponse::generateHash().
 */
function hook_rest_api_response_hash_generation_alter(array &$context) {
  unset($context['body']['response-date']);
}
