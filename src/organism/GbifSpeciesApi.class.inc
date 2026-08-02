<?php

/**
 * @file
 * Contains static 'GbifSpeciesApi' class.
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
class GbifSpeciesApi {

  /**
   * Request Taxonomy Id for given organism name.
   *
   * @param string $organism
   *   Organism Name.
   */
  public static function getId($organism) {
    try {
      $response = (new RestApi([
        'url' => 'https://api.gbif.org/v1/species/match',
        'query' => ['name' => $organism],
        'tag' => 'GBIF Taxonomy Id: ' . $organism,
        'cache' => TRUE,
      ]))->get()->response();
      return $response['usageKey'] ?? NULL;
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
