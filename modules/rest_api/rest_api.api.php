<?php

/**
 * @file
 * Hooks provided by the REST API module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Invoked right before a live or cached REST API request is executed.
 *
 * @param array $context
 *   Request context array containing:
 *   - 'request_id': (int|string|null) Existing request ID if known.
 *   - 'response_id': (int|string|null) Existing response ID if known.
 *   - 'hash': (string) MD5 hash of request metadata.
 *   - 'created': (int) Unix timestamp when the request was initiated.
 *   - 'method': (string) HTTP method in uppercase ('GET', 'POST', 'PUT',
 *     'DELETE').
 *   - 'url': (string) Full API endpoint URL including query parameters.
 *   - 'payload': (mixed) Request payload (JSON string or array).
 *   - 'header': (array) Normalized HTTP request headers.
 *   - 'tag': (string|null) Human-readable request tag.
 *   - 'cache': (bool) Whether caching is enabled for this request.
 *   - 'log': (bool) Whether database logging is enabled for this request.
 *
 * @return int|string|null|bool
 *   The saved or existing request_id (for 'rest_api' module implementation),
 *   NULL when logging is disabled, or FALSE on validation failure.
 */
function hook_rest_api_request_started(array $context = []) {
  return $context['request_id'] ?? NULL;
}

/**
 * Invoked immediately after a live cURL request finishes and is decoded.
 *
 * @param array $context
 *   Response and request context array containing:
 *   - 'request': (array) Request context array from prepareRequestExecution().
 *   - 'request_id': (int|string|null) Database ID of the request record.
 *   - 'code': (int) HTTP response status code.
 *   - 'created': (int) Unix timestamp when the request was initiated.
 *   - 'body': (mixed) Decoded response body (array for JSON, string for text).
 *   - 'duration': (float) Request execution time in milliseconds.
 *   - 'content_type': (string) Response Content-Type header value.
 *   - 'curl_info': (array) Metadata array returned by curl_getinfo().
 *   - 'response_id': (int|string|null) Database ID of the response if preset.
 *   - 'cache': (bool) Whether response caching is enabled for this request.
 *   - 'log': (bool) Whether database logging is enabled for this request.
 */
function hook_rest_api_response_received(array $context = []) {
}

/**
 * Invoked when a cached response is loaded from the database.
 *
 * @param array $context
 *   Context array containing:
 *   - 'request_id': (int|string) Unique request identifier.
 *   - 'response_id': (int|string) Unique response identifier loaded from DB.
 */
function hook_rest_api_response_loaded(array $context = []) {
  if (!empty($context['request_id']) && !empty($context['response_id'])) {
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
 * Alters the exception message before RestApi throws on non-2xx HTTP status.
 *
 * Called from RestApi->validateResponseStatus() when an API responds with an
 * HTTP status code outside 200..299.
 *
 * @param string $message
 *   Exception message to be altered by reference.
 * @param array $exception_context
 *   Request and response context data containing:
 *   - 'payload': (array|string) Decoded or raw request payload.
 *   - 'url': (string) Sanitized API endpoint URL.
 *   - 'method': (string) HTTP method in uppercase.
 *   - 'header': (array) Sanitized list of request headers.
 *   - 'body': (mixed) Decoded or raw API server response body.
 *   - 'response': (mixed) Alias of 'body' for backward compatibility.
 *   - 'code': (int) HTTP response status code.
 *   - 'message': (string) Original unmodified status message string.
 *   - 'curl_info': (array) Data provided by curl_getinfo().
 *   - 'cache': (bool) Whether caching was enabled for this request.
 *   - 'log': (bool) Whether logging was enabled for this request.
 *   - 'backtrace': (array) Call stack from debug_backtrace().
 */
function hook_rest_api_exception_alter(&$message, array $exception_context) {
}

/**
 * Alters response context before generating the response MD5 hash.
 *
 * Used to strip dynamic or time-based fields from the response body so
 * identical payloads are deduplicated in {rest_api_response}.
 *
 * @param array $context
 *   Response metadata array passed to RestApiResponse::generateHash().
 */
function hook_rest_api_response_hash_generation_alter(array &$context) {
  if (is_array($context['body'])) {
    unset($context['body']['response-date']);
  }
}

/**
 * @} End of "addtogroup hooks".
 */
