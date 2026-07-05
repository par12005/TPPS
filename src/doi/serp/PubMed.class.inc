<?php

/**
 * @file
 * Contains 'PubMed' class.
 *
 * GET-request but when DOI appears in textfield does GET-request:
 * https://pubmed.ncbi.nlm.nih.gov/suggestions/?term=10.3732%20apps.1300025
 * Note: '/' in DOI replaced with space!
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Usage:
 *  // Save SERP.
 *  $provider = new PubMed();
 *  $provider->doi = $doi;
 *  $provider->serp = $content;
 *  $provider->save();
 *
 *  // Load SERP.
 *  $provider = new PubMed($doi);
 *  dpm($provider->serp);
 *
 *  // Parse SERP.
 *  $provider = new PubMed($doi);
 *  dpm($provider->parse());
 */

/**
 * Gets publication info by DOI form PubMed web-site.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PubMed extends DoiSerp {

  // Unique key used to get Drupal variables and etc.
  // To get key by class name: $name = constant($class_name . '::KEY');
  // WARNING:
  // In DB table 'public.tpps_publication_doi.source' used class name
  // (not class's KEY constant).
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
  // Token to detect page without publication data.
  const NOT_FOUND_TOKEN = 'The following terms were not found in PubMed';

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
