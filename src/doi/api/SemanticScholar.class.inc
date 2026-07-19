<?php

/**
 * @file
 * Contains 'SemanticScholar' class.
 */

/**
 * Gets DOI Publication data from SemanticScholar service.
 *
 * API Documentation:
 * https://api.semanticscholar.org/api-docs/datasets.
 *
 *
 *
 * https://api.semanticscholar.org/graph/v1/paper/DOI:10.1093/ageing/afaa119
 *
 * Usage:
 *   To get and save DOI Publication data:
 *   SemanticScholar::insert($doi);
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SemanticScholar extends DoiApi {

  // Unique key used to get Drupal variables and etc.
  // To get key by class name: $name = constant($class_name . '::KEY');
  // WARNING:
  // In DB table 'public.tpps_publication_doi.source' used class name
  // (not class's KEY constant).
  const KEY = 'semantic_scholar';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = 'api.semanticscholar.org';
  // IPv4 address.
  const HOST_IP = '';
  // Endpoint path (without leading and trailing slash).
  const ENDPOINT_PATH = 'graph/v1/paper';
  // Default delay between requests, in seconds.
  // @todo Use this delay.
  const DEFAULT_DELAY = 2;

  /**
   * Builds URL to get SERP.
   *
   * @return string
   *   Returns escaped URL.
   */
  public static function buildUrl($doi) {
    global $user;

    // Fields in the API response.
    $fields = [
      'title',
      'abstract',
      'authors',
      'venue',
      'citationCount',
      'journal',
      'publicationDate',
      //
      // Common fields you can request:
      // doi, title, abstract, url, venue, year
      // authors (includes name, authorId),
      // citations, references (each as a sub-graph of papers),
      // citationCount, referenceCount, influentialCitationCount,
      // isOpenAccess (with openAccessPdf URL),
      // publicationTypes, publicationDate, journal.
      // More fields:
      // https://api.semanticscholar.org/api-docs/graph#tag/Paper-Data/operation/get_graph_get_paper
    ];
    $params = [
      'query' => [
        'fields' => implode(',', $fields),
      ],
      'https' => TRUE,
    ];
    return url(static::getEndpointUrl() . '/DOI:' . $doi, $params);
  }

  /**
   * Request DOI Publication data using API and store in DB.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   */
  public static function insert($doi) {
    if (PublicationDoi::isExists($doi, static::class)) {
      return;
    }

    $response = static::request($doi);
    $response_code = $response['code'] ?? NULL;
    if (!empty($response_code) || ($response_code < 200 && $response_code > 205)) {
      // Do not insert unsuccess response.
      return;
    }

    // Create an empty record to avoid the same requests to API.
    $data = ['doi' => $doi, 'source' => static::class];
    if (
      ($response['error'] ?? '') != 'Paper with id DOI:' . $doi . ' not found'
      && !empty($response)
    ) {
      // Save in local DB.
      $author_list = [];
      foreach ($response['authors'] as $item) {
        // Example of the $item:
        // authorId: "1820966245",
        // name: "D. Attwood".
        if (!empty($item['name'])) {
          $author_list[] = $item['name'];
        }
      }
      $data = [
        'doi' => $doi,
        'source' => static::class,
        'authors' => implode(', ', $author_list),
        'date' => $response['publicationDate'] ?? $response['year'] ?? '',
        'title' => $response['title'] ?? '',
        'journal' => $response['venue'] ?? $response['journal']['name'] ?? '',
        'abstract' => trim($response['abstract'] ?? ''),
      ];
    }
    PublicationDoi::save($data);
  }

  /**
   * Request DOI Metadata.
   *
   * @param string $doi
   *   Digital Object Identifier (DOI).
   */
  public static function request($doi) {
    $url = static::buildUrl($doi);
    $header = [
      'Content-Type: application/json',
    ];
    try {
      $response = (new RestApi([
        'url' => $url,
        'header' => $header,
        'tag' => implode(', ', [$doi, static::class]),
      ]))->get()->response();
      return $response;
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

}
