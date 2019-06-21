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
      'plusgeno_view' => array('chado', 'project_id'),
      'coordinates_view' => array('chado', 'project_id'),
      'new_pheno_view' => array('chado', 'uniquename'),
      'new_geno_view' => array('chado', 'tree_acc'),
      'ct_view' => array('chado', 'uniquename'),
      'ct_geno_view' => array('public', 'tree_acc'),
      'ct_pub_view' => array('public', 'project_id'),
      'ct_view_distinct' => array('public', 'uniquename'),
      'ct_view_complete' => array('public', 'uniquename'),
    );
  }

  foreach ($indexes as $table => $info) {
    $schema = $info[0];
    $col_name = $info[1];
    $and = db_and()
      ->condition('schemaname', $schema)
      ->condition('tablename', $table)
      ->condition('indexname', 'tpps_' . $table . '_cron');
    
    $query = db_select('pg_indexes', 'i')
      ->fields('i')
      ->condition($and)
      ->execute();

    $result = $query->fetchObject();
    if (!$result) {
      db_query("create unique index tpps_{$table}_cron on {$schema}.{$table} ($col_name})");
    }
    db_query("refresh materialized view concurrently $schema.$table");
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
