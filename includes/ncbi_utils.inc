<?php

/**
 * @file
 * Defines EUtils functions for NCBI's EUtils API.
 */

/**
 * Retrieves taxonomy information based on an organism name.
 *
 * @param string $organism
 *   The name of the organism.
 *   E.g., 'Gymnocarpos przewalskii'.
 * @param string $rank
 *   The taxonomic rank (subkingdom, order, family, etc).
 *
 * @return mixed
 *   The name of the family or FALSE on failure.
 */
function tpps_ncbi_get_taxonomy($organism, $rank) {
  // Subkingdom is a little different.
  if ($rank == 'subkingdom') {
    return tpps_ncbi_get_subkingdom($organism);
  }

  try {
    $id = tpps_ncbi_get_taxon_id($organism);
  }
  catch (Exception $e) {
    $message = t("Can't get data from NCBI database: @message",
      ['@message' => $e->getMessage()]);
    drupal_set_message($message);
    return FALSE;
  }
  $fetch = new EFetch('taxonomy');
  $fetch->addParam('id', "$id");
  $response = $fetch->get()->xml();
  if (empty($response->Taxon->LineageEx->Taxon)) {
    $message = t('Unable to load Lineage information from NCBI for organism: @org',
      ['@org' => $organism]);
    drupal_set_message($message);
    return FALSE;
  }

  $lineage = $response->Taxon->LineageEx->Taxon;
  foreach ($lineage as $item) {
    if ($item->Rank == $rank) {
      return $item->ScientificName;
    }
  }
  $message = t('Unable to determine @rank from NCBI for organism: @organism',
    ['@rank' => $rank, '@organism' => $organism]);
  drupal_set_message($message);
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
  try {
    $id = tpps_ncbi_get_taxon_id($organism);
  }
  catch (Exception $e) {
    $message = t("Can't get data from NCBI database: @message",
      ['@message' => $e->getMessage()]);
    drupal_set_message($message);
    return FALSE;
  }

  $fetch = new EFetch('taxonomy');
  $fetch->addParam('id', "$id");
  $response = $fetch->get()->xml();
  if (empty($response->Taxon->Lineage)) {
    $message = t('Unable to load Lineage information from NCBI for organism: @org',
      ['@org' => $organism]
    );
    drupal_set_message($message);
    return FALSE;
  }

  $lineage = $response->Taxon->Lineage;
  if (preg_match('/(angiosperm|gymnosperm)/', $lineage, $matches)) {
    if (!empty($matches[1])) {
      return $matches[1];
    }
  }
  $message = t('Unable to determine subkingdom from NCBI for organism: @org',
    ['@org' => $organism]
  );
  drupal_set_message($message);
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
 *
 * @thrown Exception
 *   When all attempts to get data from NCBI failed.
 */
function tpps_ncbi_get_taxon_id($organism, $list = FALSE, $level = 1) {
  $formatted_org = implode('+', explode(' ', $organism));
  $search = new ESearch('taxonomy');
  $search->addParam('term', "{$formatted_org}[Scientific+Name]");

  $err_msg = 'Unable to load Taxon ID from NCBI for organism named: ' . $organism;
  // Delay between requests in milliseconds.
  $delay = variable_get('tpps_ncbi_api_delay', 300);
  // Halt time in microseconds. A microsecond is one millionth of a second.
  usleep($level * $delay * 1000);
  try {
    $response = $search->get();
    if (!$response->isSuccessful()) {
      // Fail. Try again up to 5 times with dynamically increased delay.
      if ($level < variable_get('tpps_ncbi_api_max_attempt_number', 5)) {
        $level++;
        try {
          return tpps_ncbi_get_taxon_id($organism, $list, $level);
        }
        catch (Exception $e) {
          throw new Exception($e->getMessage(), $e->getCode());
        }
      }
      else {
        $message = t('Reached the limit of requests to NCBI API. Increase NCBI API Request Delay.');
        watchdog('tpps', $message, [], WATCHDOG_WARNING);
        throw new Exception($message);
        return FALSE;
      }
    }
    // Success.
    // Parse XML ONLY (!) when we've got correct answer.
    $xml = $response->xml();
    if (empty($xml->IdList->Id)) {
      watchdog('tpps', $err_msg, [], WATCHDOG_ERROR);
      throw new Exception($err_msg);
      //drupal_set_message($err_msg);
      return FALSE;
    }
    if (!$list) {
      return $xml->IdList->Id;
    }
    return $xml->IdList;
  }
  catch (\Exception $e) {
    $message = $err_msg . ' EUtils Error: ' . $e->getMessage();
    watchdog('tpps', $message, [], WATCHDOG_ERROR);
    throw new Exception($message, $e->getCode());
  }
}

/**
 * A relaxed version used for testing: Retrieves the NCBI Taxonomy Id of an organism based on its name.
 *
 * WARNING:
 * This function is outdated (see mulile request and delay in implementation
 * of the tpps_ncbi_get_taxon_id() and seems not used by this module anymore.
 *
 * @param string $organism
 *   The name of the organism.
 *
 * @return mixed
 *   The NCBI Taxonomy Id, or FALSE on failure.
 *
 * @TODO Check if there are any calls outside module and remove if possible.
 */
function tpps_ncbi_get_taxon_id_v2($organism, $list = FALSE) {
  if (variable_get('tpps_block_function_tpps_ncbi_get_taxon_id_v2', TRUE)) {
    return;
  }

  $formatted_org = implode('+', explode(' ', $organism));
  // $formatted_org = implode(' ', explode(' ', $organism));
  echo "NCBI Search String Formatted: $formatted_org\n";
  // $search = new ESearch('taxonomy');
  // $search->addParam('term', "$formatted_org");
  $response_raw = file_get_contents("https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi?db=taxonomy&term=" . $formatted_org);
  print_r($response_raw);
  $err_msg = 'Unable to load Taxon ID from NCBI for organism named: ' . $organism;
  try {
    // $response = $search->get()->xml();
    $response = simplexml_load_string($response_raw);
    print_r($response);
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
