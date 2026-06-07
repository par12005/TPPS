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

}
