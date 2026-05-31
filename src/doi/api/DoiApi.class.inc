<?php

/**
 * @file
 * Contains 'DoiApi' class.
 *
 * This is parent class for APIs which provide DOI Publication data.
 * It's alternative to 'DoiSerp' class which parses SERP to get DOI
 * Publication data.
 */

/**
 * Contans common methods to get DOI Publication data using API.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
abstract class DoiApi {

  // Unique key used to get Drupal variables and etc.
  const KEY = '';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = '';
  // IPv4 address.
  const HOST_IP = '';
  // Endpoint path (without leading and trailing slash).
  const ENDPOINT_PATH = '';
  // Default delay between requests, in seconds.
  const DEFAULT_DELAY = 2;

  /**
   * Gets DOI Publication information.
   *
   * Based on code from function tpps_doi_info($doi).
   * URL example:
   *   https://datadryad.org/api/v2/datasets/doi:10.5061/dryad.j1fd7
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   *
   * @return array
   *   Returns response which will be returned back as JSON. Keys:
   *   'error' bool TRUE means validation failed and FALSE that
   */
  public static function getInfo($doi) {
    return [];
  }

  /**
   * Request DOI Publication data using API and store in DB.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   */
  public static function insert($doi) {}

  /**
   * Builds Endopoint URL.
   *
   * @return string
   *   Returns Endpoint URL.
   *   No trailing slash.
   */
  public static function getEndpointUrl() {
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
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
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
