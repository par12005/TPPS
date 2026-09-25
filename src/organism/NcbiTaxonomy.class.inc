<?php

/**
 * @file
 * Contains static 'NcbiTaxonomy' class.
 *
 * Parent class 'OrganismTaxonomy' provides methods:
 * - getSchema(),
 * - save(),
 * - isExists().
 */

/**
 * Manages requests to NCBI Taxonomy API.
 *
 * Probably other (or newer) API:
 * https://www.ncbi.nlm.nih.gov/datasets/docs/v2/api/rest-api/
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class NcbiTaxonomy extends OrganismTaxonomy {
  /**
   * Name of the table which stores NCBI Taxonomy Ids per organism.
   */
  const TABLE = 'tpps_ncbi_taxonomy';

  // The API URL to search items.
  const SEARCH_URL = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi';

  // The API URL to fetch items.
  const FETCH_URL = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi';

  /**
   * Generates URL to the species page.
   *
   * @param int $taxonomy_id
   *   Taxonomy Id.
   *
   * @return string
   *   Returns URL.
   */
  public static function getUrl($taxonomy_id) {
    return url(
      'https://www.ncbi.nlm.nih.gov/Taxonomy/Browser/wwwtax.cgi',
      ['query' => ['id' => $taxonomy_id]]
    );
  }

  /**
   * Builds direct URL to the API.
   *
   * Used in 'Organism's Taxonomy' report.
   *
   * @param string $organism_name
   *   Full organism name.
   *
   * @return string
   *   Returns URL.
   */
  public static function getApiUrl($organism_name) {
    $url = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi';
    $formatted_organism_name = implode('+', explode(' ', $organism_name));
    $query = [
      'db' => 'taxonomy',
      'term' => "{$formatted_organism_name}[Scientific+Name]",
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
    ];
    return url($url, ['query' => $query]);
  }

  /**
   * Gets NCBI Taxonomy Term data.
   *
   * @param int $taxonomy_id
   *   NCBI Taxonomy Id.
   *
   * @return string
   *   Returns NCBI response in XML-format.
   *
   * @throws Exception
   *   When API-request failed.
   */
  public static function requestTerm($taxonomy_id) {
    if (empty($taxonomy_id)) {
      return FALSE;
    }

    // Use Endpoint with full data.
    $query = [
      'db' => 'taxonomy',
      'id' => $taxonomy_id,
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
      'retmode' => 'xml',
    ];

    try {
      $xml = (new RestApi([
        'url' => static::FETCH_URL,
        'query' => $query,
        'tag' => 'NCBI name by taxonomy id: ' . $taxonomy_id,
        'cache' => variable_get('tpps_organism_ncbi_use_cache', TRUE),
      ]))->get()->response();

      return trim(str_replace("\r\n", "\n", $xml));
    }
    catch (Exception $e) {
      $message = $e->getMessage();
      $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
      watchdog('tpps', '@message <pre>@backtrace</pre>',
        ['@message' => $message, '@backtrace' => print_r($stack, 1)],
        WATCHDOG_ERROR
      );
      drupal_set_message($e->getCode() . ' ' . $message);
    }
  }

  /**
   * Gets organism primary name by taxonomy id from NCBI database.
   *
   * @param int $taxonomy_id
   *   NCBI Taxonomy Id.
   *
   * @return mixed
   *   Returns (string) organism primary name.
   *   Returns FALSE when $taxonomy_id is empty.
   *   Returns NULL if no data found by $taxonomy_id (taxon doesn't exist).
   *
   * @throws Exception
   *   When API-request failed.
   */
  public static function getPrimaryName($taxonomy_id) {
    $xml = static::requestTerm($taxonomy_id);
    if (preg_match('/<Taxon>/', $xml)) {
      // Exists. Not empty response.
      preg_match('/<ScientificName>(.+)<\/ScientificName>/', $xml, $matches);
      return $matches[1] ?? FALSE;
    }
    return NULL;
  }

  /**
   * Request NCBI Taxonomy Term Id for given organism name.
   *
   * @param mixed $organism_key
   *   Organism Name or Organism Id.
   *   AJAX-requests from browser uses name but Batch Update uses id.
   *
   * @return int
   *   Returns NCBI Taxonomy Id.
   *
   * @throws Exception
   *   When request to NCBI API fails.
   */
  public static function getId($organism_key) {
    $organism_key = check_plain($organism_key);
    try {
      // Get taxonomy id from local DB.
      $organism = new Organism($organism_key);
      // Get previously saved taxonomy id from local DB.
      if (!empty($taxonomy_id = static::load($organism->id))) {
        return [
          'taxonomy_id' => (int) $taxonomy_id,
          'primary_name' => $organism_key,
        ];
      }
      else {
        throw new Exception('Taxonomy Id not found in local DB.');
      }
    }
    catch (Exception $e) {
      // Get taxonomy id from remote DB.
      if (is_numeric($organism_key)) {
        // Organism id.
        // Organism wasn't found in local DB.
        // Taxonomy id can't be found because there is no organism name.
        watchdog('tpps', $e->getMessage(), [], WATCHDOG_WARNING);
        return NULL;
      }
      else {
        // Organism name.
        // Organism wasn't found in local DB but it's not a problem.
        // Let's search it in NCBI database.
        $xml = static::getInfo($organism_key);
        if (preg_match('/<OutputMessage>No items found\.<\/OutputMessage>/', $xml)) {
          return NULL;
        }
        else {
          // Item was found.
          preg_match('/<Id>(\d+)<\/Id>/', $xml, $matches);
          $id = $matches[1] ?? '';
          $taxonomy_id = (empty($id) ? NULL : (int) $id);
          // Get organism primary name.
          $organism_name = static::getPrimaryName($taxonomy_id);
          if (!empty($organism_name)) {
            $organism = new Organism();
            $organism->setName($organism_name);
            $organism->save();
            // Save relation between taxonomy_id and organism name.
            static::save(
              $organism->id,
              $taxonomy_id
            );
          }
          return [
            'taxonomy_id' => (int) $taxonomy_id,
            'primary_name' => $organism_name,
          ];
        }
      }
    }
    return NULL;
  }

  /**
   * Gets list of synonyms.
   *
   * @param int $taxonomy_id
   *   GBIF Taxonomy Id.
   *
   * @return mixed
   *   Returns list of synonyms or NULL.
   *
   * @throws Exception
   *   When API-request failed.
   */
  public static function getSynonymList($taxonomy_id) {
    if (empty($taxonomy_id)) {
      return FALSE;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get synonym_list.
    $synonym_list = [];

    $xml = static::requestTerm($taxonomy_id);
    preg_match_all('/<(?:Synonym|DispName)>(.*?)<\/(?:Synonym|DispName)>/s', $xml, $matches);
    $synonym_list = $matches[1];
    return $synonym_list;
  }

  /**
   * Gets organism info.
   *
   * @param string $organism_name
   *   Organism name (canonical or synonym).
   *
   * @return array
   *   Returns API response.
   */
  public static function getInfo($organism_name) {
    $query = [
      'db' => 'taxonomy',
      'term' => check_plain($organism_name),
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
    ];
    try {
      $xml = (new RestApi([
        'url' => static::SEARCH_URL,
        'query' => $query,
        'tag' => 'NCBI Taxonomy: ' . $organism_name,
        'cache' => variable_get('tpps_organism_ncbi_use_cache', TRUE),
      ]))->get()->response();
      if (!is_array($xml)) {
        $xml = trim(str_replace("\r\n", "\n", $xml));
      }
      else {

        // array ( 'error' => 'API rate limit exceeded', 'api-key' => '', 'count' => '11', 'limit' => '10', )
        if (!empty($xml['error'])) {
          watchdog('tpps', 'NCBI error: @error_message',
            ['@error_message' => $xml['error']], WATCHDOG_ERROR
          );
        }
        else {
          watchdog('tpps', 'NCBI response: <pre>@message</pre>',
            ['@message' => var_export($xml, 1)], WATCHDOG_ERROR
          );
        }
      }
    }
    catch (Exception $e) {
      $message = $e->getMessage();
      $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
      watchdog('tpps', '@message <pre>@backtrace</pre>',
        ['@message' => $message, '@backtrace' => print_r($stack, 1)],
        WATCHDOG_ERROR
      );
      drupal_set_message($e->getCode() . ' ' . $message, 'error');
    }
    return $xml;
  }

  /**
   * Prints pretty XML.
   *
   * How to use:
   * dpm(NcbiTaxonomy::printXml($info));
   *
   * @param string $xml
   *   XML-string without formatting and linebreaks.
   *
   * @return string
   *   Human-readable XML.
   */
  public static function printXml($xml) {
    $dom = new DOMDocument();
    $dom->preserveWhiteSpace = FALSE;
    $dom->formatOutput = TRUE;
    $dom->loadXML($xml);
    return $dom->saveXML();
  }

  /**
   * Checks if given BioProject Id is valid.
   *
   * @param string $bio_project_id
   *   BioProject Id. E.g., 'PRJNA429695'.
   *
   * @return bool
   *   Returns TRUE if BioProject Id was found and FALSE otherwise.
   *
   * @throws Exception
   *   If curl request failed.
   */
  public static function isValidBioProjectId($bio_project_id) {
    $debug_mode = TRUE;

    $query = [
      'retmax' => 0,
      'usehistory' => 'y',
      'db' => 'bioproject',
      'term' => check_plain($bio_project_id),
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
    ];
    $xml = (new RestApi([
      'url' => static::SEARCH_URL,
      'query' => $query,
      'tag' => 'NCBI BioProject Id Validation: ' . $bio_project_id,
      'cache' => variable_get('tpps_organism_ncbi_use_cache', TRUE),
    ]))->post()->response();

    if (!is_array($xml)) {
      $xml = trim(str_replace("\r\n", "\n", $xml));
    }
    else {
      if (!empty($xml['error'])) {
        watchdog('tpps', 'NCBI error: @error_message',
          ['@error_message' => $xml['error']], WATCHDOG_ERROR
        );
      }
      else {
        watchdog('tpps', 'NCBI response: <pre>@message</pre>',
          ['@message' => var_export($xml, 1)], WATCHDOG_ERROR
        );
      }
    }

    if ($debug_mode ?? FALSE) {
      dpm(static::printXml($xml));
    }
    // Process response.
    // The other pattern: '<OutputMessage>No items found.</OutputMessage>'.
    return (bool) preg_match('/<Count>1<\/Count>/', $xml);
  }

}
