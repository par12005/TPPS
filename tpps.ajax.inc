<?php

/**
 * @file
 * AJAX-callbacks and helper-functions.
 */

/**
 * Main AJAX callback which calls other functions.
 *
 * Use POST to send data to callback.
 * Use GET to just get response.
 * Note: To send data using GET you need to implement custom processing of
 * data from URL in callback and those data will appear in server logs.
 *
 * Callback must return:
 *  in case of errors: ['errors' => ['list', 'of', 'errors']],
 *  in case of success: ['success' => $data],
 *
 * @param string $callback
 *   Part of function name (without 'tpps_ajax_' prefix).
 *
 * @return string
 *   Returns callback-function result in JSON format.
 */
function tpps_ajax_callback($callback = '') {
  $function = 'tpps_ajax_' . check_plain($callback);
  if (!function_exists($function)) {
    $response['errors'][] = t('Callback not exists: @function', ['@function' => $function]);
  }
  else {
    $response = call_user_func($function, $_POST);
    if (empty($response)) {
      $response['errors'][] = t('Empty response from AJAX callback.');
    }
  }
  // If we never succed we fail.
  $response['success'] = $response['success'] ?? FALSE;
  return drupal_json_output($response);
}

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
    return [
      'errors' => [t('Empty POST requrest.')],
      'success' => FALSE,
    ];
  }
  module_load_include('inc', 'tpps', 'includes/manage_doi');
  // Validate.
  // Note: Could be used for 'Publication DOI' and 'Dataset DOI' so key is 'doi'.
  $doi = $data['doi'] ?? NULL;
  if (!preg_match(tpps_doi_regex(), $doi)) {
    $response['errors'][] = t('Invalid DOI format. Example DOI: 10.1111/dryad.111');
  }
  else {
    $response['statuses'][] = t('DOI is valid.');
    // Check if DOI in use.
    $doi_check_source = variable_get(
      'tpps_page_1_doi_check_source',
      'public.tpps_submissions'
    );
    if ($doi_check_source == 'public.tpps_submissions') {
      if ($accession = tpps_doi_get_accession($doi)) {
        $response['warnings'][]
          = t('WARNING: DOI is already used by @accession.',
            ['@accession' => $accession]);
      }
      else {
        $response['statuses'][] = t('DOI is not in use.');
      }
    }
    else {
      if (tpps_doi_count_dbxref($doi) == 0) {
        $response['statuses'][] = t('DOI is not in use.');
      }
      else {
        $response['warnings'][]
          = t('This DOI is already in use for another submission.');
      }
    }
    // DOI is valid so we could try to get data.
    $response['doi_info'] = tpps_get_doi_info($doi);
    if (!empty($response['doi_info'])) {
      $response['success'] = TRUE;
    }
    else {
      $response['success'] = FALSE;
      $response['warnings'][]
        = t('DOI Information is empty.');
    }
  }
  return $response;
}
