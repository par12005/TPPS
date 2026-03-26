<?php

/**
 * @file
 * Contains 'GoogleScholar' class.
 *
 * What this class does?
 * 1. Saves/loads Google Scholar SERP for certain DOI into DB table:
 * 'tpps_google_scholar'.
 * 2. This class DO NOT requests SERP directly because those requests will be
 * blocked by Google Scholar.
 * 3. Manages delay between requests.
 * 4. Manages parsing. Data will be stored in PublicationDOI::TABLE table.
 *
 * Workaround.
 *
 * Direct requests to Google Scholar will be blocked.
 * Setup CORS Anywhere proxy server which cuts some headers from the Google
 * Scholar GET-request of the search result page (SERP).
 * Use client's browser to get Google Scholar response in iframe.
 * Note: curl requests looks like a a bot/scraper requests and will be blocked.
 * Request from client's browser looks more natural (has cookies and etc.).
 * Content from client's browser will be sent to backend and saved by this class.
 *
 *
 * Usage:
 *  // Save SERP.
 *  $gs = new GoogleScholar();
 *  $gs->doi = $doi;
 *  $gs->serp = $content;
 *  $gs->save();
 *  // Load SERP.
 *  $gs = new GoogleScholar($doi);
 *  dpm($gs->serp);
 *
 * :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * @todo Currently every call of save() will create new record in DB table.
 * This means that table size will grow very quickly but it means we have a
 * history. Find a way to remove outdated data.
 * @todo Minor. Request Google scholar front page at first step to emulate human's
 * behavior. Note: form uses hidden fields which could be changed anytime.
 */

/**
 * Gets publication info by DOI form Google Scholar.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class GoogleScholar extends DoiSerp {

  // Unique key used to get Drupal variables and etc.
  const KEY = 'google_scholar';
  // Name of the DB table where SERP is stored.
  const TABLE = 'tpps_google_scholar';
  // Default value of the delay between requests.
  // Value 10-60 seconds is suggested over the internet to avoid IP address ban.
  const DEFAULT_DELAY = 5;
  // Path to the SERP page without leading slash '/'.
  // Used to build URL for iframe.
  const SERP_PATH = 'scholar';
  // Name of the GET-query parameter which holds DOI.
  const QUERY_PARAM = 'q';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = 'scholar.google.com';
  // Alternative IP address: 142.251.37.100.
  const HOST_IP = '142.250.180.68';

  /**
   * Gets query (url params) to build URL to get SERP.
   *
   * @return array
   *   Returns query for url().
   */
  public function getQuery() {
    return [
      'hl' => 'en',
      'as_sdt' => '0,5',
      self::QUERY_PARAM => urlencode(check_plain($this->doi)),
      'btnG' => NULL,
    ];
  }

}
