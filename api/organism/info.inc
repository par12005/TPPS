<?php

/**
 * @file
 * API Callback. Return's file info in JSON format.
 *
 *
 * URL: 'api/organism/%/%/info'.
 * Format: <genus>/<species>/info.
 */

/**
 * API Callback. Return's Organism's data in JSON format.
 *
 * @param string $genus
 *   Organism Genus.
 * @param string $species
 *   Organism Species.
 *
 * @return array
 *   Returns data from DB tabe 'chado.organism' related to given
 *   $genus and $species.
 */
function tpps_api_organism_info($genus, $species = NULL) {
  $query = db_select('chado.organism', 'o')->fields('o', []);
  if (is_numeric($genus) && is_null($species)) {
    $organism_id = $genus;
    $query->condition('organism_id', $organism_id);
    $result = $query->execute()->fetchAssoc();
  }
  else {
    $query->condition('genus', $genus)
      ->condition('species', $species)
      ->orderBy('organism_id', 'DESC');
    $result = $query->execute()->fetchAllAssoc('organism_id', PDO::FETCH_ASSOC);
  }
  return $result;
}
