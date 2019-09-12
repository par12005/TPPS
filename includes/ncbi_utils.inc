<?php

/**
 * 
 */

/**
 * 
 */
function tpps_ncbi_get_family($organism) {
  $id = tpps_ncbi_get_taxon_id($organism);
  $response = tpps_ncbi_fetch($id, 'taxonomy');
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
 * 
 */
function tpps_ncbi_get_subkingdom($organism) {
  $id = tpps_ncbi_get_taxon_id($organism);
  $response = tpps_ncbi_fetch($id, 'taxonomy');
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
 * 
 */
function tpps_ncbi_get_taxon_id($organism) {
  $formatted_org = implode('+', explode(' ', $organism));
  $url = tpps_ncbi_url('search', array(
    'db' => 'taxonomy',
    'term' => "{$formatted_org}[Scientific+Name]",
  ));

  $err_msg = 'Unable to load Taxon ID from NCBI for organism named: ' . $organism;
  try {
    $response = simplexml_load_file($url);
    if (empty($response->IdList->Id)) {
      drupal_set_message($err_msg);
      return FALSE;
    }
    return $response->IdList->Id;
  }
  catch (\Exception $e) {
    drupal_set_message($err_msg . ' EUtils Error: ' . $e->getMessage());
  }
}

/**
 * 
 */
function tpps_ncbi_fetch($id, $db) {
  $url = tpps_ncbi_url('fetch', array(
    'db' => $db,
    'id' => $id,
  ));

  $err_msg = "Unable to fetch from NCBI with database: $db and id: $id";
  try {
    $response = simplexml_load_file($url);
    if (empty($response)) {
      drupal_set_message($err_msg);
      return FALSE;
    }
    return json_decode(json_encode($response));
  }
  catch (\Exception $e) {
    drupal_set_message($err_msg . ' EUtils Error: ' . $e->getMessage());
  }
}

/**
 * 
 */
function tpps_ncbi_url($type, $args) {
  $url = TPPS_NCBI_EUTILS_BASE . "e{$type}.fcgi?";

  $api_key = variable_get('tpps_ncbi_api_key', NULL);
  if (!empty($api_key)) {
    $url .= "api_key=$api_key";
  }

  foreach ($args as $key => $val) {
    $url .= "&$key=$val";
  }
  return $url;
}
