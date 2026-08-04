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
   * Gets Taxonomy Id.
   *
   * @param string $organism_name
   *   Organism Name.
   *
   * @return mixed
   *   Returns Taxonomy Id or NULL.
   */
  public static function getId($organism_name) {
    $organism_name = check_plain($organism_name);
    // Load.
    if (!empty($taxonomy_id = static::load($organism_name))) {
      return $taxonomy_id;
    }

    // Get.
    $taxonomy_id = NULL;
    $use_new_version = TRUE;
    try {
      if ($use_new_version) {
        $response = (new RestApi([
          'url' => 'https://api.gbif.org/v2/species/match',
          'query' => ['genericName' => $organism_name],
          'tag' => 'GBIF v2 Taxonomy Id: ' . $organism_name,
          'cache' => FALSE,
        ]))->get()->response();
        if (in_array($response['usage']['rank'], ['SPECIES', 'SUBSPECIES'])) {
          $taxonomy_id = $response['usage']['key'] ?? NULL;
        }
      }
      else {
        $response = (new RestApi([
          'url' => 'https://api.gbif.org/v1/species/match',
          'query' => ['name' => $organism_name],
          'tag' => 'GBIF v1 Taxonomy Id: ' . $organism_name,
          'cache' => FALSE,
        ]))->get()->response();
        if ($response['rank'] == 'SPECIES') {
          $taxonomy_id = $response['usageKey'] ?? NULL;
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

    return $taxonomy_id;
  }

}
