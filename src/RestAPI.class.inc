<?php

/**
 * @file
 * Contains 'RestAPI' class.
 *
 * Usage:
 *  RestAPI::post($url, $data);
 *  RestAPI::delete($url);
 */

/**
 * Performs Rest API requiests using curl.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestAPI {

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
    if (empty($header)) {
      $header = [
        'Content-Type: application/json',
      ];
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
   * @param array $data
   *   Data for request.
   * @param array $header
   *   HTTP-headers.
   *
   * @return mixed
   *   Returns API response.
   */
  public static function put($url, array $data = [], array $header = []) {
    if (empty($header)) {
      $header = [
        'Content-Type: application/json',
      ];
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
   * @param mixed $data
   *   Data. Required for POST-requests (payload).
   *   Could be a string or JSON-string.
   * @param array $header
   *   HTTP-headers. Required for POST-requests.
   *
   * @return mixed
   *   Returns API response.
   *
   * @todo Convert to private or protected.
   */
  public static function doRequest($method, $url, $data = [], array $header = []) {
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_HTTPHEADER, $header);
    $method = strtoupper($method);

    switch ($method) {
      case 'POST':
        curl_setopt($curl, CURLOPT_POST, 1);
        // Custom:
        if ($data) {
          //$json = json_encode($data, JSON_FORCE_OBJECT);
          $json = json_encode($data);
          curl_setopt($curl, CURLOPT_POSTFIELDS, $json);
        }
        break;

      case 'DELETE':
      case 'PUT':
        curl_setopt($curl, CURLOPT_CUSTOMREQUEST, $method);
        if ($data) {
          curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
        }
        break;

      default:
        if ($data) {
          $url = url($url, [
            'query' => $data,
            'https' => TRUE,
            'external' => TRUE,
          ]);
        }
    }
    // OPTIONS:
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    // EXECUTE:
    $result = curl_exec($curl);
    curl_close($curl);
    return $result;
  }

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
    $debug_mode = FALSE;
    if ($debug_mode) {
      dpm(print_r(debug_backtrace(
        !DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3
      ), TRUE));
    }
    $data = json_decode((string) $json, TRUE);
    if (JSON_ERROR_NONE !== json_last_error()) {
      $message = 'Unable to parse response body into JSON. ' . json_last_error();
      throw new RuntimeException($message);
    }
    return $data === NULL ? [] : $data;
  }

}
