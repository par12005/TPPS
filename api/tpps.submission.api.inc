<?php

/**
 * @file
 * Defines TPPS API functions for submission-related operations.
 */

/**
 * Returns information about TPPS submissions satisfying provided conditions.
 *
 * This function first calls the tpps_api_submission_std_query function with
 * the provided conditions, then filters the list of submissions using the
 * various tpps_api_*_pid_query functions if there are remaining unused
 * conditions. The supported standard query conditions include:
 *   accession number
 *   tpps_submission table record id
 *   submission uid
 *   submisison status
 *   submission dbxref_id
 *   submission state
 * Supported additional query conditions include:
 *   author
 *   organism
 *   organization
 * The default database comparison operator is '~*', but different operators
 * can be specified using square brackets. For example, calling
 *   submission?organization=University
 * would produce a condition like
 *   WHERE name ~* 'University'
 * but calling
 *   submission?organization=University[LIKE]
 * would produce a condition like
 *   WHERE name LIKE 'University'
 *
 * @return array
 *   An array of information about submissions that satisfy the conditions.
 */
function tpps_api_submission_query() {
  $conditions = drupal_get_query_parameters();

  $submissions = tpps_api_submission_std_query($conditions);
  if (!empty($conditions)) {
    foreach ($conditions as $key => $val) {
      $pids = array();
      $vals = explode(',', $val);
      $function_name = "tpps_api_{$key}_pid_query";

      if (!function_exists($function_name)) {
        return array();
      }

      $query = $function_name($vals);
      while (($result = $query->fetchObject())) {
        $pids[] = $result->project_id;
      }

      if (empty($pids)) {
        return array();
      }

      foreach ($submissions as $num => $state) {
        if (array_search($state['ids']['project_id'], $pids) === FALSE) {
          unset($submissions[$num]);
        }
      }
    }
  }

  $results = array();
  foreach ($submissions as $submission) {
    $results[] = array(
      'accession' => $submission['accession'],
      'data' => $submission['saved_values'],
      'dbxref_id' => $submission['dbxref_id'],
      'file_info' => $submission['file_info'] ?? NULL,
      'file_rank' => $submission['file_rank'] ?? NULL,
      'ids' => $submission['ids'] ?? NULL,
      'job_id' => $submission['job_id'] ?? NULL,
      'stage' => $submission['stage'],
      'stats' => $submission['stats'] ?? NULL,
      'status' => $submission['status'],
      'submitting_uid' => $submission['submitting_uid'] ?? NULL,
      'tpps_type' => $submission['tpps_type'] ?? NULL,
    );
  }

  return $results;
}

/**
 * Performs the initial TPPS submission standard query.
 *
 * This function filters submissions by conditions that match columns in the
 * tpps_submission table. These columns include:
 *   tpps_submission_id
 *   accession
 *   uid
 *   status
 *   dbxref_id
 *   submission_state
 *   shared_state
 *
 * The function will also unset items in the conditions array that have already
 * been used, so that the tpps_api_submission_query function knows whether to
 * perform additional query filtering.
 * If no conditions in the conditions array match the columns of the
 * tpps_submission table, this function will simply return all TPPS submissions
 * found in the table.
 *
 * @param array $conditions
 *   The conditions to filter the submission query by.
 *
 * @return array
 *   An array of submissions that satisfy the provided conditions.
 */
function tpps_api_submission_std_query(array &$conditions) {
  $args = array();
  $valid_args = tpps_table_columns('tpps_submission');
  foreach ($conditions as $key => $val) {
    if (array_search($key, $valid_args)) {
      $args[$key] = explode(',', $val);
      unset($conditions[$key]);
    }
  }

  $states = tpps_load_submission_multiple($args);
  $results = array();
  foreach ($states as $state) {
    if (!isset($state['ids']['project_id'])) {
      continue;
    }
    $results[$state['ids']['project_id']] = $state;
  }
  return $results;
}

/**
 * Returns information about submissions based on provided accessions.
 *
 * This function will return data and possibly Tripal Entities for TPPS
 * submissions based on provided accessions and data requests. Possible data
 * that can be retrieved using this function includes:
 *   project
 *   publication
 *   author
 *   organism
 *   design
 *   stock
 *   phenotype
 *   genotype
 *   stats
 * By default, the function will simply return the database ids of the
 * associated data, but if the entities parameter is provided and set to true,
 * the function will instead return Tripal Entites of associated data if
 * possible. Data options that currently support Tripal Entities include:
 *   project
 *   publication
 *   organism
 *   stock
 *   genotype
 * For example, to retrieve the database ids of the authors and species
 * associated with a TPPS submission, you could call
 *   submission_info?accessions=TGDR001&data=author,organism
 * If you instead want to return the Tripal Entities for the species, call
 *   submission_info?accessions=TGDR001&data=author,organism&entities=TRUE
 * The returned information is sorted by data type, and if entities are both
 * requested and valid, then the information for that data type will be indexed
 * by Tripal Entity id.
 *
 * @return array
 *   An array of information about the specified TPPS Submissions.
 */
function tpps_api_submission_info() {
  $params = drupal_get_query_parameters();
  if (empty($params['accessions'])) {
    return NULL;
  }

  if (empty($params['data'])) {
    return NULL;
  }

  $entities = FALSE;
  if (array_key_exists('entities', $params)) {
    $entities = $params['entities'];
  }

  $accessions = explode(',', $params['accessions']);
  $states = tpps_load_submission_multiple(array('accession' => $accessions));
  $pids = array();
  foreach ($states as $state) {
    $pids[] = $state['ids']['project_id'];
  }
  $data_types = explode(',', $params['data']);
  $results = array();
  foreach ($data_types as $type) {
    $function_name = "tpps_api_project_{$type}_query";
    if ($type == 'project') {
      $function_name = "tpps_api_project_query";
    }

    if ($type == 'stats') {
      $results[$type] = $function_name($states);
      continue;
    }

    if (!function_exists($function_name)) {
      return NULL;
    }

    $results[$type] = $function_name($pids, $entities);
  }

  return $results;
}
