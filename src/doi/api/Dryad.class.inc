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
  const KEY = 'dryad';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = 'datadryad.org';
  // IPv4 address.
  const HOST_IP = '34.218.162.251';
  // Endpoint path (without trailing slash).
  const ENDPOINT_PATH = 'api/v2/datasets';
  // Default delay between requests, in seconds.
  //
  //
  // @TODO Use this delay.
  const DEFAULT_DELAY = 2;

  /**
   * Gets DOI Publication information from remote server dryad.
   *
   * Based on code from function tpps_doi_info($doi).
   * URL example:
   *   https://datadryad.org/api/v2/datasets/doi:10.5061/dryad.j1fd7
   *
   * @param string $doi
   *   DOI.
   *
   * @return array
   *   Returns response which will be returned back as JSON. Keys:
   *   'error' bool TRUE means validation failed and FALSE that
   */
  public static function getInfo($doi) {
    $info = [];

// @TODO Use 'RestAPI' class.
    $url = static::buildUrl($doi);
    $use_rest_api = 1;
    if ($use_rest_api) {
      $header = [
        'Content-Type: application/json',
      ];
      try {
        $response = (new RestAPI([
          'url' => $url,
          'header' => $header,
        ]))->get()->response;
        $response = json_decode($response, 1);
        //$response = json_decode($response);
      }
      catch (Exception $e) {
        // @TODO Minor. Code 429 (Too many requiests).
        $message = $e->getMessage();
        $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
        watchdog('tpps', $message . '<pre>@backtrace</pre>',
          ['@backtrace' => print_r($stack, 1)],
          WATCHDOG_ERROR
        );
        // throw new Exception($message, $e->getCode());
        drupal_set_message($e->getCode() . ' ' . $message);
      }
    }
    else {
      // Note: json_decode() could throw an exception on invalid data.
      try {
        $content = file_get_contents($url);
        $response = json_decode($content);
      }
      catch (Exception $e) {
        return t('Caught exception: @message. Try again later.',
          ['@message' => $e->getMessage()]);
      }
    }

dpm($response);

    if (!empty($response)) {
      // Authors.
      foreach ($response->authors as $author) {
        $authors[] = trim($author->firstName . ' ' . $author->lastName);
      }
      $primary_author = $authors[0];
      $secondary_authors = array_slice($authors, 1);
      // Species.
      $species = [];
      foreach ($response->keywords as $keyword) {
        if (preg_match('/^([A-Z][a-z]* [a-z]+)$/', $keyword, $matches)) {
          $species[] = $matches[1];
        }
      }
      $info = [
        'title' => ((substr($response->title, 0, 11) == 'Data from: ')
          ? substr($response->title, 11) : $response->title),
        'primary' => $authors[0],
        'secondary' => $secondary_authors,
        'secondaryNumber' => count($secondary_authors),
        // @TODO [VS] Probably not used. Check!
        'secondaryCheck' => FALSE,
        'year' => explode('-', $response->publicationDate)[0],
        'abstract' => $response->abstract,
        'species' => $species,
        'speciesNumber' => count($species),
      ];

      if (variable_get('tpps_page_1_ajax_debug')) {
        $info['response'] = $response;
        $info['url'] = $url;
      }
    }
    return $info;
  }

  /**
   * Request DOI Publication data using API and store in DB.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   */
  public static function loadPublicationData($doi) {
    if (PublicationDOI::isExists($doi, static::KEY)) {
      return;
    }
    $info = [];

    $url = static::buildUrl($doi);
    $header = [
      'Content-Type: application/json',
    ];
    try {
      $response = (new RestAPI([
        'url' => $url,
        'header' => $header,
      ]))->get()->response;
      //$response = json_decode($response, 1);
    }
    catch (Exception $e) {
      // @TODO Minor. Code 429 (Too many requiests).
      $message = $e->getMessage();
      $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
      watchdog('tpps', $message . '<pre>@backtrace</pre>',
        ['@backtrace' => print_r($stack, 1)],
        WATCHDOG_ERROR
      );
      // throw new Exception($message, $e->getCode());
      drupal_set_message($e->getCode() . ' ' . $message);
    }

dpm(print_r($response, 1));

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Save in local DB.
    //
    // @TODO Add check: If you exceed the rate limits, you will receive a
    // 429 HTTP response. In this case, wait a short while and try your
    // request again at a lower rate or with fewer simultaneous requests.
    if (!empty($response) && 0) {
      $author_list = [];
      foreach ($response['message']['author'] as $item) {
        // Example of the $item:
        //   given: "J L",
        //   family: "Hamrick",
        //   sequence: "additional",
        //   affiliation: [ ]
        // Possible values for 'sequence': "first", "additional".
        $author_list[] = $item['given'] . ' ' . $item['family'];
      }
      $data = [
        'doi' => $doi,
        'source' => static::KEY,
        'authors' => implode(', ', $author_list),
        'date' => implode('/', $response['message']['published-print']['date-parts'][0]),
        'title' => $response['message']['title'][0],
        'journal' => $response['message']['container-title'][0],
        // There is no abstract in response.
        'abstract' => '',
      ];
      PublicationDOI::save($data);
    }
  }

}
