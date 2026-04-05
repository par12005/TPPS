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
   *
   * @param string $doi
   *   DOI.
   *
   * @return array
   *   Returns response which will be returned back as JSON. Keys:
   *   'error' bool TRUE means validation failed and FALSE that
   */
  public static function getInfo($doi) {
    $key = strtolower($doi);
    $info = [];

// @TODO Use 'RestAPI' class.
// @TODO Update. New variables.

    $api_endpoint = variable_get(
      'tpps_page_1_dryad_api_url',
      'http://datadryad.org/api/v2/datasets/'
    );
    $url = $api_endpoint . urlencode('doi:' . $doi);
    // Note: json_decode() could throw an exception on invalid data.
    try {
      $content = file_get_contents($url);
      $data = json_decode($content);
    }
    catch (Exception $e) {
      return t('Caught exception: @message. Try again later.',
        ['@message' => $e->getMessage()]);
    }

    if (!empty($data)) {
      // Authors.
      foreach ($data->authors as $author) {
        $authors[] = trim($author->firstName . ' ' . $author->lastName);
      }
      $primary_author = $authors[0];
      $secondary_authors = array_slice($authors, 1);
      // Species.
      $species = [];
      foreach ($data->keywords as $keyword) {
        if (preg_match('/^([A-Z][a-z]* [a-z]+)$/', $keyword, $matches)) {
          $species[] = $matches[1];
        }
      }
      $info = [
        'title' => ((substr($data->title, 0, 11) == 'Data from: ')
          ? substr($data->title, 11) : $data->title),
        'primary' => $authors[0],
        'secondary' => $secondary_authors,
        'secondaryNumber' => count($secondary_authors),
        // @TODO [VS] Probably not used. Check!
        'secondaryCheck' => FALSE,
        'year' => explode('-', $data->publicationDate)[0],
        'abstract' => $data->abstract,
        'species' => $species,
        'speciesNumber' => count($species),
      ];

      if (variable_get('tpps_page_1_ajax_debug')) {
        $info['data'] = $data;
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
   * Gets class name.
   *
   * @return string
   *   Returns class name.
   */
  public static function getClassName() {
    return static::class;
  }

  /**
   * Builds URL to get SERP.
   *
   * @return string
   *   Returns escaped URL.
   */
  public static function buildUrl() {


// @TODO update.

    if (empty($this->doi)) {
      throw new Exception("Can't create URL because of the empty DOI.");
    }
    $url = url(
      $this->getEndpointURL(),
      ['query' => $this->getQuery()]
    );
    return $url;
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

}
