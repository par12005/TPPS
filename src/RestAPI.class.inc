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
   * @param array $data
   *   Data for request.
   *
   * @return mixed
   *   Returns API response.
   */
  public static function post($url, array $data = '') {
    return self::doRequest('POST', $url, $data);
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
  public static function get($url, array $data = '') {
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
   *
   * @return mixed
   *   Returns API response.
   */
  public static function put($url, array $data = '') {
    return self::doRequest('PUT', $url, $data);
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
  public static function delete($url, array $data = '') {
    return self::doRequest('DELETE', $url, $data);
  }

  /**
   * Performs REST API calls.
   *
   * @param string $method
   *   HTTP method. Possible values are: 'POST', 'GET', 'PUT', 'DELETE'.
   * @param string $url
   *   API Endpoint URL.
   * @param array $data
   *   Data. Required for POST-requests.
   *
   * @return mixed
   *   Returns API response.
   *
   * @todo Convert to private or protected.
   */
  public static function doRequest($method, $url, array $data = '') {
    $curl = curl_init();
    switch (strtoupper($method)) {
      case "POST":
        curl_setopt($curl, CURLOPT_POST, 1);
        if ($data) {
          curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
        }
        break;

      case "PUT":
        curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if ($data) {
          curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
        }
        break;

      default:
        if ($data) {
          $url = sprintf("%s?%s", $url, http_build_query($data));
        }
    }
    // OPTIONS:
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_HTTPHEADER, array(
       'APIKEY: 111111111111111111111',
       'Content-Type: application/json',
    ));
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    // EXECUTE:
    $result = curl_exec($curl);
    if (!$result) {
      die("Connection Failure");
    }
    curl_close($curl);
    return $result;
  }

}
