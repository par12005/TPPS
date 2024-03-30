<?php

/**
 * @file
 * Functions for manipulation of TPPS Submissions.
 *
 * ----------------------------------------------------------------------------
 * Concept.
 *
 * Submission interface initially is a copy of the Submission Form State
 * modified for Submission Pipeline to always have the same structure and
 * data format (type). For this each Submission Form State has version
 * (default version is '1') and function $submission->generateSharedState()
 * will convert existing $form_state to the $submission_interface which
 * will be stored in public.tpps_submissions database table.
 *
 * ----------------------------------------------------------------------------
 * Storage.
 *
 * 1. Serialized SIA is stored in DB (tpps_submission.submission_interface).
 * 2. PHP code always uses unserialized version if SIA.
 *
 * ----------------------------------------------------------------------------
 * API:
 * Note: functions has prefix 'tpps_submission_interface' (represented by '*').
 *   *_load() - Loads Submission Interface Array from DB. Uses cache.
 *   *_save() - Saves given Submission Interface Array to DB.
 *   *_generate() - Creates SIA and returns it (DB won't be updated).
 *   *_new() - Creates SIA, saves to DB and returns it.
 *   *_remove($accession) - Removes SIA field in DB.
 *
 * ----------------------------------------------------------------------------
 * Usage.
 *   module_load_include('inc', 'tpps', 'includes/submission_interface');
 *
 * Since submission data could be updated from admin's pages and they not use
 * a single function but update DB table directly it's safer for now to update
 * Submission Interface Array on each change of Submission State Array.
 *
 * Study statuses previously stored serialized in column tpps_submission.submission_state.
 * So we will store status for now inside Submission Interface Array as well.
 *
 * ----------------------------------------------------------------------------
 * Dates.
 *
 * There are several dates which also was stored in $submission_interface.
 *  updated
 *  created
 *  approved
 *  completed
 *  loaded
 *
 *  @todo Better to store them not inside serialized array.
 *  Create separate table for study statuses:
 *  Accession | Status Id | Timestamp
 *
 * ----------------------------------------------------------------------------
 * Usage tips.
 *
 * To exclude only 'submission_interface' from the list of query's fields:
 *   $query = db_select('tpps_submission', 's')
 *     // Exclude 'submission_interface' column.
 *     ->fields('s', array_diff(
 *       array_keys(drupal_get_schema('tpps_submission')['fields']),
 *       ['submission_interface']
 *     ));
 *
 * Exclude 'submission_interface' from chado_query().
 *   // Exclude 'submission_interface' column.
 *   $fields = implode(', ', array_diff(
 *     array_keys(drupal_get_schema('tpps_submission')['fields']),
 *     ['submission_interface']
 *   ));
 *   $all_submissions = chado_query("SELECT $fields FROM public.tpps_submission;");
 *
 * ----------------------------------------------------------------------------
 * @todo Use 'study_state' or even 'study' instead of 'Submission Interface'
 * because 'Submission Interface' is a name of the feature but not name of
 * the array with data.
 *
 * @todo See submit_all.php:75. There 'status' is stored twice:
 * 1. $submission_interface['status'].
 * 2. public.tpps_submission.status.
 * Check if $submission_interface['status'] is used anywhere and replace with
 * public.tpps_submission.status. Then update this code to use
 * Submission Interface only. Do not use tpps_update_submission() but
 * update db table directly to avoid $submission_interface update.
 *
 * @TODO Remove column 'Submission Interface' but it stores results of
 * study processing which must be then stored in State and all the pages
 * must be updated to use State instead of Interface.
 * Generation of Submision Interface takes about 25-30 msec so there is no
 * sense to store it in database because it must be updated each time when
 * Submission State changes.
 */
