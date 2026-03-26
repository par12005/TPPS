<?php

/**
 * @file
 * Contains 'PubMed' class.
 *
 * GET-request but when DOI appears in textfield does GET-request:
 * https://pubmed.ncbi.nlm.nih.gov/suggestions/?term=10.3732%20apps.1300025
 * Note: '/' in DOI replaced with space!
 */

/**
 * Gets publication info by DOI form PubMed web-site.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PubMed extends DoiSerp {

  // Unique key used to get Drupal variables and etc.
  const KEY = 'pubmed';
  // Name of the DB table where SERP is stored.
  const TABLE = 'tpps_pubmed';
  // Default value of the delay between requests.
  // Value 10-60 seconds is suggested over the internet to avoid IP address ban.
  const DEFAULT_DELAY = 5;
  // Path to the SERP page without leading slash '/'.
  // Used to build URL for iframe.
  // Example: https://pubmed.ncbi.nlm.nih.gov/32638366/
  const SERP_PATH = 'pubmed';
  // Name of the GET-query parameter which holds DOI.
  const QUERY_PARAM = 'term';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = 'pubmed.ncbi.nlm.nih.gov';
  // Alternative IP address: 142.251.37.100.
  const HOST_IP = '130.14.29.110';

  /**
   * Gets query (url params) to build URL to get SERP.
   *
   * @return array
   *   Returns query for url().
   */
  public function getQuery() {
    return [
      self::QUERY_PARAM => urlencode(check_plain($this->doi)),
    ];
  }

}
