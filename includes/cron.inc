<?php

/**
 * @file
 * Defines the callback functions for TPPS Ultimate Cron jobs.
 */

/**
 * Refreshes TPPS and CartograTree views.
 *
 * Make a call to CartograTree trees/reload API endpoint.
 */
function tpps_refresh_views() {
  global $base_url;
  if (module_exists('cartogratree')) {
    $response = file_get_contents("https://$base_url/cartogratree/api/v2/trees/reload");
    dpm($response);
  }
}

/**
 * Submits delayed TPPS submissions if they are past their release date.
 */
function tpps_delayed_submissions() {
  $submissions = variable_get('tpps_delayed_submissions', array());
  foreach ($submissions as $accession) {
    $state = tpps_load_submission($accession);
    $date = $state['saved_values']['summarypage']['release-date'];
    $time = strtotime("{$date['year']}-{$date['month']}-{$date['day']}");
    if (time() > $time) {
      tpps_submit_all($accession);
      unset($submissions[$accession]);
    }
  }
  variable_set('tpps_delayed_submissions', $submissions);
}
