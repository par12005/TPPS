<?php

/**
 * @file
 * Contains 'RestAPI' class.
 *
 * Usage:
 *  RestAPI::post($url, $data);
 *  RestAPI::delete($url);
 *
 *  @todo Minor. Do not use dpm().
 */

/**
 * Performs Rest API requiests using curl.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestAPI {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Static class constants.

  /**
   * Is Debug Mode enabled.
   *
   * When TRUE then more debug messages will be shown.
   *
   * @return bool
   *   Returns TRUE when 'Debug Mode' for 'RestAPI' class is enabled.
   *   Returns FALSE otherwise.
   */
  public static function isDebugMode() {
    return variable_get('tpps_rest_api_debug_mode', TRUE);
  }

  /**
   * Gets timeout value.
   *
   * The maximum number of seconds to allow cURL functions to execute.
   * To set in milliseconds use: CURLOPT_TIMEOUT_MS.
   * https://curl.se/libcurl/c/CURLOPT_TIMEOUT.html.
   *
   * @return int
   *   Returns value of the timeout in milliseconds.
   */
  public static function getTimeout() {
    return variable_get('tpps_rest_api_timeout', 30);
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Methods.

  /**
   * Decode JSON and handles invalid JSON.
   *
   * Idea: https://stackoverflow.com/a/44448921/1041470.
   * See: https://www.php.net/manual/en/class.jsonexception.php.
   * https://stackoverflow.com/questions/31578927.
   *
   * @param string $json
   *   JSON string.
   *
   * @return array
   *   Returns decoded data.
   */
  public static function decodeJSON($json) {
    $data = json_decode((string) $json, $associative = TRUE);
    // @TODO Minor. Use JSON_THROW_ON_ERROR (PHP 7.3+) if possible.
    if (JSON_ERROR_NONE !== json_last_error()) {
      $message = 'Unable to parse response body into JSON. ' . json_last_error();
      throw new RuntimeException($message);
    }
    return $data === NULL ? [] : $data;
  }

  /**
   * Makes POST-request.
   *
   * Could be used to update/create entity.
   *
   * @param string $url
   *   API Endpoint URL.
   * @param mixed $data
   *   Data for request. Could be string or JSON-string.
   * @param array $header
   *   HTTP-headers.
   *
   * @return mixed
   *   Returns API response.
   */
  public static function post($url, $data = [], array $header = []) {
    // @TODO Minor. Implement more complex check. Regexp?
    if (in_array('Content-Type: application/json', $header)) {
      $data = json_encode($data);
    }
    if (empty($header)) {
      $header = [
        'Content-Type: application/json',
      ];
      $data = json_encode($data);
    }
    if (static::isDebugMode()) {
      dpm($url, __METHOD__);
      dpm($data, __METHOD__);
      dpm($header, __METHOD__);
    }
    return self::doRequest('POST', $url, $data, $header);
  }

  /**
   * Makes GET-request.
   *
   * Could be used to get lists or load entity.
   *
   * @param string $url
   *   API Endpoint URL.
   * @param array $data
   *   Data for request.
   *
   * @return mixed
   *   Returns API response.
   */
  public static function get($url, array $data = []) {
    return self::doRequest('GET', $url, $data);
  }

  /**
   * Makes PUT-request.
   *
   * Could be used to sort entities.
   *
   * @param string $url
   *   API Endpoint URL.
   * @param mixed $data
   *   Data for request.
   * @param array $header
   *   HTTP-headers.
   *
   * @return mixed
   *   Returns API response.
   */
  public static function put($url, $data = [], array $header = []) {
    // @TODO Minor. Implement more complex check. Regexp?
    if (in_array('Content-Type: application/json', $header)) {
      $data = json_encode($data);
    }
    if (empty($header)) {
      $header = [
        'Content-Type: application/json',
      ];
      $data = json_encode($data);
    }
    return self::doRequest('PUT', $url, $data, $header);
  }

  /**
   * Makes DELETE-request.
   *
   * Could be used to sort entities.
   *
   * @param string $url
   *   API Endpoint URL.
   * @param array $data
   *   Data for request.
   *
   * @return mixed
   *   Returns API response.
   */
  public static function delete($url, array $data = []) {
    return self::doRequest('DELETE', $url, $data);
  }

  /**
   * Gets message by HTTP-response code.
   *
   * @param int $code
   *   HTTP Response code.
   *
   * @return mixed
   *   Returns list of all codes or string with a human readable response code
   *   message.
   */
  public static function getHttpCodeMessage($code = NULL) {
    /**
     * Content from http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
     *
     * You may also want a list of unofficial codes:
     *
     *  103 => 'Checkpoint',
     *  218 => 'This is fine', // Apache Web Server
     *  419 => 'Page Expired', // Laravel Framework
     *  420 => 'Method Failure', // Spring Framework
     *  420 => 'Enhance Your Calm', // Twitter
     *  430 => 'Request Header Fields Too Large', // Shopify
     *  450 => 'Blocked by Windows Parental Controls', // Microsoft
     *  498 => 'Invalid Token', // Esri
     *  499 => 'Token Required', // Esri
     *  509 => 'Bandwidth Limit Exceeded', // Apache Web Server/cPanel
     *  526 => 'Invalid SSL Certificate', // Cloudflare and Cloud Foundry's gorouter
     *  529 => 'Site is overloaded', // Qualys in the SSLLabs
     *  530 => 'Site is frozen', // Pantheon web platform
     *  598 => 'Network read timeout error', // Informal convention
     *  440 => 'Login Time-out', // IIS
     *  449 => 'Retry With', // IIS
     *  451 => 'Redirect', // IIS
     *  444 => 'No Response', // nginx
     *  494 => 'Request header too large', // nginx
     *  495 => 'SSL Certificate Error', // nginx
     *  496 => 'SSL Certificate Required', // nginx
     *  497 => 'HTTP Request Sent to HTTPS Port', // nginx
     *  499 => 'Client Closed Request', // nginx
     *  520 => 'Web Server Returned an Unknown Error', // Cloudflare
     *  521 => 'Web Server Is Down', // Cloudflare
     *  522 => 'Connection Timed Out', // Cloudflare
     *  523 => 'Origin Is Unreachable', // Cloudflare
     *  524 => 'A Timeout Occurred', // Cloudflare
     *  525 => 'SSL Handshake Failed', // Cloudflare
     *  526 => 'Invalid SSL Certificate', // Cloudflare
     *  527 => 'Railgun Error', // Cloudflare
     */
    $http_code_list = [
      100 => 'Continue',
      101 => 'Switching Protocols',
      102 => 'Processing', // WebDAV; RFC 2518
      103 => 'Early Hints', // RFC 8297
      200 => 'OK',
      201 => 'Created',
      202 => 'Accepted',
      203 => 'Non-Authoritative Information', // since HTTP/1.1
      204 => 'No Content',
      205 => 'Reset Content',
      206 => 'Partial Content', // RFC 7233
      207 => 'Multi-Status', // WebDAV; RFC 4918
      208 => 'Already Reported', // WebDAV; RFC 5842
      226 => 'IM Used', // RFC 3229
      300 => 'Multiple Choices',
      301 => 'Moved Permanently',
      302 => 'Found', // Previously "Moved temporarily"
      303 => 'See Other', // since HTTP/1.1
      304 => 'Not Modified', // RFC 7232
      305 => 'Use Proxy', // since HTTP/1.1
      306 => 'Switch Proxy',
      307 => 'Temporary Redirect', // since HTTP/1.1
      308 => 'Permanent Redirect', // RFC 7538
      400 => 'Bad Request',
      401 => 'Unauthorized', // RFC 7235
      402 => 'Payment Required',
      403 => 'Forbidden',
      404 => 'Not Found',
      405 => 'Method Not Allowed',
      406 => 'Not Acceptable',
      407 => 'Proxy Authentication Required', // RFC 7235
      408 => 'Request Timeout',
      409 => 'Conflict',
      410 => 'Gone',
      411 => 'Length Required',
      412 => 'Precondition Failed', // RFC 7232
      413 => 'Payload Too Large', // RFC 7231
      414 => 'URI Too Long', // RFC 7231
      415 => 'Unsupported Media Type', // RFC 7231
      416 => 'Range Not Satisfiable', // RFC 7233
      417 => 'Expectation Failed',
      418 => 'I\'m a teapot', // RFC 2324, RFC 7168
      421 => 'Misdirected Request', // RFC 7540
      422 => 'Unprocessable Entity', // WebDAV; RFC 4918
      423 => 'Locked', // WebDAV; RFC 4918
      424 => 'Failed Dependency', // WebDAV; RFC 4918
      425 => 'Too Early', // RFC 8470
      426 => 'Upgrade Required',
      428 => 'Precondition Required', // RFC 6585
      429 => 'Too Many Requests', // RFC 6585
      431 => 'Request Header Fields Too Large', // RFC 6585
      451 => 'Unavailable For Legal Reasons', // RFC 7725
      500 => 'Internal Server Error',
      501 => 'Not Implemented',
      502 => 'Bad Gateway',
      503 => 'Service Unavailable',
      504 => 'Gateway Timeout',
      505 => 'HTTP Version Not Supported',
      506 => 'Variant Also Negotiates', // RFC 2295
      507 => 'Insufficient Storage', // WebDAV; RFC 4918
      508 => 'Loop Detected', // WebDAV; RFC 5842
      510 => 'Not Extended', // RFC 2774
      511 => 'Network Authentication Required', // RFC 6585
    ];

    if (!is_null($code)) {
      if ($code === 0) {
        return 'Curl timeout reached.';
      }
      return $http_code_list[$code] ?? 'Unknown';
    }
    return $http_code_list;
  }

  /**
   * Performs REST API calls.
   *
   * @param string $method
   *   HTTP method. Possible values are: 'POST', 'GET', 'PUT', 'DELETE'.
   * @param string $url
   *   API Endpoint URL.
   * @param mixed $data
   *   Payload. Required for POST-requests (payload).
   *   Could be an array for GET-requests.
   *   To send JSON use header: 'Content-Type: application/json'.
   * @param array $header
   *   HTTP-headers. Required for POST-requests.
   *
   * @return mixed
   *   Returns API response.
   *
   * @throws \Exception
   *   Zenodo Deposit was not found.
   *
   * @todo Convert to private or protected method if it's possible for static.
   */
  public static function doRequest($method, $url, $data = [], array $header = []) {
    // To measure execution time of the request.
    $execution_time_start = microtime(1);
    $method = strtoupper($method);
    $curl = curl_init();
    // Why not use CURLOPT_POS: https://stackoverflow.com/a/74812403
    curl_setopt($curl, CURLOPT_CUSTOMREQUEST, $method);
    curl_setopt($curl, CURLOPT_HTTPHEADER, $header);

    if ($data) {
      if (in_array($method, ['POST', 'DELETE', 'PUT'])) {
        curl_setopt($curl, CURLOPT_SAFE_UPLOAD, TRUE);
        // Different processing of array and string:
        // https://stackoverflow.com/questions/5224790
        curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
      }
      else {
        // GET.
        $url = url($url, [
          'query' => $data,
          'https' => TRUE,
          'external' => TRUE,
        ]);
      }
    }

    // OPTIONS:
    // See http://www.php.net/manual/en/function.curl-setopt.php
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    // The maximum number of seconds for curl operation.
    curl_setopt($curl, CURLOPT_TIMEOUT, static::getTimeout());

    // EXECUTE:
    $response = curl_exec($curl);
    $info = curl_getinfo($curl);
    curl_close($curl);
    $execution_time = (microtime(1) - $execution_time_start) * 1000;

    // Allow other modules to react on request execution time.
    if (function_exists('module_invoke_all')) {
      module_invoke_all(
        'rest_api_request_execution_time',
        'RestAPI ' . $method . '-request',
        $execution_time
      );
    }

    if ($info['content_type'] == 'application/json') {
      if (!empty($response)) {
        // PHP's json_decode() could throw an exception of invalid JSON.
        // @todo Minor. Use try and catch block.
        $response = static::decodeJSON($response);
      }
      else {
        $response = NULL;
      }
    }
    // Validate HTTP request code.
    $code = $info['http_code'];
    $message = static::getHttpCodeMessage($code);
    if ($code == 0 && empty($response)) {
      throw new Exception($message);
    }
    if ($code >= 200  && $code <= 204) {
      // https://developers.zenodo.org/?shell#http-status-codes
      return $response;
    }
    else {
      $exception_data = [
        'payload' => empty($data) ? [] : json_decode($data),
        'url' => $url,
        'method' => $method,
        'header' => $header,
        'response' => $response,
        'code' => $code,
        'message' => $message,
        'curl_info' => $info,
        'backtrace' => debug_backtrace(
          !DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS
        ),
      ];
      if (static::isDebugMode()) {
        dpm($exception_data['url'], 'URL');
        dpm($exception_data['method'], 'Method');
        dpm(print_r($exception_data['header'], 1), 'Header');
        dpm(print_r($exception_data['payload'], 1), 'Payload');
        dpm(print_r($exception_data['response'], 1), 'Response Body');
        dpm(print_r($exception_data['curl_info'], 1), 'Curl Info');
        dpm(print_r($exception_data['backtrace'], 1), 'Backtrace');
      }
      if (function_exists('drupal_alter')) {
        // tpps_rest_api_exception_alter().
        drupal_alter('rest_api_exception', $message, $exception_data);
      }
      throw new Exception('RestAPI HTTP-request: ' . $message, $code);
    }
  }

}
