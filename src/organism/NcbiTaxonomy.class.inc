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
   * Gets NCBI organism name by taxonomy id.
   *
   * @param int $taxonomy_id
   *   NCBI Taxonomy Id.
   *
   * @return string
   *   Returns NCBI Organism name.
   */
  public static function getName($taxonomy_id) {
    $url = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esummary.fcgi';
    $query = [
      'db' => 'taxonomy',
      'id' => $taxonomy_id,
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
    ];

    try {
      $xml = (new RestApi([
        'url' => $url,
        'query' => $query,
        'tag' => 'NCBI name by taxonomy id: ' . $taxonomy_id,
        'cache' => variable_get('tpps_organism_ncbi_use_cache', TRUE),
      ]))->get()->response();

      $xml = trim(str_replace("\r\n", "\n", $xml));
      preg_match('/<Item Name="ScientificName" Type="String">(.+)<\/Item>/', $xml, $matches);
      $organism_name = $matches[1] ?? '';
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
    return $organism_name;
  }

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
    $taxonomy_id = NULL;
    $url = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi';
    $formatted_organism_name = implode('+', explode(' ', $organism_name));
    $query = [
      'db' => 'taxonomy',
      'term' => "{$formatted_organism_name}[Scientific+Name]",
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
    ];

    try {
      $xml = (new RestApi([
        'url' => $url,
        'query' => $query,
        'tag' => 'NCBI Taxonomy: ' . $organism_name,
        'cache' => variable_get('tpps_organism_ncbi_use_cache', TRUE),
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

    return $taxonomy_id;
  }

}
