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
  const NOT_FOUND_TOKEN = 'The following terms were not found';

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

  /**
   * Scraps SERP to get publication info.
   *
   * Publication data will be saved using PublicationDOI::save().
   *
   * Execution time:
   * PubMed custom parser: 0.3-0.4 msec.
   * Google Scholar custom parser: 0.30-0.40 msec.
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

      if (
        PublicationDOI::isExists($doi, $source)) {
        // Record already exists.
        // @TODO Imlement ability to force parsing (especially for custom parsing (not AI).
        return PublicationDOI::load($doi, $source);
      }
      else {
        $string = $this->serp;
        // Reduce size of the HTMl-code to speed-up processing.
        // Without this step execution time is 2.1 msec instead of 1.3 msec.
        $pattern = '/(<meta name="citation_title".*'
          . '<p class="disclaimer" id="disclaimer">)/s';
        if (preg_match($pattern, $string, $matches)) {
          $string = $matches[1];
        }

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // WARNING:
        // Replace all non-breaking spaces, zero-width spaces, and other
        // whitespace with regular spaces.
        // Takes about 1 msec.
        if (0) {
          $string = preg_replace('/[\s\x{00A0}\x{200B}\x{FEFF}]+/u', ' ', $string);
        }

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Title.
        $pattern = '/<meta name="citation_title" content="(.*?)">/s';
        if (preg_match($pattern, $string, $matches)) {
          $fields['title'] = $matches[1];
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Abstract.
        $pattern = '/<div class="abstract-content selected"\s+id="eng-abstract">'
          . '\s*<p>\s*(.*)<p class="disclaimer" id="disclaimer">/s';
        if (preg_match($pattern, $string, $matches)) {
          $fields['abstract'] = $matches[1];
          // Do some cleanup.
          // @TODO Minor. Combine into single regex.
          $fields['abstract'] = preg_replace(
            '/\s*<\/p>\s*<\/div>\s*.*$/s',
            '', $fields['abstract']
          );
          $fields['abstract'] = trim($fields['abstract']);
          // Make sense to remove only this piece because it's big enough.
          $string = str_replace($matches[0], '', $string);
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Authors.
        $pattern = '/<meta name="citation_authors" content="(.*?);">/s';
        if (preg_match($pattern, $string, $matches)) {
          $fields['authors'] = str_replace(';', ', ', $matches[1]);
          $fields['authors'] = str_replace('  ', ' ', $fields['authors']);
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Date.
        $pattern = '/<meta name="citation_date" content="(.*?)">/s';
        if (preg_match($pattern, $string, $matches)) {
          // Format: mm/dd/yyyy.
          $fields['date'] = $matches[1];
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Journal.
        $pattern = '/<meta name="citation_journal_title" content="(.*?)">/s';
        if (preg_match($pattern, $string, $matches)) {
          $fields['journal'] = $matches[1];
        }

        // @TODO Check is complete?
        if (empty($fields)) {
          // If authors can't be found at SERP then DOI wasn't found and
          // Google Scholar suggests other articles which could have DOI as
          // reference.
          // Fallback: Gemini AI.
          $fields = parent::parse();
        }
        else {
          $fields['source'] = $source;
          $fields['doi'] = $doi;
          $this->saveParsed($fields);
        }
      }
    }
    return $fields;
  }

}
