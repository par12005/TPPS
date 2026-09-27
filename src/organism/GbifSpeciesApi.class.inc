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

  // Key used to label source of the data stored in database.
  // See class 'OrganismTaxonomy'.
  const KEY = 'gbif';

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
      $taxonomy = OrganismTaxonomy::load($organism->id, static::KEY);
      $taxonomy_id = $taxonomy[static::KEY][$taxonomy_id];
      if (!empty($taxonomy_id)) {
        if ($debug_mode ?? FALSE) {
          throw new Exception('Debug.');
        }
        $output = [
          'primary_name' => $organism_key,
          'taxonomy_id' => (int) $taxonomy_id,
        ];
        return $output;
      }
      else {
        $organism->load();
        $organism_key = $organism->name;
        throw new Exception('Taxonomy Id not found in local DB.');
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
        $response = static::getInfo($organism_key);
        if (($response['diagnostics']['matchType'] ?? NULL) == 'NONE') {
          return NULL;
        }
        else {
          // Item was found.
          // Note:
          // To get primary name for synonyms we could simply use
          // 'acceptedUsage' if it's present in the API-response.
          // When given primary name 'acceptedUsage' will be missing.
          // But current code is cleaner and self-descriptive.
          $match_type = $response['diagnostics']['matchType'] ?? NULL;
          switch ($match_type) {
            case 'HIGHERRANK':
            case 'EXACT':
              $status = $response['usage']['status'] ?? NULL;
              switch ($status) {
                case 'ACCEPTED':
                  // The name you searched for is the primary name.
                  // Note: $response['synonym'] == FALSE.
                  if ($response['usage']['rank'] == 'SPECIES') {
                    $output = [
                      'taxonomy_id' => (int) $response['usage']['key'] ?? NULL,
                      'primary_name' => $response['usage']['name'] ?? NULL,
                    ];
                  }
                  break;

                case 'SYNONYM':
                  // The name you searched for is a synonym.
                  // Note: $response['synonym'] == TRUE.
                  // Note:
                  // There is 'canonicalName' which seems to be a right choice
                  // but 'name' has infraspecific rank in it at correct place
                  // so we could just parse 'name' to save oranism name later
                  // instead of parsing 'rank: VARIETY' which could have
                  // different infraspecific rank.
                  $output = [
                    'taxonomy_id' => (int) $response['acceptedUsage']['key'] ?? NULL,
                    'primary_name' => $response['acceptedUsage']['name'] ?? NULL,
                  ];
                  break;
              }
              break;

            case 'FUZZY':
            case 'NONE':
            default:
              // The synonym wasn't resolved to a species.
              // Need a different strategy.
              return NULL;
          }
          // Get organism primary name.
          if (!empty($output['primary_name'])) {
            $organism = new Organism();
            $organism->setName($output['primary_name']);
            $organism->save();

            // Save relation between taxonomy_id and organism name.
            OrganismTaxonomy::save(
              $organism->id,
              static::KEY,
              $output['taxonomy_id'],
            );
          }
          return $output;
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
