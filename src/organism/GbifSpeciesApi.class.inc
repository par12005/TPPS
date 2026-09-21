<?php

/**
 * @file
 * Contains static 'GbifSpeciesApi' class.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Parent class 'OrganismTaxonomy' provides methods:
 * - getSchema(),
 * - save(),
 * - isExists().
 * - load()
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Species synonyms
 *
 * Stored in table 'chado.organismprop'
 * 'type_id' = 2780 which is cvterm 'synonym'.
 */

/**
 * Manages requests to GBIF Species API.
 *
 * See
 * https://techdocs.gbif.org/en/openapi/v1/species.
 *
 * V1: https://api.gbif.org/v1/species/match?name=Arabidopsis+thaliana
 * V2: https://api.gbif.org/v2/species/match?genericName=Arabidopsis+thaliana
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class GbifSpeciesApi extends OrganismTaxonomy {

  /**
   * Name of the table which stores GBIF Taxonomy Ids per organism.
   */
  const TABLE = 'tpps_gbif_taxonomy';

  /**
   * Name of the table which stores species synonyms provided by GBIFkkk.
   */
  const TABLE_SYNONYM = 'tpps_gbif_synonym';


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
    return 'https://www.gbif.org/species/' . $taxonomy_id;
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
    return url(
      'https://api.gbif.org/v2/species/match',
      ['query' => ['genericName' => $organism_name]]
    );
  }

  /**
   * Gets Taxonomy Id.
   *
   * @param mixed $organism_key
   *   Organism Name or Organism Id.
   *   AJAX-requests from browser uses name but Batch Update uses id.
   *
   * @return mixed
   *   Returns Taxonomy Id or NULL.
   */
  public static function getId($organism_key) {
    $organism_key = check_plain($organism_key);
    try {
      // Get taxonomy id from local DB.
      $organism = new Organism($organism_key);
      // Get previously saved taxonomy id from local DB.
      if (!empty($taxonomy_id = static::load($organism->id))) {
        return $taxonomy_id;
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
        $response = static::getInfo($organism_name);
        if (($response['diagnostics']['matchType'] ?? NULL) == 'NONE') {
          return NULL;
        }
        else {
          // Item was found.
          $taxonomy_id = $response['usage']['key'] ?? NULL;
          $organism_name = $response['usage']['canonicalName'] ?? NULL;

          // Get organism primary name.
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
          return $taxonomy_id;
        }
      }
    }
    return NULL;
  }

  /**
   * Changes organism name to improve search results.
   *
   * @param string $organism_name
   *   Full organism name.
   *
   * @return string
   *   Improved organism name.
   */
  public static function prepareOrganismName($organism_name) {
    // Remove Duplicate Genus.
    $words = explode(' ', check_plain($organism_name));
    // Get the first word (genus)
    $firstWord = $words[0];
    $result = [];
    foreach ($words as $index => $word) {
      if ($index === 0) {
        $result[] = $word;
      }
      elseif (strtolower($word) !== strtolower($firstWord)) {
        $result[] = strtolower($word);
      }
    }
    $organism_name = implode(' ', $result);
    return $organism_name;
  }

  /**
   * Gets list of synonyms.
   *
   * @param int $taxonomy_id
   *   GBIF Taxonomy Id.
   *
   * @return mixed
   *   Returns list of synonyms or NULL.
   */
  public static function getSynonymList($taxonomy_id) {
    if (empty($taxonomy_id)) {
      return NULL;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get synonym_list.
    $synonym_list = [];
    try {
      $url = 'https://api.gbif.org/v1/species/' . $taxonomy_id . '/synonyms';
      $synonym_response = (new RestApi(['url' => $url]))->get()->response();
      if (
        isset($synonym_response['results'])
        && !empty($synonym_response['results'])
      ) {
        foreach ($synonym_response['results'] as $synonym) {
          $synonym_list[] = $synonym['scientificName'];
        }
        return $synonym_list;
      }
      else {
        //echo "No synonyms found.\n";
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
    try {
      $response = (new RestApi([
        'url' => 'https://api.gbif.org/v2/species/match',
        'query' => [
          'genericName' => static::prepareOrganismName($organism_name),
        ],
        'tag' => 'GBIF v2 Taxonomy Id: ' . $organism_name,
        'cache' => variable_get('tpps_organism_gbif_use_cache', TRUE),
      ]))->get()->response();
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
    return $response;
  }

}
