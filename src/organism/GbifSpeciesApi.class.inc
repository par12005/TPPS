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
 * @todo Implement method which fills list of the accepted synonyms. Currently
 * only Batch Taxonomy Id Update does this.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class GbifSpeciesApi {

  // Key used to label source of the data stored in database.
  // See class 'OrganismTaxonomy'.
  const KEY = 'gbif';

  /**
   * Name of the list of the accapted synonym names in 'chado.organism' table.
   *
   * Class 'StateList' is using to manage this list.
   * This list is used for reports:
   * - /admin/reports/tpps/organism/synonym/ncbi/accepted.
   * - /admin/reports/tpps/organism/synonym/gbif/accepted.
   */
  const ACCEPTED_SYNONYM_LIST = 'organism_synonym_' . self::KEY . '_accepted_list';

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
   * @return array
   *   Returns associative arrayn with keys:
   *   - 'taxonomy_id': (int) Taxonomy id.
   *   - 'primary_name': (string|null) Primary (accepted) organism name.
   *   - 'primary_id': (int) Primary organism id.
   *
   * @throws Exception
   *   When request to GBIF API fails.
   */
  public static function getId($organism_key) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // The same code.
    $organism_key = check_plain($organism_key);
    try {
      // Get taxonomy id from local DB.
      $organism = new Organism($organism_key);
      $taxonomy = OrganismTaxonomy::load($organism->id, static::KEY);
      $taxonomy_id = $taxonomy[static::KEY] ?? NULL;
      if (!empty($taxonomy_id)) {
        if ($debug_mode ?? FALSE) {
          throw new Exception('Debug.');
        }
        $output = [
          'primary_name' => $organism->name,
          'primary_id' => $organism->id,
          'taxonomy_id' => (int) $taxonomy_id,
        ];
        return $output;
      }
      else {
        // If Taxonomy Id wasn't found we load organism to get it's id which
        // will be used later.
        $organism->load();
        $organism_id = $organism->id;
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
        $output = NULL;
        // Organism wasn't found in local DB but it's not a problem.
        $organism_name = $organism_key;
        // Let's search it in external taxonomy database.
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Below custom code.

        $response = static::getInfo($organism_name);
        if (($response['diagnostics']['matchType'] ?? NULL) == 'NONE') {
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          // Custom code ends.

          // Taxonomy Id not found.
          OrganismTaxonomy::save($organism->id, static::KEY, NULL);
          return NULL;
        }
        else {
          // Item was found.
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          // >>> Custom code starts.
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
                    $taxonomy_id = $response['usage']['key'] ?? NULL;
                    $primary_name = $response['usage']['name'] ?? NULL;
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
                  $taxonomy_id = $response['acceptedUsage']['key'] ?? NULL;
                  $primary_name = $response['acceptedUsage']['name'] ?? NULL;
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
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          // <<< Custom code ends.
          if (empty($primary_name)) {
            return NULL;
          }
          else {
            $primary_id = NULL;
            if ($organism_name == $primary_name) {
              $primary_id = $organism->id;
              StateList::remove(
                static::ACCEPTED_SYNONYM_LIST,
                $primary_id
              );
            }
            else {
              // Primary name and organism name (synonym) are different.
              $primary_organism = new Organism();
              $primary_organism->setName($primary_name);
              if (
                $organism->getShortName() == $primary_organism->getShortName()
              ) {
                // Short names are equal but full names (including authority,
                // year) are different.
                $primary_id = $organism->id;
                StateList::remove(
                  static::ACCEPTED_SYNONYM_LIST,
                  $primary_id
                );
              }
              else {
                // Primary name is totally different.
                $primary_organism->findId();
                if (empty($primary_organism->id)) {
                  if (
                    !empty($primary_organism->species)
                    && !empty($primary_organism->genus)
                  ) {
                    // No need to store if only $id property was set because
                    // it was already found in DB by findId() method.
                    $primary_organism->save();
                  }
                }
                $primary_id = $primary_organism->id ?? NULL;
                if (!empty($primary_id)) {
                  StateList::add(
                    static::ACCEPTED_SYNONYM_LIST,
                    // Value => Key,
                    // Synonym Id -> Primary Id.
                    (int) $primary_id,
                    (int) $organism->id
                  );
                }
              }
            }
            // Save relation between taxonomy_id and organism name.
            OrganismTaxonomy::save(
              $primary_id,
              static::KEY,
              $taxonomy_id
            );
          }
          $output = [
            'taxonomy_id' => $taxonomy_id,
            'primary_name' => $primary_name,
            'primary_id' => $primary_id,
          ];
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

  /**
   * Gets list of the accepted synonym names stored in 'chado.organism' table.
   *
   * @return array|false
   *   Returns list of organism ids.
   *   Returns FALSE if class 'StateList' not found.
   */
  public static function getAcceptedSynonymList() {
    if (class_exists('StateList')) {
      return StateList::getList(static::ACCEPTED_SYNONYM_LIST);
    }
    return FALSE;
  }

  /**
   * Clears list of the accepted synonyms.
   *
   * To generate this run Batch Taxonomy Id update.
   *
   * @return bool
   *   Returns TRUE if list was really cleared and FALSE otherwise.
   */
  public static function clearAcceptedSynonymList() {
    if (class_exists('StateList')) {
      StateList::clearList(static::ACCEPTED_SYNONYM_LIST);
      return TRUE;
    }
    return FALSE;
  }

}
