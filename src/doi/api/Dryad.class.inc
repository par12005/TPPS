<?php

/**
 * @file
 * Contains static 'Dryad' class.
 *
 * No need to extend DoiSerp class because Dryad has API.
 */

/**
 * Manages publication DOI information.
 *
 * To get lists of Publication/Dataset DOI see reports:
 * /admin/reports/tpps/doi/submission/publication.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class Dryad extends DoiApi {

  // Unique key used to get Drupal variables and etc.
  // To get key by class name: $name = constant($class_name . '::KEY');
  // WARNING:
  // In DB table 'public.tpps_publication_doi.source' used class name
  // (not class's KEY constant).
  const KEY = 'dryad';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = 'datadryad.org';
  // IPv4 address.
  const HOST_IP = '34.218.162.251';
  // Endpoint path (without leading and trailing slash).
  const ENDPOINT_PATH = 'api/v2/datasets';
  // Default delay between requests, in seconds.
  //
  //
  // @TODO Use this delay.
  const DEFAULT_DELAY = 2;

  /**
   * Builds URL for API Endpoint.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   *
   * @return string
   *   Returns escaped URL.
   */
  public static function buildUrl($doi) {
    return static::getEndpointUrl() . '/' . urlencode('doi:' . $doi);
  }

  /**
   * Request DOI Publication data using API and store in DB.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   */
  public static function insert($doi) {
    if (PublicationDoi::isExists($doi, static::class)) {
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
      !empty($response)
      && ($response['error'] ?? '') != "not-found"
    ) {
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Save in local DB.
      //
      // @TODO Add check: If you exceed the rate limits, you will receive a
      // 429 HTTP response. In this case, wait a short while and try your
      // request again at a lower rate or with fewer simultaneous requests.
      $author_list = [];
      foreach ($response['authors'] as $item) {
        // Example of the $item:
        //   firstName: "Ryan",
        //   lastName: "Schmid",
        //   email: "ryan.schmid@ecdysis.bio",
        //   affiliation: "Ecdysis Foundation",
        //   affiliations: [{ name: "Ecdysis Foundation" }],
        //   orcid: "0000-0002-7638-5619".
        $author_list[] = $item['firstName'] . ' ' . $item['lastName'];
      }
      // Get species.
      $species = [];
      // Note: Not all keywords are species but those which has 2 words.
      foreach ($response['keywords'] as $keyword) {
        if (preg_match('/^([A-Z][a-z]* [a-z]+)$/', $keyword, $matches)) {
          $species[] = trim($matches[1]);
        }
      }
      $data = [
        'doi' => $doi,
        'source' => static::class,
        'authors' => implode(', ', $author_list),
        'date' => str_replace('-', '/', ($response['publicationDate'] ?? '')),
        'title' => str_replace('Data from: ', '', $response['title']),
        // Note: no record for Journal.
        'journal' => '',
        'abstract' => trim($response['abstract']),
        // Only this service provides list of species.
        'species' => implode(', ', $species),
      ];
    }
    PublicationDoi::save($data);
  }

  /**
   * Request DOI Metadata.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   */
  public static function request($doi) {
    $url = static::buildUrl($doi);
    $header = [
      'Content-Type: application/json',
    ];
    try {
      $response = (new RestApi([
        'url' => $url,
        'header' => $header,
        'tag' => implode(', ', [$doi, static::class]),
      ]))->get()->response();
      return $response;
    }
    catch (Exception $e) {
      // @TODO Minor. Code 429 (Too many requiests).
      $message = $e->getMessage();
      $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
      watchdog('tpps', '@message <pre>@backtrace</pre>',
        ['@message' => $message, '@backtrace' => print_r($stack, 1)],
        WATCHDOG_ERROR
      );
      // throw new Exception($message, $e->getCode());
      drupal_set_message($e->getCode() . ' ' . $message);
    }
  }

}
