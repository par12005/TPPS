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
