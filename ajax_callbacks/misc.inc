<?php

/**
 * @file
 * Misc. AJAX-callbacks.
 *
 * See /tpps.ajax.inc.
 *
 * How to add new AJAX-callback?
 * 1. Add new function with 'tpps_ajax_' prefix.
 * 2. Use function name withtout prefix 'tpps_ajax_' at client side.
 *
 * Example:
 * function: tpps_ajax_save_google_scholar_serp()
 * URL: /ajax/tpps/save_google_scholar_serp
 */

/**
 * Checks if DOI is unique and not used by other studies.
 *
 * Note: There is no need to cache check results because both functions
 * called from this AJAX-callback cache their results and results format
 * are different so it's better to check them separately.
 *
 * @param array $data
 *   POST data.
 *
 * @return array
 *   Returns response which will be returned back as JSON. Keys:
 *   'error' bool TRUE means validation failed and FALSE that
 */
function tpps_ajax_get_doi($data = NULL) {
  if (empty($data)) {
    // Empty request. Expected DOI value.
    return ['errors' => [t('Empty POST requrest.')], 'success' => FALSE];
  }
  // Validate.
  // Note: Could be used for 'Publication DOI' and 'Dataset DOI' so key is 'doi'.
  $doi = $data['doi'] ?? NULL;
  if (empty($doi)) {
    return ['errors' => [t('Empty DOI.')], 'success' => FALSE];
  }
  $doi = DOI::searchFirst($doi);
  $params = ['!doi_link' => DOI::print($doi)];
  if (!DOI::isValid($doi)) {
    $message = 'DOI !doi_link has invalid format. Example DOI: 10.NNNN/dryad.NNN';
    $response['errors'][] = t($message, $params);
  }
  else {
    $response['statuses'][] = t('DOI !doi_link is valid.', $params);
    // Check if DOI in use.
    $doi_check_source = variable_get(
      'tpps_page_1_doi_check_source',
      'public.tpps_submissions'
    );
    if ($doi_check_source == 'public.tpps_submissions') {
      if ($accession = DOI::getAccession($doi)) {
        $response['warnings'][]
          = t('WARNING: DOI !doi_link is already used by @accession.',
            array_merge($params, ['@accession' => $accession])
          );
      }
      else {
        $response['statuses'][] = t('DOI !doi_link is not in use.', $params);
      }
    }
    else {
      if (DOI::countDbxref($doi) == 0) {
        $response['statuses'][] = t('DOI !doi_link is not in use.', $params);
      }
      else {
        $response['warnings'][]
          = t('DOI !doi_link is used by another submission.', $params);
      }
    }
    // DOI is valid so we could try to get data.
    $response['doi_info'] = Dryad::getInfo($doi);
    if (!empty($response['doi_info'])) {
      $response['success'] = TRUE;
    }
    else {
      $response['success'] = FALSE;
      $response['warnings'][]
        = t('DOI !doi_link: no publication information found.', $params);
    }
  }
  return $response;
}

/**
 * Get NCBI Taxonomy Id.
 *
 * Wrapper for the function tpps_ncbi_get_taxon_id().
 *
 * @param array $data
 *   POST data with required 'organism' item which must contain organism name.
 *
 * @return array
 *   Returns array with data to be sent back to browser.
 */
function tpps_ajax_get_ncbi_taxonomy_id(array $data) {
  try {
    $xml = tpps_ncbi_get_taxon_id($data['organism']);
  }
  catch (Exception $e) {
    $response['id'] = FALSE;
    $response['success'] = FALSE;
    $response['errors'][] = t("Can't get data from NCBI database.");
    return $response;
  }
  // Convert Simple XML Object into PHP's array.
  $ncbi_response = json_decode(json_encode($xml), TRUE);
  $response['id'] = check_plain($ncbi_response[0]) ?? FALSE;
  if (empty($response['id'])) {
    $response['success'] = FALSE;
    $response['warnings'][] = t('Organism is missing in NCBI database.');
  }
  else {
    $response['success'] = TRUE;
    // Build link to NCBI Taxonomy Browser page.
    $url = 'https://www.ncbi.nlm.nih.gov/Taxonomy/Browser/wwwtax.cgi';
    $params = [
      'absolute' => TRUE,
      'query' => ['id' => $response['id']],
      'external' => TRUE,
      'attributes' => [
        'target' => '_blank',
        'title' => t('View using NCBI Taxonomy Browser'),
      ],
    ];
    $response['statuses'][] = t('Organism was found in NCBI database. '
      . 'Taxonomy Id: !id',
      ['!id' => l($response['id'], $url, $params)]
    );
  }
  return $response;
}

