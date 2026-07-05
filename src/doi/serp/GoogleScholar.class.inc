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
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Usage:
 *  // Save SERP.
 *  $provider = new GoogleScholar();
 *  $provider->doi = $doi;
 *  $provider->serp = $content;
 *  $provider->save();
 *
 *  // Load SERP.
 *  $provider = new GoogleScholar($doi);
 *  dpm($provider->serp);
 *
 *  // Parse SERP.
 *  $provider = new GoogleScholar($doi);
 *  dpm($provider->parse());
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
  // To get key by class name: $name = constant($class_name . '::KEY');
  // WARNING:
  // In DB table 'public.tpps_publication_doi.source' used class name
  // (not class's KEY constant).
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
  // Token to detect page without publication data.
  const NOT_FOUND_TOKEN = '- did not match any articles.';

  /**
   * Gets query (url params) to build URL to get SERP.
   *
   * @return array
   *   Returns query for url().
   */
  public function getQuery() {
    $query = [
      'hl' => 'en',
      'as_sdt' => '0,5',
      self::QUERY_PARAM => urlencode(check_plain($this->doi)),
      'btnG' => NULL,
    ];
    if (variable_get('tpps_doi_' . static::KEY . '_unique_url', FALSE)) {
      $query['z'] = REQUEST_TIME;
    }
    return $query;
  }

  /**
   * Scraps SERP to get publication info.
   *
   * Publication data will be saved using PublicationDOI::save().
   *
   * @return array
   *   Returns parsed data.
   *
   * @throws Exception
   *   When $doi is empty.
   *   When data loaded from DB but $serp is empty.
   */
  public function parse() {

    if (variable_get('tpps_doi_' . static::KEY . '_use_gemini')) {
      // Use Gemini AI to parse which implemented in parent class.
      return parent::parse();
    }
    else {
      $fields = [];
      // Custom parser.
      if (empty($this->doi)) {
        throw new Exception("Can't create URL because of the empty DOI.");
      }
      if (!$this->isLoaded) {
        $this->load();
      }
      if (empty($this->serp)) {
        // @TODO Call $this->request().
        throw new Exception("Empty SERP. Check CORS Proxy server.");
      }
      $source = static::class;
      $doi = $this->doi;

      if (
        PublicationDOI::isExists($doi, $source)


// @TOdO Remove debug code:
        && 0
      ) {
        // Record already exists.
        // @TODO Imlement ability to force parsing (especially for custom parsing (not AI).
        return PublicationDOI::load($doi, $source);
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Custom parser.
      // Replace all non-breaking spaces, zero-width spaces, and other
      // whitespace with regular spaces.
      $string = preg_replace('/[\s\x{00A0}\x{200B}\x{FEFF}]+/u', ' ', $this->serp);

      // Get authors, journal, date, abstract. No title.
      // Need to add the s modifier for multiline matching.
      $pattern = '/<h3 class="gs_rt"[^>]*><a[^>]*>(.*?)<\/a><\/h3>'
        . '<div class="gs_a">(.*?)<\/div><div class="gs_rs">(.*?)<\/div>'
        . '<div class="gs_fl gs_flb">/s';

      if (preg_match($pattern, $string, $matches)) {
        $fields['title'] = $matches[1];
        $fields['abstract'] = $matches[3];

        // Authors, journal and date.
        $string = $matches[2];
        // Extract authors (text inside <a> tags)
        // Example: "M Lu, CA Loopstra, KV Krutovsky".
        preg_match_all('/<a[^>]*>([^<]+)<\/a>/', $string, $authorMatches);
        $fields['authors'] = implode(', ', $authorMatches[1]);

        // Remove authors.
        $string = str_replace($authorMatches[0], '', $string);
        // Clean up: remove extra commas and spaces.
        $string = trim(preg_replace('/^(\s*,\s*)*-\s*/', '', $string));

        // Extract journal (between " - " and ", ")
        // Example: "Ecology and evolution".
        preg_match('/^([^,]+), /', $string, $journalMatches);
        $fields['journal'] = $journalMatches[1] ?? '';
        $string = str_replace($journalMatches[0], '', $string);

        // Extract date.
        // Example: "2019".
        preg_match('/^(.*?) - /', $string, $dateMatches);
        $fields['date'] = $dateMatches[1] ?? '';
      }

dpm(var_export($fields, 1), 'fields'); return;

      $fields['source'] = $source;
      $fields['doi'] = $doi;

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Store result (if possible).
// @Todo Remove debug code:
$fields = NULL;
      if (!empty($fields)) {
        $this->saveParsed($fields);
      }
      return $fields;
    }
  }

}
