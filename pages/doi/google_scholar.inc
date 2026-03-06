<?php

/**
 * @file
 * TPPS Google Scholar in iframe page.
 */

/**
 * TPPS DOI Lookup Form.
 */
function tpps_doi_google_scholar_page($doi1 = '', $doi2 = '') {
  if (!empty($doi2)) {
    $doi = $doi1 . '/' . $doi2;
  }
  else {
    $doi = '10.1002/ece3.4216';
  }

  $gs = new GoogleScholar($doi);
  $url = $gs->buildUrl();
  $proxy = variable_get('tpps_doi_lookup_proxy');
  if (empty($proxy)) {
    drupal_set_message(t('Set CORS Proxy Server URL'), 'error');
    drupal_goto('admin/config/tpps/doi');
  }
  $url = $proxy . $gs->buildUrl();

  $output = '<div>
  <h3>DOI: ' . $doi . '</h3>
  <iframe src="' . $url . '" width="500px" height="300px"
    name="google_scholar" target="google_scholar"
    class="google-scholar">' . $url . '</iframe>
  </div>';

  return $output;
}