/**
 * AJAX callback for 'Remember Fieldset Collapsed State' feature.
 *
 * Stores state of given fieldset.
 *
 * @param array $data
 *   POST data.
 *
 * @return bool
 *   Returns TRUE in case of success and FALSE otherwise.
 */
function tpps_ajax_remember_fieldset_collapsed_state(array $data) {
  if (
    empty($data['name'])
    && empty($data['form_id'])
    && !isset($data['value'])
  ) {
    return FALSE;
  }
  $_SESSION[TPPS_RFCS_KEY][$data['form_id']][$data['name']] = (int) $data['value'];
  return TRUE;
}

/**
 * AJAX callback for 'Get publication data by DOI'.
 *
 * @param array $payload
 *   POST payload. Required keys are: 'doi'.
 *   See tpps_ajax_callback().
 *   Keys:
 *     'doi' string.
 *       DOI.
 *     'source' string
 *       Optional. Possible values are:
 *       'all', or data provider (DoiSerp::KEY, DoiAPI::KEY)
 *
 * @return array
 *   Returns response.
 *   Key 'success' will have value TRUE in case of success and FALSE otherwise.
 */
function tpps_ajax_get_publication_data(array $payload) {
  $response['success'] = FALSE;
  if (empty($payload['doi'])) {
    $response['errors'][] = t('Empty DOI.');
  }
  else {
    $response['doi'] = $doi = $payload['doi'];
    $response['source'] = $souce = $payload['source'] ?? NULL;
    $response['publication_data'] = PublicationDOI::requestAPI($doi, $source);
    $response['is_complete'] = TppsArray::isComplete(
      $response['publication_data']
    );
    $response['success'] = TRUE;
  }
  return $response;
}

/**
 * AJAX callback for 'Save Google Scholar SERP'.
 *
 * @param array $payload
 *   POST payload. Required keys are: 'serp'.
 *   See tpps_ajax_callback().
 *
 * @return array
 *   Returns response.
 *   Key 'success' will have value TRUE in case of success and FALSE otherwise.
 */
function tpps_ajax_save_google_scholar_serp(array $payload) {
  if (empty($payload['doi'])) {
    $response['errors'][] = t('Empty DOI.');
  }
  elseif (empty($payload['serp'])) {
    $response['errors'][] = t('Empty SERP.');
  }
  elseif (empty($payload['source'])) {
    $response['errors'][] = t('Empty source.');
  }
  else {
    $doi = $payload['doi'];
    $provider = new GoogleScholar();
    // Skip loading serp from database even if it exists.
    $provider->doi = $doi;
    $provider->serp = base64_decode($payload['serp']);
    if (
      strpos($provider->serp, 'Not found because of proxy error') === FALSE
      && strpos($provider->serp, '404 Not Found') === FALSE
    ) {
      $provider->save();
      // Parse using Gemini AI.
      $provider->parse();

      $response['doi'] = $payload['doi'];
      $response['source'] = $payload['source'];
      $response['publication_data'] = PublicationDOI::load($doi, $payload['source']);
    }
    else {
      $response['errors'][] = t('Proxy error.');
    }
  }
  return $response;
}

/**
 * AJAX callback for 'Save Google Scholar SERP'.
 *
 * @param array $payload
 *   POST payload. Required keys are: 'serp'.
 *   See tpps_ajax_callback().
 *
 * @return array
 *   Returns response.
 *   Key 'success' will have value TRUE in case of success and FALSE otherwise.
 */
function tpps_ajax_save_pubmed_serp(array $payload) {
  if (empty($payload['doi'])) {
    $response['errors'][] = t('Empty DOI.');
  }
  elseif (empty($payload['serp'])) {
    $response['errors'][] = t('Empty SERP.');
  }
  elseif (empty($payload['source'])) {
    $response['errors'][] = t('Empty source.');
  }
  else {
    $doi = $payload['doi'];
    $provider = new PubMed($doi);
    $provider->serp = base64_decode($payload['serp']);

    if (
      strpos($provider->serp, 'Not found because of proxy error') === FALSE
      && strpos($provider->serp, '404 Not Found') === FALSE
    ) {
      $provider->save();
      // Parse using Gemini AI.
      $provider->parse();

      $response['doi'] = $payload['doi'];
      $response['source'] = $payload['source'];
      $response['publication_data'] = PublicationDOI::load($doi, $payload['source']);
    }
    else {
      $response['errors'][] = t('Proxy error.');
    }
  }
  return $response;
}
