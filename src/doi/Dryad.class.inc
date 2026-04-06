<?php

/**
 * @file
 * Contains static 'Dryad' class.
 *
 * No need to extend DoiSerp class beause Dryad has API.
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
class Dryad {

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
   * Gets Endopoint URL.
   *
   * @return string
   *   Returns Endpoint URL.
   *   No trailing slash.
   */
  public static function getEndpointURL() {
    $host = static::HOST_NAME;
    if (variable_get('tpps_doi_' . static::KEY . '_use_ip', FALSE)) {
      $host = static::HOST_IP;
    }
    $path = variable_get(
      'tpps_doi_' . static::KEY . '_endpoint_path',
      static::ENDPOINT_PATH
    );
    return 'https://' . $host . '/' . $path;
  }

  /**
   * Builds URL to get SERP.
   *
   * @return string
   *   Returns escaped URL.
   */
  public static function buildUrl($doi) {
    return static::getEndpointURL() . '/' . urlencode('doi:' . $doi);
  }

  /**
   * Saves publication data into DB table.
   *
   * @param array $data
   *   See self::TABLE columns.
   *
   * @return bool
   *   Returns TRUE in case of success and FALSE otherwise.
   *
   * @throws Exception
   *   When record for given $data['doi'] already exists.
   */
  public static function save(array $data = []) {
    return PublicationDOI::save($data);
  }

  /**
   * Saves DOI publication data into database.
   */
  public static function load($doi) {
    return PublicationDOI::load($doi, static::KEY);
  }

  /**
   * Gets class name.
   *
   * @return string
   *   Returns class name.
   */
  public static function getClassName() {
    return static::class;
  }
}
