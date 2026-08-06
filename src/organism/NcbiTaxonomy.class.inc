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
   * @return mixed
   *   Returns NCBI Organism short name (string).
   *   Returns FALSE when $taxonomy_id is empty.
   */
  public static function getName($taxonomy_id) {
    if (empty($taxonomy_id)) {
      return FALSE;
    }

    // Use Endpoint with full data.
    $full = TRUE;
    $query = [
      'db' => 'taxonomy',
      'id' => $taxonomy_id,
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
    ];

    if ($full) {
      $url = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi';
      $query['retmode'] = 'xml';
    }
    else {
      $url = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esummary.fcgi';
    }

    try {
      $xml = (new RestApi([
        'url' => $url,
        'query' => $query,
        'tag' => 'NCBI name by taxonomy id: ' . $taxonomy_id
          . ($full ? ' (full)' : ''),
        'cache' => variable_get('tpps_organism_ncbi_use_cache', TRUE),
      ]))->get()->response();

      $xml = trim(str_replace("\r\n", "\n", $xml));
      if ($full) {

        // Get short name.
        preg_match('/<ScientificName>(.+)<\/ScientificName>/', $xml, $matches);
        $name = $matches[1] ?? '';
        // Get full name.
        preg_match('/<DispName>(.+)<\/DispName>/', $xml, $matches);
        $full_name = $matches[1] ?? '';
      }
      else {
        preg_match('/<Item Name="ScientificName" Type="String">(.+)<\/Item>/', $xml, $matches);
        $name = $matches[1] ?? '';
        $full_name = NULL;
      }

      if (!empty($name) || !empty($full_name)) {
        db_update(static::TABLE)
          ->fields([
            'name' => $name,
            'full_name' => $full_name,
          ])
          ->condition('taxonomy_id', $taxonomy_id)
          ->execute();
      }
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
    return $name ?? NULL;
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
    if (is_numeric($organism_key)) {
      $organism_id = $organism_key;
      if (empty($organism_name = tpps_organism($organism_id)->name)) {
      }
    }
    else {
      $organism_name = $organism_key;
      if (empty($organism_id = tpps_organism($organism_name)->id)) {
      }
    }
    // Load.
    if (!empty($taxonomy_id = static::load($organism_id))) {
      return $taxonomy_id;
    }

    // Get.
    $taxonomy_id = NULL;
    $url = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi';
    $formatted_organism_name = implode('+', explode(' ', $organism_name));
    $query = [
      'db' => 'taxonomy',
      'term' => $organism_name,
      //'term' => "{$formatted_organism_name}[Scientific+Name]",
      'api_key' => variable_get('tripal_eutils_ncbi_api_key'),
    ];

    try {
      $xml = (new RestApi([
        'url' => $url,
        'query' => $query,
        'tag' => 'NCBI Taxonomy: ' . $organism_name,
        'cache' => variable_get('tpps_organism_ncbi_use_cache', TRUE),
      ]))->get()->response();
      if (!is_array($xml)) {
        $xml = trim(str_replace("\r\n", "\n", $xml));
        preg_match('/<Id>(\d+)<\/Id>/', $xml, $matches);
        $id = $matches[1] ?? '';
        $taxonomy_id = (empty($id) ? NULL : (int) $id);
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
      drupal_set_message($e->getCode() . ' ' . $message);
    }

    // Save relation.
    if (!empty($organism_id)) {
      static::save(
        $organism_id,
        $organism_name,
        $taxonomy_id
      );
    }
    static::getName($taxonomy_id);
    return $taxonomy_id;
  }

}
