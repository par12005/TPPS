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
  $port = 8100;
  $proxy = 'tgwebdev.cam.uchc.edu';
  $url = 'https://' . $proxy . ':' . $port . '/' . $gs->buildUrl();

  $output = '
  <div class="field_myiframe">
  <h3>DOI: ' . $doi . '</h3>
  <iframe src="' . $url . '" width="500px" height="300px"
    name="google_scholar" target="google_scholar"
    class="google-scholar">' . $url . '</iframe>
</div>';

  return $output;
}
