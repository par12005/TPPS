<?php

/**
 * @file
 * Contains 'RestAPI' class.
 *
 * WARNING:
 * Do not use any caching in class. Caching must be done outside the class
 * because not all responses and not all methods could be cached.
 *
 *
 * Usage:
 *  RestAPI::post($url, $payload);
 *  RestAPI::delete($url);
 *
 *  @TODO Convert to dynamyc (not static) class to store response data
 *  after request and generated 'requestId' in __construct() method.
 *  @TODO Idea: Give each request a nickname to store in log.
 *  @TODO Allow in the call of ::doRequest() decide if previous response
 *  could be used.
 */

/**
 * Performs Rest API requiests using curl.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestAPI {

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
   *   WARNING: Item 'query' is a sub-element of the $data.
   * @param array $header
   *   HTTP-headers.
   *
   * @return mixed
   *   Returns API response.
   */
  public static function get($url, array $data = [], array $header = []) {
    $query = $data['query'] ?? [];
    return self::doRequest('GET', $url, $query, $header);
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
   * Performs REST API calls.
   *
   * @param string $method
   *   HTTP method. Possible values are: 'POST', 'GET', 'PUT', 'DELETE'.
   * @param string $url
   *   API Endpoint URL.
   * @param mixed $payload
   *   Payload. Required for POST-requests (payload).
   *   Could be a URL query part for GET-requests.
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
  public static function doRequest($method, $url, $payload = [], array $header = []) {
    $method = strtoupper($method);
    $request_id = RestAPILog::createRequestId($method, $url, $payload, $header);

    if (
      variable_get('tpps_rest_api_use_response_cache', FALSE)
      && RestAPILog::isResponseExists($request_id)
    ) {
      // Reuse stored response if possible.
      extract(RestAPILog::loadResponse($request_id));
    }
    else {
      $curl = curl_init();
      // Why not use CURLOPT_POS: https://stackoverflow.com/a/74812403
      curl_setopt($curl, CURLOPT_CUSTOMREQUEST, $method);
      curl_setopt($curl, CURLOPT_HTTPHEADER, $header);

      if (!empty($payload)) {
        if (in_array($method, ['POST', 'DELETE', 'PUT'])) {
          curl_setopt($curl, CURLOPT_SAFE_UPLOAD, TRUE);
          // Different processing of array and string:
          // https://stackoverflow.com/questions/5224790
          curl_setopt($curl, CURLOPT_POSTFIELDS, $payload);
        }
        else {
          // GET.
          // Note: $payload contains URL's query.
          $url = url($url, [
            'https' => TRUE,
            'external' => TRUE,
            'query' => $payload,
          ]);
        }
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Allow other modules to react on start of the request.
      if (function_exists('module_invoke_all')) {
        // tpps_rest_api_request_started().
        $context = [
          'request_id' => $request_id,
          'method' => $method,
          'url' => $url,
          'payload' => $payload ?? NULL,
          'header' => $header ?? NULL,
        ];
        module_invoke_all('rest_api_request_started', $context);
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // OPTIONS:
      // See http://www.php.net/manual/en/function.curl-setopt.php
      curl_setopt($curl, CURLOPT_URL, $url);
      curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
      curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
      // The maximum number of seconds for curl operation.
      // Note: disabled because valid requests failed because of timeout.
      //curl_setopt($curl, CURLOPT_TIMEOUT, static::getTimeout());
      //curl_setopt($curl, CURLOPT_CONNECTTIMEOUT, static::getTimeout());

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // EXECUTE:
      $execution_time_start = microtime(1);
      $response = curl_exec($curl);
      $info = curl_getinfo($curl);
      curl_close($curl);

      // Execution time.
      $execution_time = (microtime(1) - $execution_time_start) * 1000;

      // Prepare response.
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
      $message = HttpCode::getMessage($code);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Allow other modules to react on request's finish.
      if (function_exists('module_invoke_all')) {
        // tpps_rest_api_request_completed().
        $context = [
          'request' => $context,
          'request_id' => $request_id,
          'code' => $code,
          //'message' => $message,
          'duration' => $execution_time,
          'body' => $response,
        ];
        module_invoke_all('rest_api_request_completed', $context);
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if ($code == 0 && empty($response)) {
      throw new Exception($message);
    }
    if ($code >= 200  && $code <= 204) {
      // Codes: https://developers.zenodo.org/?shell#http-status-codes
      return $response;
    }
    else {
      $exception_data = [
        'payload' => empty($payload) ? [] : json_decode($payload),
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
      if (function_exists('drupal_alter')) {
        // tpps_rest_api_exception_alter().
        drupal_alter('rest_api_exception', $message, $exception_data);
      }
      throw new Exception('RestAPI HTTP-request: ' . $message, $code);
    }
  }

}
