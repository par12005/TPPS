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

  /**
   * Request NCBI Taxonomy Term Id for given organism name.
   *
   * @param string $organism_name
   *   Organism Name.
   *
   * @return int
   *   Returns NCBI Taxonomy Id.
   *
   * @throws Exception
   *   When request to NCBI API fails.
   */
  public static function getId($organism_name) {
    $organism_name = check_plain($organism_name);
    // Load.
    if (!empty($taxonomy_id = static::load($organism_name))) {
      return $taxonomy_id;
    }

    // Get.
    $url = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi';
    $formatted_organism_name = implode('+', explode(' ', $organism_name));
    $query = [
      'db' => 'taxonomy',
      'term' => "{$formatted_organis_name}[Scientific+Name]",
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
    ];

    try {
      $xml = (new RestApi([
        'url' => $url,
        'query' => $query,
        'tag' => 'NCBI Taxonomy: ' . $organism_name,
        'cache' => TRUE,
      ]))->get()->response();
      $xml = trim(str_replace("\r\n", "\n", $xml));
      preg_match('/<Id>(\d+)<\/Id>/', $xml, $matches);
      $id = $matches[1] ?? '';
      $taxonomy_id = (int) $id;
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

    // Save.
    static::save(
      tpps_organism($organism_name)->id,
      $organism_name,
      $taxonomy_id
    );
    return $taxonomy_id;
  }

}
