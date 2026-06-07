<?php

/**
 * @file
 * Contains 'TppsAjax' class.
 */

/**
 * Collection of the AJAX callbacks.
 *
 * See tpps_ajax_callback().
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class TppsAjax {

  /**
   * AJAX callback for 'Get publication data by DOI'.
   *
   * @param array $payload
   *   POST payload. Required keys are: 'doi'.
   *   Keys:
   *     'doi' string.
   *       DOI.
   *     'source' string
   *       Optional. Possible values are:
   *       'all', or data provider (DoiSerp::KEY, DoiAPI::KEY)
   *
   * @return array
   *   Returns response.
   *   Key 'success' will have value TRUE in case of success
   *   and FALSE otherwise.
   */
  public static function getPublicationData(array $payload) {
    $response['success'] = FALSE;
    if (empty($payload['doi'])) {
      $response['errors'][] = t('Empty DOI.');
    }
    else {
      $response['doi'] = $doi = $payload['doi'];
      $response['source'] = $source = $payload['source'] ?? NULL;
      $response['publication_data'] = PublicationDOI::requestAPI($doi, $source);
      $response['is_complete'] = (
        is_array($response['publication_data'])
        && TppsArray::isComplete($response['publication_data'])
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
  public static function saveGoogleScholarSerp(array $payload) {
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
  public static function savePubMedSerp(array $payload) {
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

}
