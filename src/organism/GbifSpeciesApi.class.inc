<?php

/**
 * @file
 * Contains static 'GbifSpeciesApi' class.
 *
 * Parent class 'OrganismTaxonomy' provides methods:
 * - getSchema(),
 * - save(),
 * - isExists().
 * - load()
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
    // Load.
    if (!empty($taxonomy_id = static::load($organism_id))) {
      return $taxonomy_id;
    }

    // Get taxonomy id.
    $taxonomy_id = NULL;
    if (!empty($organism_name)) {

      $use_new_version = TRUE;
      try {
        if ($use_new_version) {
          $response = (new RestApi([
            'url' => 'https://api.gbif.org/v2/species/match',
            'query' => ['genericName' => $organism_name],
            'tag' => 'GBIF v2 Taxonomy Id: ' . $organism_name,
            'cache' => variable_get('tpps_organism_gbif_use_cache', TRUE),
          ]))->get()->response();
          $rank = $response['usage']['rank'] ?? NULL;
          if (!empty($rank) && in_array($rank, ['SPECIES', 'SUBSPECIES'])) {
            $taxonomy_id = $response['usage']['key'] ?? NULL;
            $full_name = $response['usage']['name'] ?? NULL;
            $name = $response['usage']['canonicalName'] ?? NULL;
          }
        }
        else {
          $response = (new RestApi([
            'url' => 'https://api.gbif.org/v1/species/match',
            'query' => ['name' => $organism_name],
            'tag' => 'GBIF v1 Taxonomy Id: ' . $organism_name,
            'cache' => variable_get('tpps_organism_gbif_use_cache', TRUE),
          ]))->get()->response();

          // @todo Test if it's correct field.
          $rank = $response['rank'] ?? NULL;
          if (!empty($rank) && in_array($rank, ['SPECIES', 'SUBSPECIES'])) {
            $taxonomy_id = $response['usageKey'] ?? NULL;
            $full_name = $response['name'] ?? NULL;
            $name = $response['canonicalName'] ?? NULL;
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

}
