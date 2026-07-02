<?php

/**
 * @file
 * Contains 'DataDryad' class.
 *
 * Class names:
 * DataDryad - SERP,
 * Dryad - API.
 *
 * datadryad.org uses Anubis (https://github.com/TecharoHQ/anubis) which
 * sends some kind of cripto-task to browser (like block-chain and bitcoin).
 */

/**
 * Gets publication info by DOI form DataDryad web-site's SERP.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class DataDryad extends DoiSerp {

  // Unique key used to get Drupal variables and etc.
  // To get key by class name: $name = constant($class_name . '::KEY');
  // WARNING:
  // In DB table 'public.tpps_publication_doi.source' used class name
  // (not class's KEY constant).
  const KEY = 'datadryad';
  // Name of the DB table where SERP is stored.
  const TABLE = 'tpps_datadryad';
  // Default value of the delay between requests.
  // Value 10-60 seconds is suggested over the internet to avoid IP address ban.
  const DEFAULT_DELAY = 5;
  // Path to the SERP page without leading slash '/'.
  // Used to build URL for iframe.
  // Example: https://datadryad.org/search?q=10.1002%2Fece3.6829
  const SERP_PATH = 'search';
  // Name of the GET-query parameter which holds DOI.
  const QUERY_PARAM = 'q';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = 'datadryad.org';
  // Token to detect page without publication data.
  const NOT_FOUND_TOKEN = 'Results 0 of 0';

  /**
   * Gets query (url params) to build URL to get SERP.
   *
   * @return array
   *   Returns query for url().
   */
  public function getQuery() {
    $query = [
      self::QUERY_PARAM => urlencode(check_plain($this->doi)),
    ];
    if (variable_get('tpps_doi_' . static::KEY . '_unique_url', FALSE)) {
      $query['z'] = REQUEST_TIME;
    }
    return $query;
  }

}
