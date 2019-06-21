<?php

/**
 * @file
 * Defines the callback functions for TPPS Ultimate Cron jobs.
 */

/**
 * Refreshes TPPS and CartograTree views.
 *
 * CartograTree and the TPPS landing page both depend heavily on large
 * materialized views that need to be refreshed periodically. This is a
 * resource-heavy process, so it is best if it is done in a cron job.
 */
function tpps_refresh_views() {
  if (module_exists('cartogratree')) {
    $indexes = array(
      'plusgeno_view' => 'chado',
      'coordinates_view' => 'chado',
      'new_pheno_view' => 'chado',
      'new_geno_view' => 'chado',
      'ct_view' => 'chado',
      'ct_geno_view' => 'public',
      'ct_pub_view' => 'public',
      'ct_view_distinct' => 'public',
      'ct_view_complete' => 'public',
    );
  }

  foreach ($indexes as $table => $schema) {
    db_query("refresh materialized view $schema.$table");
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
