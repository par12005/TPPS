<?php

/**
 * @file
 * Defines the callback functions for TPPS Ultimate Cron jobs.
 */

/**
 * Refreshes TPPS and CartograPlant views.
 *
 * Make a call to CartograPlant trees/reload API endpoint.
 */
function tpps_refresh_views() {
  global $base_url;
  if (module_exists('cartogratree') and ($ct_api_key = variable_get('tpps_ct_api_key', NULL))) {
    $response = file_get_contents("$base_url/cartogratree/api/v2/trees/reload?api_key=$ct_api_key");
  }
  db_query('refresh materialized view chado.tpps_search_genotype_name;');
  db_query('refresh materialized view chado.tpps_search_genotype_marker;');
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
      $state = tpps_load_submission($accession);
      $includes = array();
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $args = array($accession);
      $jid = tripal_add_job("TPPS Record Submission - $accession", 'tpps', 'tpps_submit_all', $args, $state['submitting_uid'], 10, $includes, TRUE);
      $state['job_id'] = $jid;
      tpps_update_submission($state);
      unset($submissions[$accession]);
    }
  }
  variable_set('tpps_delayed_submissions', $submissions);
}
