<?php

/**
 * @file
 * Contains DOI-related functions for TPPS.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/manage_doi');
 */

/**
 * Shows DOI of a TPPS submission.
 *
 * If no DOI exists for the submission, one is automatically generated.
 *
 * @param string $accession
 *   The accession number of the TPPS submission.
 *
 * @return string
 *   The HTML for the DOI page.
 */
function tpps_manage_doi($accession) {
  $output = "";

  $form_state = tpps_load_submission($accession);

  $token = variable_get('tpps_zenodo_api_key', NULL);
  if (!empty($token) and empty($form_state['zenodo_id'])) {
    $form_state['zenodo_id'] = tpps_generate_doi($accession)->id;
    $doi_object = tpps_doi_publish($form_state['zenodo_id']);
    tpps_update_submission($form_state);
    $output .= "<h4>DOI successfully published!</h4>";
  }
  elseif (!empty($token) and !empty($form_state['zenodo_id'])) {
    $doi_object = tpps_doi_load($form_state['zenodo_id']);
  }

  if (empty($token) or empty($doi_object) or (isset($doi_object->status) and $doi_object->status != 200)) {
    $output .= "<h4>Unable to load Zenodo DOI. There is likely a problem with the Zenodo API key. Please contact your system administrator.</h4>";
  }
  else {
    $doi = $doi_object->doi;
    $url = $doi_object->doi_url;
    $badge = $doi_object->links->badge;
    $output .= "DOI: <a target=\"blank\" href=\"$url\">$doi</a><br>"
    . "DOI Badge: <a target=\"blank\" href=\"$badge\"><img src=\"$badge\"></a><br>";
  }

  return render($output);
}


/**
 * Returns expression for matching DOIs.
 *
 * @return string
 *   The regular expression pattern for DOIs.
 */
function tpps_doi_regex() {
  return '/^10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+$/';
}

/**
 * Searches for DOI in already submitted studies.
 *
 * Note: Since caching enabled if multiple studies uses the same DOI (which
 * usually possible only for test studies) then latest study accession will
 * be used. Before first match was used.
 *
 * Total execution time:
 * Getting data from cache: 0.00034999847412109 sec
 * Empty cache or reset:    0.96340107917786 sec
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
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $use_cache = variable_set('tpps_page_1_cache_ajax_responses', TRUE);
  if ($reset || !$use_cache || empty($cache)) {
    $all_submissions = chado_query("SELECT * FROM public.tpps_submission;");
    foreach ($all_submissions as $item) {
      $form_state = unserialize($item->submission_state);
      $submission_doi = strtolower($form_state['saved_values']['1']['doi'] ?? NULL);
      if ($submission_doi) {
        $cache->data[$submission_doi] = $item->accession ?? NULL;
      }
    }
    cache_set($cid, $cache->data, $cache_bin);
  }
  return $cache->data[strtolower($doi)] ?? NULL;
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
 */
function tpps_doi_count_dbxref($doi, $reset = FALSE) {
  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $use_cache = variable_set('tpps_page_1_cache_ajax_responses', TRUE);
  if ($reset || !$use_cache ||  empty($cache) || empty($cache->data[strtolower($doi)])) {
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
 * @TODO Think about caching and/or sending to browser unchanged $data
 */
function tpps_get_doi_info($doi, $reset = FALSE) {
  $key = strtolower($doi);

  $cid = __FUNCTION__;
  $cache_bin = TPPS_CACHE_BIN ?? 'cache';
  $cache = cache_get($cid, $cache_bin);
  $use_cache = variable_set('tpps_page_1_cache_ajax_responses', TRUE);
  if ($reset || !$use_cache || empty($cache) || !isset($cache->data[$key])) {
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
        // ['second']['check'] = FALSE;
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
