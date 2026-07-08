<?php

/**
 * @file
 * This file contains miscellaneous API functions and Drupal hooks.
 */

/**
 * Implements hook 'rest_api_response_received'.
 *
 * @param array $context
 *   Request's context.
 *   Keys:
 *   'Code' - int. HTTP code.
 *   'Message' - string. HTTP code message.
 *   'Execution Time' - float. Request execution time in milliseconds.
 *   'Request' - array. Keys are 'Method', 'URL', 'Payload', 'Headers'.
 *   'Response' - mixed. Remote server's response.
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
        '@request_id' => $constext['request_id'],
      ]
    );
  }
}

/**
 * Implements alter-hook 'rest_api_exception'.
 *
 * Called from RestApi->doRequest() when API responded with HTTP code different
 * from 200..204.
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
 *    'curl_info' - array. Data provided by curl_info().
 *    'backtrace' - array. Stack of the callers.
 *    'message' - string. Clone of the $message to always have original string.
 */
function hook_rest_api_exception($message, array $exception_context) {
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

