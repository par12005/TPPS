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
      $publication_data = PublicationDOI::getCombined($doi);
      $response['publication_data'] = $publication_data;
      $response['is_complete'] = PublicationDOI::isComplete($publication_data);
      $response['success'] = TRUE;
    }
    return $response;
  }

  /**
   * AJAX callback for 'Save DOI SERP'.
   *
   * Saves SERP source code into db table per SERP provider.
   * Parses source code using Gemini AI and stores DOI Publication Data
   * into db table 'public.tpps_publication_doi'.
   *
   * @param array $payload
   *   POST payload. Required keys are: 'serp'.
   *   See tpps_ajax_callback().
   *
   * @return array
   *   Returns DOI Publication Data in item with key 'publication_data'.
   *   Other keys and their values:
   *   'doi' (string) - DOI
   *   'success' (bool) - TRUE in case of success and FALSE otherwise.
   *   'source' (string) - source of the DOI Publication data.
   */
  public static function saveSerp(array $payload) {
    if (empty($payload['doi'])) {
      $response['errors'][] = t('Empty DOI.');
    }
    elseif (empty($payload['serp'])) {
      $response['errors'][] = t('Empty SERP.');
    }
    elseif (empty($payload['provider_class_name'])) {
      $response['errors'][] = t('Empty SERP provider class name.');
    }
    else {
      $doi = $payload['doi'];
      $provider_class_name = $payload['provider_class_name'];
      $provider = new $provider_class_name();
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

        $response['doi'] = $doi;
        $response['provider_class_name'] = $payload['provider_class_name'];
        $response['publication_data'] = PublicationDOI::load($doi, NULL, TRUE);
      }
      else {
        $response['errors'][] = t('Proxy error.');
      }
    }
    return $response;
  }

}
