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
   * Execution time:
   * Custom parser: 0.30-0.40 msec.
   * Gemini AI (parent::parse()): 7874 msec.
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
      $fields = parent::parse();
    }
    else {
      $fields = [];
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
      if (PublicationDOI::isExists($doi, $source)) {
        // Record already exists.
        // @TODO Imlement ability to force parsing (especially for custom parsing (not AI).
        return PublicationDOI::load($doi, $source);
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Reduce number of code to speed-up processing.
      $string = $this->serp;
      $pattern = '/<div\s+class\s*=\s*"gs_ri(.*?)<\/div>'
        . '<div\s+class\s*=\s*"gs_fl/s';
      if (preg_match($pattern, $this->serp, $matches)) {
        $string = $matches[1];
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // WARNING:
      // Replace all non-breaking spaces, zero-width spaces, and other
      // whitespace with regular spaces.
      $string = preg_replace('/[\s\x{00A0}\x{200B}\x{FEFF}]+/u', ' ', $string);

      $needle = '<span class="gs_pdot">•</span>';
      if (strpos($string, $needle) !== FALSE) {
        $string = str_replace($needle, ' - ', $string);
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Title.
      $pattern = '/<h3 class="gs_rt"[^>]*><a[^>]*>(.*?)<\/a><\/h3>/s';
      if (preg_match($pattern, $string, $matches)) {
        $fields['title'] = $matches[1];
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Abstract.
      // Full version.
      $pattern = '/<div class="gsh_csp">(.*?)<\/div>'
        . '<\/div><div class="gs_fma_fons">/s';
      if (preg_match($pattern, $string, $matches)) {
        $fields['abstract'] = $matches[1];
      }
      else {
        // Short version.
        $pattern = '/<div class="gs_rs\s*(?:gs_fma_s)?">(.*?)'
          . '<\/div><div class="gs_fma\s*(?:gs_fma_p)?/s';
        if (preg_match($pattern, $string, $matches)) {
          $fields['abstract'] = $matches[1];
        }
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Authors, journal, date.
      // Full version.
      $pattern = '/<div class="gs_fmaa">(.*?)<\/div>'
        . '<div class="gs_rs\s+gs_fma_s">/s';
      if (preg_match($pattern, $string, $matches)) {
        $a_string = $matches[1];
        $a_string = str_replace('</div>', ' - ', $a_string);
      }
      else {
        // Shortened version.
        $pattern = '/<div class="gs_a gs_fma_s">(.*?)'
          . '<\/div><div class="gs_a gs_fma_p">/s';
        if (preg_match($pattern, $string, $matches)) {
          $a_string = $matches[1];
          $a_string = str_replace('…', '', $a_string);
        }
      }

      if (!empty($a_string)) {
        // Extract authors (text inside <a> tags)
        // Example: "M Lu, CA Loopstra, KV Krutovsky".
        preg_match_all('/<a[^>]*>([^<]+)<\/a>/', $a_string, $authorMatches);
        $fields['authors'] = implode(', ', $authorMatches[1]);

        // Remove authors.
        $a_string = str_replace($authorMatches[0], '', $a_string);
        // Clean up: remove extra commas and spaces.
        $a_string = trim(preg_replace('/^(\s*,\s*)*[^-]*-\s*/', '', $a_string));

        // Extract journal (between " - " and ", ")
        // Example: "Ecology and evolution".
        preg_match('/^([^,]+), /', $a_string, $journalMatches);
        $fields['journal'] = $journalMatches[1] ?? '';
        $a_string = str_replace($journalMatches[0], '', $a_string);

        // Extract date.
        // Example: "2019".
        preg_match('/^(.*?) - /', $a_string, $dateMatches);
        $fields['date'] = $dateMatches[1] ?? '';

        $fields['source'] = $source;
        $fields['doi'] = $doi;
        // Save.
        if (!empty($fields['authors'])) {
          // parent::parse() saves own results itself.
          $this->saveParsed($fields);
        }
      }
      else {
        // If authors can't be found at SERP then DOI wasn't found and
        // Google Scholar suggests other articles which could have DOI as
        // reference.
        // Fallback: Gemini AI.
        $fields = parent::parse();
      }
    }
    return $fields;
  }

}
