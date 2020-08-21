<?php

/**
 * @file
 * Defines EUtils functions for NCBI's EUtils API.
 */

/**
 * Retrieves a family based on an organism name.
 *
 * @param string $organism
 *   The name of the organism.
 *
 * @return mixed
 *   The name of the family or FALSE on failure.
 */
function tpps_ncbi_get_family($organism) {
  $id = tpps_ncbi_get_taxon_id($organism);
  $fetch = new EFetch('taxonomy');
  $fetch->addParam('id', "$id");
  $response = $fetch->get()->xml();
  if (empty($response->Taxon->LineageEx->Taxon)) {
    drupal_set_message('Unable to load Lineage information from NCBI for organism: ' . $organism);
    return FALSE;
  }

  $lineage = $response->Taxon->LineageEx->Taxon;
  foreach ($lineage as $item) {
    if ($item->Rank == 'family') {
      return $item->ScientificName;
    }
  }
  drupal_set_message('Unable to determine family from NCBI for organism: ' . $organism);
  return FALSE;
}

/**
 * Retrieves a subkingdom based on an organism name.
 *
 * @param string $organism
 *   The name of the organism.
 *
 * @return mixed
 *   The name of the subkingdom or FALSE on failure.
 */
function tpps_ncbi_get_subkingdom($organism) {
  $id = tpps_ncbi_get_taxon_id($organism);
  $fetch = new EFetch('taxonomy');
  $fetch->addParam('id', "$id");
  $response = $fetch->get()->xml();
  if (empty($response->Taxon->Lineage)) {
    drupal_set_message('Unable to load Lineage information from NCBI for organism: ' . $organism);
    return FALSE;
  }

  $lineage = $response->Taxon->Lineage;
  if (preg_match('/(angiosperm|gymnosperm)/', $lineage, $matches)) {
    if (!empty($matches[1])) {
      return $matches[1];
    }
  }
  drupal_set_message('Unable to determine subkingdom from NCBI for organism: ' . $organism);
  return FALSE;
}

/**
 * Retrieves the NCBI Taxonomy Id of an organism based on its name.
 *
 * @param string $organism
 *   The name of the organism.
 *
 * @return mixed
 *   The NCBI Taxonomy Id, or FALSE on failure.
 */
function tpps_ncbi_get_taxon_id($organism, $list = FALSE) {
  $formatted_org = implode('+', explode(' ', $organism));
  $search = new ESearch('taxonomy');
  $search->addParam('term', "{$formatted_org}[Scientific+Name]");

  $err_msg = 'Unable to load Taxon ID from NCBI for organism named: ' . $organism;
  try {
    $response = $search->get()->xml();
    if (empty($response->IdList->Id)) {
      drupal_set_message($err_msg);
      return FALSE;
    }
    if (!$list) {
      return $response->IdList->Id;
    }
    return $response->IdList;
  }
  catch (\Exception $e) {
    drupal_set_message($err_msg . ' EUtils Error: ' . $e->getMessage());
  }
}
