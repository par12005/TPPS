<?php

/**
 * @file
 * Contains DOI-related functions for TPPS.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/manage_doi');
 *
 * @todo Convert to 'doi' class.
 */

/**
 * Returns expression for matching DOIs.
 *
 * @return string
 *   The regular expression pattern for DOIs.
 *
 * @TODO Move to includes/common.inc.
 */
function tpps_doi_regex() {
  return '/^10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+$/';
}

/**
 * Searches for DOI in already submitted studies.
 *
 * @param string $doi
 *   DOI value.
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 *
 * @return string
 *   Returns accession in format 'TGDRxxx'.
 *   Returns FALSE if DOI wasn't found.
 */
function tpps_doi_get_accession($doi, $reset = FALSE) {
  if (empty($doi)) {
    return FALSE;
  }
  $doi = strtolower($doi);
  $data = SubmissionCache::getAll($reset);
  foreach ($data as $accession => $cached_submission) {
    if (strtolower($cached_submission['publication']['publication_doi']) == $doi) {
      return $accession;
    }
  }
  return FALSE;
}

/**
 * Counts DOI in 'chado.dbxref' table.
 *
 * @param string $doi
 *   DOI value.
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 *
 * @return int
 *   Returns number of records for given DOI in 'chado.dbxref' table.
 *
 * @TODO Minor. Find better name for the function.
 */
function tpps_doi_count_dbxref($doi, $reset = FALSE) {
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $is_cache_allowed = variable_get('tpps_page_1_cache_ajax_responses', TRUE);
  if ($reset || !$is_cache_allowed ||  empty($cache) || empty($cache->data[strtolower($doi)])) {
    $result = db_select('chado.dbxref', 'd')
      ->condition('accession', $doi)
      ->execute()
      ->fetchAll();
    $cache->data[strtolower($doi)] = count($result);
    cache_set($cid, $cache->data, $cache_bin);
  }
  return $cache->data[strtolower($doi)] ?? NULL;
}

/**
 * Gets DOI Publication information from remote server dryad.
 *
 * Based on code from function tpps_doi_info($doi).
 *
 * @param string $doi
 *   Valid DOI.
 *
 * @return array
 *   Returns response which will be returned back as JSON. Keys:
 *   'error' bool TRUE means validation failed and FALSE that
 *
 * @TODO Minor. Rename function. E.g., 'tpps_publication_doi_get_info()'.
 * @todo Create class 'Dryad'.
 */
function tpps_get_doi_info($doi, $reset = FALSE) {

  $key = strtolower($doi);

  $cid = 'tpps_get_doi_info';
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $is_cache_allowed = variable_get('tpps_page_1_cache_ajax_responses', TRUE);

  if ($reset || !$is_cache_allowed || empty($cache) || !isset($cache->data[$key])) {
    $api_endpoint = variable_get('tpps_page_1_dryad_api_url',
      'http://datadryad.org/api/v2/datasets/');
    $url = $api_endpoint . urlencode('doi:' . $doi);
    // Note: json_decode() could throw an exception on invalid data.
    try {
      $content = file_get_contents($url);
      $data = json_decode($content);
    }
    catch (Exception $e) {
      return t('Caught exception: @message. Try again later.',
        ['@message' => $e->getMessage()]);
    }

    if (empty($data)) {
      // Fake DOI or not found. {"error":"not-found"}.
      $cache->data[$key] = NULL;
    }
    else {
      // Authors.
      foreach ($data->authors as $author) {
        $authors[] = trim($author->firstName . ' ' . $author->lastName);
      }
      $primary_author = $authors[0];
      $secondary_authors = array_slice($authors, 1);
      // Species.
      $species = [];
      foreach ($data->keywords as $keyword) {
        if (preg_match('/^([A-Z][a-z]* [a-z]+)$/', $keyword, $matches)) {
          $species[] = $matches[1];
        }
      }
      $info = [
        'title' => ((substr($data->title, 0, 11) == 'Data from: ')
          ? substr($data->title, 11) : $data->title),
        'primary' => $authors[0],
        'secondary' => $secondary_authors,
        'secondaryNumber' => count($secondary_authors),
        // @TODO [VS] Probably not used. Check!
        'secondaryCheck' => FALSE,
        'year' => explode('-', $data->publicationDate)[0],
        'abstract' => $data->abstract,
        'species' => $species,
        'speciesNumber' => count($species),
      ];
      $cache->data[$key] = $info;
    }
    if (variable_get('tpps_page_1_ajax_debug')) {
      $cache->data[$key]['data'] = $data;
      $cache->data[$key]['url'] = $url;
    }
    cache_set($cid, $cache->data, $cache_bin);
  }
  return $cache->data[$key] ?? [];
}
