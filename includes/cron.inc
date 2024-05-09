<?php

/**
 * @file
 * Defines the callback functions for TPPS Ultimate Cron jobs.
 */

/**
 * Recreates table "chado.tpps_ref_genomes".
 */
function tpps_cron_table_tpps_ref_genomes_recreate() {
  chado_query("DROP TABLE IF EXISTS chado.tpps_ref_genomes;", []);
  chado_query("CREATE TABLE chado.tpps_ref_genomes AS (
    select distinct a.name, a.analysis_id, a.programversion,
       o.genus||' '||o.species as species from chado.analysis a
    join chado.analysisfeature af on a.analysis_id = af.analysis_id
    join chado.feature f on af.feature_id = f.feature_id
    join chado.organism o on f.organism_id = o.organism_id
    where f.type_id in (379,595,597,825,1245,422) AND a.name LIKE '% v%'
  );", []);
}

/**
 * Refreshes TPPS and CartograPlant views.
 *
 * Make a call to CartograPlant trees/reload API endpoint.
 */
function tpps_cron_refresh_views() {
  global $base_url;
  if (module_exists('cartogratree') and ($ct_api_key = variable_get('tpps_ct_api_key', NULL))) {
    file_get_contents("$base_url/cartogratree/api/v2/trees/reload?api_key=$ct_api_key");
  }
  db_query('refresh materialized view chado.tpps_search_genotype_name;');
  db_query('refresh materialized view chado.tpps_search_genotype_marker;');
}

/**
 * Check for latest Tripal Job status to Slack.
 *
 * Make a call to CartograPlant trees/reload API endpoint.
 */
function tpps_cron_latest_job_status_slack_updates() {
  global $base_url;

  $slack_url = variable_get('tpps_latest_job_status_slack_updates_api_url', NULL);
  if ($slack_url == NULL || $slack_url == "") {
    return;
  }

  $last_job_id = variable_get('tpps_latest_job_status_slack_updates_last_job_id', NULL);
  if ($last_job_id == NULL || $last_job_id == "") {
    return;
  }

  $results = db_query('SELECT * FROM public.tripal_jobs ORDER BY job_id DESC LIMIT 20');
  $old_status = variable_get('tpps_latest_job_status_slack_updates_previous_job_status', NULL);
  if ($old_status == NULL) {
    $old_status = [];
  }
  $message = "";
  $current_job_ids = [];
  foreach ($results as $row) {
    if ($row->modulename == 'tpps') {
      $job_id = $row->job_id;
      $current_job_ids[] = $job_id; // keep track of current job_ids
      $user_tmp = user_load($row->uid);
      // check if old_status data exists
      if (isset($old_status[$job_id])) {
        $status = $old_status[$job_id];
        // if the new status is not the same as the old
        if ($row->status != $status) {
          if ($status == 'Error') {
            $message .= "An error occurred, please check on ";
          }
          $message .= "TPPS JOB $job_id: " . $row->job_name . " from "
            . $user_tmp->name . " - status changed to "
            . strtoupper($row->status) . "\n";
          $old_status[$job_id] = $row->status;
        }
        else {
          // if ($status == "Completed") {
          //   // this job is already completed and is known to be completed, don't add to messages
          // }
          // else {
          //   // $message .= "JOB $job_id [" . $row->job_name . "] did not change.\n";
          // }
        }
      }
      // If old_status data does not exist, this is a new job found
      else {
        if ($row->status == 'Waiting') {
          $message .= "Found new TPPS job $job_id [" . $row->job_name
            . "] from " . $user_tmp->name . " to monitor with status "
            . strtoupper($row->status) . ". Monitoring for status change.\n";
        }
        $old_status[$job_id] = $row->status;
      }
    }
    else {
      // ignore since we only wanted TPPS jobs
    }
  }

  // Remove ids that are not in the current job ids from $old_status.
  $diff = array_diff(array_keys($old_status), $current_job_ids);
  // watchdog('tpps', print_r($diff, true));
  $old_ids = count($diff);
  if ($old_ids == 0) {
    // $message .= "I didn't find any new TPPS jobs to monitor. I'll check back periodically.\n";
  }
  foreach ($diff as $vals) {
    unset($old_status[$vals]);
  }


  variable_set('tpps_latest_job_status_slack_updates_previous_job_status', $old_status);
  // variable_set('tpps_latest_job_status_slack_updates_previous_job_status', NULL);

  watchdog('tpps', $message . print_r($old_status, true));

  if ($message != "") {
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $slack_url);
    curl_setopt($ch, CURLOPT_POST, 1);
    $payload = json_encode(array("text" => $message));
    curl_setopt($ch, CURLOPT_POSTFIELDS, $payload);
    curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type:application/json']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
    $server_output = curl_exec($ch);
    curl_close($ch);
  }
}

/**
 * Refreshes "chado.plusgeno_view".
 *
 * TPPS "plusgeno_view" refreshes the TPPS Details Directory listing.
 */
function tpps_cron_refresh_plusgeno_view() {
  global $base_url;
  db_query('refresh materialized view concurrently chado.plusgeno_view;');
}

/**
 * Submits delayed TPPS submissions if they are past their release date.
 *
 * @todo Use Drupal Queue instead.
 */
function tpps_cron_delayed_submissions() {
  $accession_list = variable_get('tpps_delayed_submissions', []);
  foreach ($accession_list as $accession) {
    $submission = new Submission($accession);
    $date = $submission->state['saved_values']['summarypage']['release-date'];
    $time = strtotime("{$date['year']}-{$date['month']}-{$date['day']}");
    if (time() > $time) {
      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $args = [$accession];
      $jid = tripal_add_job("TPPS Record Submission - $accession", 'tpps',
        'tpps_submit_all', $args, $submission->state['submitting_uid'], 10,
        $includes, TRUE
      );
      $submission->state['job_id'] = $jid;
      $submission->save();
      unset($accession_list[$accession]);
    }
  }
  variable_set('tpps_delayed_submissions', $accession_list);
}
