<?php

/**
 * @file
 * Contains 'CrossRef' class.
 */

/**
 * Gets DOI Publication data from CrossRef service.
 *
 * Usage:
 *   To get and save DOI Publication data:
 *   CrossRef::insert($doi);
 *
 * No need to extend DoiSerp class because Dryad has API.
 * https://www.crossref.org/documentation/retrieve-metadata/rest-api/
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class CrossRef extends DoiApi {

  // Unique key used to get Drupal variables and etc.
  // To get key by class name: $name = constant($class_name . '::KEY');
  // WARNING:
  // In DB table 'public.tpps_publication_doi.source' used class name
  // (not class's KEY constant).
  const KEY = 'crossref';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = 'api.crossref.org';
  // IPv4 address.
  const HOST_IP = '';
  // Endpoint path (without leading and trailing slash).
  const ENDPOINT_PATH = 'works';
  // Default delay between requests, in seconds.
  // @TODO Use this delay.
  const DEFAULT_DELAY = 2;

// @TODO $resonse == 'Resource not found.'

  /**
   * Builds URL to get SERP.
   *
   * @return string
   *   Returns escaped URL.
   */
  public static function buildUrl($doi) {
    global $user;
    // When 'mailto' parameter is set we could get higher rate limits.
    // https://www.crossref.org/documentation/retrieve-metadata/rest-api/access-and-authentication/
    $params = [
      'query' => [
        'mailto' => $user->mail ?? variable_get('site_mail', 'test.user@gmail.com'),
      ],
      'https' => TRUE,
    ];
    return url(static::getEndpointUrl() . '/' . $doi, $params);
  }

  /**
   * Request DOI Publication data using API and store in DB.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   */
  public static function insert($doi) {
    if (PublicationDOI::isExists($doi, static::class)) {
      return;
    }

    $response = static::request($doi);
    $response_code = $response['code'] ?? NULL;
    if (!empty($response_code) || ($response_code < 200 && $response_code > 205)) {
      // Do not insert unsuccess response.
      return;
    }
    // Create an empty record to avoid the same requests to API.
    $data = ['doi' => $doi, 'source' => static::class];
    if (
      $response != 'Resource not found.'
      && !empty($response)
      && ($response['status'] ?? '') == 'ok'
    ) {
      // Save in local DB.
      // @TODO Add check: If you exceed the rate limits, you will receive a
      // 429 HTTP response. In this case, wait a short while and try your
      // request again at a lower rate or with fewer simultaneous requests.
      $author_list = [];
      foreach ($response['message']['author'] as $item) {
        // Example of the $item:
        //   given: "J L",
        //   family: "Hamrick",
        //   sequence: "additional",
        //   affiliation: [ ]
        // Possible values for 'sequence': "first", "additional".
        $author_list[] = ($item['given'] ?? '') . ' ' . ($item['family'] ?? '');
      }
      $data = [
        'doi' => $doi,
        'source' => static::class,
        'authors' => implode(', ', $author_list),
        'date' => implode('/', ($response['message']['published-print']['date-parts'][0] ?? [])),
        'title' => ($response['message']['title'][0] ?? ''),
        'journal' => ($response['message']['container-title'][0] ?? ''),
        // There is no abstract in response.
        'abstract' => '',
      ];
    }
    PublicationDOI::save($data);
  }

  /**
   * Request DOI Metadata.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   *
   * @return array
   *   Returns response data.
   */
  public static function request($doi) {
    $url = static::buildUrl($doi);
    $header = [
      'Content-Type: application/json',
      // When 'mailto' parameter and 'User-Agent' header both are specified
      // we could get higher rate limits.
      // https://www.crossref.org/documentation/retrieve-metadata/rest-api/access-and-authentication/
      'User-Agent: Drupal',
    ];
    try {
      $response = (new RestAPI([
        'url' => $url,
        'header' => $header,
        'tag' => implode(', ', [$doi, static::class]),
      ]))->get()->response();
      return $response;
    }
    catch (Exception $e) {
      $message = $e->getMessage();
      $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
      watchdog('tpps', '@message <pre>@backtrace</pre>',
        ['@message' => $message, '@backtrace' => print_r($stack, 1)],
        WATCHDOG_ERROR
      );
      drupal_set_message($e->getCode() . ' ' . $message);
    }
  }

}
