<?php

/**
 * @file
 * Contains static 'PubMedCentral' class.
 *
 * PMC OAI-PMH API
 * The PubMed Central OAI-PMH API provides access to metadata of all items
 * in PubMed Central (PMC), as well as to the full text of articles with
 * licenses or usage rights that allow for reuse.
 *
 * Note: We also have /serp/PubMed.class.inc which parses SERP page.
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
class PubMedCentral extends DoiApi {

  // Unique key used to get Drupal variables and etc.
  const KEY = 'pubmed_central';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = 'pmc.ncbi.nlm.nih.gov';
  // IPv4 address.
  const HOST_IP = '2600:1901:0:c831::';
  // Endpoint path (without leading and trailing slash).
  const ENDPOINT_PATH = 'api/oai/v1/mh';
  // Default delay between requests, in seconds.
  // @TODO Use this delay.
  const DEFAULT_DELAY = 2;

  /**
   * Request DOI Publication data using API and store in DB.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   */
  public static function insert($doi) {
    if (PublicationDOI::isExists($doi, static::KEY)) {
      return;
    }
    // Note: $response contains XML string.
    $xml = static::request($doi);
    if (
      !empty($xml)
      && strpos($xml, 'Identifier does not exist.') !== FALSE
    ) {
      $data = static::parseXmlResponse($xml);
      // Create an empty record to avoid the same requests to API.
      $data['doi'] = $doi;
      $data['source'] = static::KEY;
      PublicationDOI::save($data);
    }
    elseif (is_null($xml)) {
      //PublicationDOI::save(['doi' => $doi, 'source' => static::KEY]);
    }
  }

  /**
   * Get response from remote API to get DOI Metadata.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   *
   * @return object
   *   Returns FALSE if DOI was invalid or ID not found.
   *   Returns NULL if there is no data for given DOI.
   *   Returns Simple XML Element.
   */
  public static function request($doi) {
    $url = static::buildUrl($doi);
    if (is_null($url)) {
      return NULL;
    }
    elseif ($url === FALSE) {
      return FALSE;
    }
    else {
      try {
        $xml = (new RestAPI(['url' => $url]))->get()->response;
        return $xml;
      }
      catch (Exception $e) {
        // @TODO Minor. Code 429 (Too many requiests).
        $message = $e->getMessage();
        $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
        watchdog('tpps', '@message <pre>@backtrace</pre>',
          ['@message' => $message, '@backtrace' => print_r($stack, 1)],
          WATCHDOG_ERROR
        );
        drupal_set_message($e->getCode() . ' ' . $message);
      }
    }
  }

  /**
   * Builds URL for API Endpoint.
   *
   * To build URL which gets XML we need to convert DOI into PMCID first
   * so this method makes extra request to API.
   * If PMCID not found makes empty record in database to avoid the same requests.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   *
   * @return mixed
   *   Returns URL.
   *   Returns NULL if PMC Id wasn't found and URL can't be generated.
   *   Returns FALSE in case of errors.
   *
   * @throws Exception
   *   1. If DOI is invalid and PubMedCentral Id can't be obtained.
   *   2. If Id wasn't found in the response (empty).
   */
  public static function buildUrl($doi) {
    global $user;
    $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);

    // ########################################################################
    // Step 1. Convert DOI → PMCID number.
    $pmcid = static::getPmcId($doi);
    // No Id could be caused invalid DOI or missing record at remote API server.
    if (is_null($pmcid)) {
      // Create an empty record in DB to avoid multiple requests.
      if (!PublicationDOI::isExists($doi, static::KEY)) {
        PublicationDOI::save(['doi' => $doi, 'source' => static::KEY]);
      }
      return NULL;
    }
    elseif ($pmcid === FALSE) {
      watchdog('tpps', "@key: PMC ID not found for DOI '@doi'.",
        ['@doi' => $doi, '@key' => static::KEY],
        WATCHDOG_ERROR
      );
      return FALSE;
    }
    elseif (empty($pmcid)) {
      watchdog('tpps', "@key: Empty PMC ID for @doi.",
        ['@key' => static::KEY, '@doi' => $doi],
        WATCHDOG_ERROR
      );
      return FALSE;
    }
    // ########################################################################
    // Step 2. Build Endpoint URL.
    // PMCID exists. For next step we need only numbers (not letters).
    if (!empty($id = str_replace('PMC', '', $pmcid))) {
      return url(static::getEndpointUrl() . '/', [
        'https' => TRUE,
        'absolute' => TRUE,
        'query' => [
          'verb' => 'GetRecord',
          'identifier' => 'oai:pubmedcentral.nih.gov:' . $id,
          'metadataPrefix' => 'oai_dc',
        ],
      ]);
    }
  }

  /**
   * Parse XML Response.
   *
   * @param string $xmlString
   *   XML string to be parsed.
   *
   * @return array
   *   Returns parsed DOI Publication data.
   */
  public static function parseXmlResponse($xmlString) {
    $dom = new DOMDocument();
    $dom->loadXML($xmlString);
    $xpath = new DOMXPath($dom);

    // Register namespaces.
    $xpath->registerNamespace('oai', 'http://www.openarchives.org/OAI/2.0/');
    $xpath->registerNamespace('dc', 'http://purl.org/dc/elements/1.1/');
    $result = [
      'title' => $xpath->evaluate('string(//dc:title)'),
      'journal' => $xpath->evaluate('string(//dc:source)'),
      'date' => str_replace('-', '/', $xpath->evaluate('string(//dc:date)')),
      'abstract' => $xpath->evaluate('string(//dc:description)'),
      'authors' => [],
    ];
    // Get all authors.
    $authors = [];
    $authorNodes = $xpath->query('//dc:creator');
    foreach ($authorNodes as $author) {
      if (strpos($author->nodeValue, ', ') !== FALSE) {
        list($last_name, $first_name) = explode(', ', $author->nodeValue);
        $authors[] = $first_name . ' ' . $last_name;
      }
      else {
        $authors[] = $author->nodeValue;
      }
    }
    $result['authors'] = implode(', ', $authors);
    return $result;
  }

  /**
   * Generates URL to convert DOI into PMC Id.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   *
   * @return string
   *   Returns URL.
   */
  public static function buildSearchUrl($doi) {
    $host = static::HOST_NAME;
    if (variable_get('tpps_doi_' . static::KEY . '_use_ip', FALSE)) {
      $host = static::HOST_IP;
    }
    return url('https://' . $host . '/tools/idconv/api/v1/articles/', [
      'query' => [
        'ids' => $doi,
        'format' => 'json',
        'tool' => 'Drupal CMS',
        'email' => $user->mail ?? variable_get('site_mail', 'test.user@gmail.com'),
      ],
    ]);
  }

  /**
   * Gets PMC Id.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   *
   * @return mixed
   *   Returns PMC Id.
   *   Returns NULL if id was not found.
   *   Returns FALSE in case of errors.
   */
  public static function getPmcId($doi) {
    $response = (new RestAPI([
      'url' => static::buildSearchUrl($doi),
      'header' => ['Content-Type: application/json'],
    ]))->get()->response;

    if (!empty($response['records'][0]['pmcid'])) {
      return $response['records'][0]['pmcid'];
    }
    elseif (($response['records'][0]['status'] ?? '') == 'error') {
      $error_message = ($response['records'][0]['errmsg'] ?? '');
      if ($error_message == 'Identifier not found in PMC') {
        // Returning NULL we create an empty record in DB.
        return NULL;
      }
      else {
        return FALSE;
      }
    }
    return NULL;
  }

}
