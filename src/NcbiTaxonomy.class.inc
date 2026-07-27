<?php

/**
 * @file
 * Contains static 'NcbiTaxonomy' class.
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
class NcbiTaxonomy {

  /**
   * Request NCBI Taxonomy Term Id for given organism name.
   *
   * @param string $organism
   *   Organism Name.
   */
  public static function getId($organism) {
    $formatted_org = implode('+', explode(' ', $organism));

    $url = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi';
    $query = [
      'db' => 'taxonomy',
      'term' => "{$formatted_org}[Scientific+Name]",
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
    ];

    try {
      $xml = (new RestApi([
        'url' => $url,
        'query' => $query,
        'tag' => 'NCBI Taxonomy: ' . $organism,
        'cache' => TRUE,
      ]))->get()->response();
      $xml = trim(str_replace("\r\n", "\n", $xml));
      preg_match('/<Id>(\d+)<\/Id>/', $xml, $matches);
      $id = $matches[1] ?? '';
      return (int) $id;
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
