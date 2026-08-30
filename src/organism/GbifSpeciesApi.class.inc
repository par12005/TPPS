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
    // Load existing Taxonomy Id (if any).
    if (!empty($taxonomy_id = static::load($organism_id))) {
      return $taxonomy_id;
    }

    // Get taxonomy id.
    $taxonomy_id = NULL;
    if (!empty($organism_name)) {

      try {
        $response = (new RestApi([
          'url' => 'https://api.gbif.org/v2/species/match',
          'query' => [
            'genericName' => static::prepareOrganismName($organism_name),
          ],
          'tag' => 'GBIF v2 Taxonomy Id: ' . $organism_name,
          'cache' => variable_get('tpps_organism_gbif_use_cache', TRUE),
        ]))->get()->response();
        $taxonomy_id = $response['usage']['key'] ?? NULL;
        $full_name = $response['usage']['name'] ?? NULL;
        $name = $response['usage']['canonicalName'] ?? NULL;
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

    // Save relation.
    if (!empty($organism_id)) {
      static::save(
        $organism_id,
        $organism_name,
        $taxonomy_id,
        $name ?? NULL,
        $full_name ?? NULL
      );
    }

    return $taxonomy_id;
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
    $words = explode(' ', $organism_name);
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

}
