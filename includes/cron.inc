<?php

/**
 * @file
 * Defines the callback functions for TPPS Ultimate Cron jobs.
 */

/**
 * Refreshes the chado.plusgeno_view materialized view.
 *
 * The chado.plusgeno_view is the materialized view that the tpps_details module
 * uses to retrieve information about TPPS submissions on the landing page. It
 * should be refreshed periodically as part of a cron job.
 */
function tpps_refresh_landing_page() {

  $and = db_and()
    ->condition('schemaname', 'chado')
    ->condition('tablename', 'plusgeno_view')
    ->condition('indexname', 'tpps_cron');

  $query = db_select('pg_indexes', 'i')
    ->fields('i')
    ->condition($and)
    ->execute();

  $result = $query->fetchAssoc();

  if (!$result) {
    db_query('create unique index tpps_cron on chado.plusgeno_view (project_id)');
  }

  db_query('refresh materialized view concurrently chado.plusgeno_view');
}

/**
 * Submits delayed TPPS submissions if they are past their release date.
 */
function tpps_delayed_submissions(){
  $submissions = variable_get('tpps_delayed_submissions', array());
  foreach ($submissions as $accession){
    $state = tpps_load_submission($accession);
    $date = $state['saved_values']['summarypage']['release-date'];
    $time = strtotime("{$date['year']}-{$date['month']}-{$date['day']}");
    if (time() > $time){
      tpps_submit_all($accession);
      unset($submissions[$accession]);
    }
  }
  variable_set('tpps_delayed_submissions');
}
