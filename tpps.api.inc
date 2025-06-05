<?php

/**
 * @file
 * This file contains miscellaneous API functions and Drupal hooks.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Hooks for Zenodo API events:
 * - 'ZenodoDeposit' class:
 *   - created, updated, deleted, retrieved.
 *   - edited, published, new_version_created, discarded.
 * - 'ZenodoFile' class:
 *   - created, updated, deleted, retrieved.
 * Alter-hooks:
 * - 'ZenodoDeposit' class: create, update, delete, retrieve.
 * - 'ZenodoFile' class:   create, update, delete, retrieve.
 */

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Zenodo Deposition's Events: created, updated, deleted, retrieved.

/**
 * Implements hook 'zenodo_deposit_deleted'.
 *
 * Called when deleted a Zenodo Deposition.
 *
 * @param array $data
 *   Request Data. Keys are:
 *   - 'deposit_id' int Required. Zenodo Deposit Id.
 *   - 'accession' string Required. TPPS Submission accession.
 *   - 'url' string Required. Zenodo API Endpoint URL.
 *   - 'response' mixed Required. Zenodo API Response. Empty on success.
 */
function hook_zenodo_deposit_deleted(array $data) {
  dpm($data);
}

/**
 * Implements hook 'zenodo_deposit_created'.
 *
 * Called when created a new Zenodo Deposition.
 *
 * @param array $data
 *   Request Data. Keys are:
 *   - 'deposit_id' int Required. Zenodo Deposit Id.
 *   - 'accession' string Required. TPPS Submission accession.
 *   - 'url' string Required. Zenodo API Endpoint URL.
 *   - 'payload' array Required. Request payload.
 *   - 'response' mixed Required. Zenodo API Response.
 */
function hook_zenodo_deposit_created(array $data) {
  dpm($data);
}

/**
 * Implements hook 'zenodo_deposit_updated'.
 *
 * Called when updated a single deposition resource.
 *
 * @param array $data
 *   Request Data. Keys are:
 *   - 'deposit_id' int Required. Zenodo Deposit Id.
 *   - 'accession' string Required. TPPS Submission accession.
 *   - 'url' string Required. Zenodo API Endpoint URL.
 *   - 'payload' array Required. Request payload.
 *   - 'response' mixed Required. Zenodo API Response.
 */
function hook_zenodo_deposit_updated(array $data) {
  dpm($data);
}

/**
 * Implements hook 'zenodo_deposit_retrieved'.
 *
 * Called when retrieved a single deposition resource.
 *
 * @param array $data
 *   Request Data. Keys are:
 *   - 'deposit_id' int Required. Zenodo Deposit Id.
 *   - 'accession' string Required. TPPS Submission accession.
 *   - 'url' string Required. Zenodo API Endpoint URL.
 *   - 'response' mixed Required. Zenodo API Response.
 */
function hook_zenodo_deposit_retrieved(array $data) {
  dpm($data);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Zenodo Deposition Actions: edit, publish, newversion, ....



// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Zenodo Deposition File's Events: created, updated, deleted, retrieved.

/**
 * Implements hook 'zenodo_file_deleted'.
 *
 * Note: Called only in successful removement of the Zenodo Deposition.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id', 'file_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_file_deleted(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_file_created'.
 *
 * Note: Called only in successful Zenodo Deposition creation.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id', 'file_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_file_created(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_file_updated'.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id', 'file_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_file_updated(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_file_retrieve'.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id', 'file_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_file_retrieved(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Zenodo Deposition API Actions.

/**
 * Perform alterations before requiest in ZenodoDeposit::delete().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request.
 *   Required item is 'deposit_id'.
 *   See https://developers.zenodo.org/?python#delete.
 */
function hook_zenodo_deposit_delete_alter(array &$data) {
  dpm($data['deposit_id']);
}

/**
 * Perform alterations before requiest in ZenodoDeposit::create().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request. Could be empty.
 */
function hook_zenodo_deposit_create_alter(array &$data) {
  dpm($data);
}

/**
 * Perform alterations before requiest in ZenodoDeposit::update().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request.
 *   Required item is 'deposit_id', 'metadata'.
 */
function hook_zenodo_deposit_update_alter(array &$data) {
  dpm($data['deposit_id']);
}

/**
 * Perform alterations before requiest in ZenodoDeposit::list([]).
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for request. Required keys are: 'query'.
 *   See https://developers.zenodo.org/?python#list.
 */
function hook_zenodo_deposit_list_alter(array &$data) {
  dpm($data);
}

/**
 * Perform alterations before requiest in ZenodoDeposit::retrieve().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request.
 *   Required item is 'deposit_id'.
 *   See https://developers.zenodo.org/?python#retrieve.
 */
function hook_zenodo_deposit_retrieve_alter(array &$data) {
  dpm($data);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Deposit Actions: edit, publish, discard, newversion.

/**
 * Alters data before requiest in ZenodoDeposit::publish().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Request Data. Keys are:
 *   - 'deposit_id' int Required. Zenodo Deposit Id.
 *   - 'accession' string Required. TPPS Submission accession.
 *   - 'action' string Required. Current action.
 *
 *   See https://developers.zenodo.org/?python#retrieve.
 */
function hook_zenodo_deposit_publish_alter(array &$data) {
  dpm($data);
}

/**
 * Alters data before requiest in ZenodoDeposit::edit().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Request Data. Keys are:
 *   - 'deposit_id' int Required. Zenodo Deposit Id.
 *   - 'accession' string Required. TPPS Submission accession.
 *   - 'action' string Required. Current action.
 *
 *   See https://developers.zenodo.org/?python#retrieve.
 */
function hook_zenodo_deposit_edit_alter(array &$data) {
  dpm($data);
}

/**
 * Alters data before requiest in ZenodoDeposit::discard().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Request Data. Keys are:
 *   - 'deposit_id' int Required. Zenodo Deposit Id.
 *   - 'accession' string Required. TPPS Submission accession.
 *   - 'action' string Required. Current action.
 *
 *   See https://developers.zenodo.org/?python#retrieve.
 */
function hook_zenodo_deposit_discard_alter(array &$data) {
  dpm($data);
}

/**
 * Alters data before requiest in ZenodoDeposit::newVersion().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Request Data. Keys are:
 *   - 'deposit_id' int Required. Zenodo Deposit Id.
 *   - 'accession' string Required. TPPS Submission accession.
 *   - 'action' string Required. Current action.
 *
 *   See https://developers.zenodo.org/?python#retrieve.
 */
function hook_zenodo_deposit_new_version_alter(array &$data) {
  dpm($data);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Zenodo Deposition File's Actions.

/**
 * Perform alterations before requiest in ZenodoFile::delete().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request.
 *   Required item is 'deposit_id'.
 *   See https://developers.zenodo.org/?python#delete.
 */
function hook_zenodo_file_delete_alter(array &$data) {
  dpm($data['deposit_id']);
}

/**
 * Perform alterations before requiest in ZenodoFile::create().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request. Could be empty.
 */
function hook_zenodo_file_create_alter(array &$data) {
  dpm($data);
}

/**
 * Perform alterations before requiest in ZenodoFile::update().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request.
 *   Required item is 'deposit_id', 'metadata'.
 */
function hook_zenodo_file_update_alter(array &$data) {
  dpm($data['deposit_id']);
}

/**
 * Perform alterations before requiest in ZenodoFile::list().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $query
 *   Data for request. Required keys are: 'query'.
 *   See https://developers.zenodo.org/?python#list.
 */
function hook_zenodo_file_list_alter(array &$query) {
  dpm($query);
}

/**
 * Perform alterations before requiest in ZenodoFile::retrieve().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request.
 *   Required item is 'deposit_id'.
 *   See https://developers.zenodo.org/?python#retrieve.
 */
function hook_zenodo_file_retrieve_alter(array &$data) {
  dpm($data);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Zenodo Deposition Actions: edited, published, new_version_created, discarded.

/**
 * Implements hook 'zenodo_deposit_edited'.
 *
 * See: https://developers.zenodo.org/?shell#edit.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_deposit_edited(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_deposit_discarded'.
 *
 * See: https://developers.zenodo.org/?shell#discard.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_deposit_discarded(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_host_toggled'.
 *
 * Called from ZenodoHost::toggle().
 *
 * Note: This hook could be called more then once during host search but
 * hook 'zenodo_host_changed' will be called once.
 *
 * @param string $new_host
 *   New Zenodo Host which was switched to.
 *   Possible values: ZenodoHost::LIVE_HOST and ZenodoHost::SANDBOX_HOST.
 */
function hook_zenodo_host_toggled($new_host) {
  drupal_set_message($new_host);
}

/**
 * Implements hook 'zenodo_host_set'.
 *
 * Called from ZenodoHost::set().
 *
 * @param string $new_host
 *   New Zenodo Host which was switched to.
 *   Possible values: 'live' and 'sandbox'.
 */
function hook_zenodo_host_set($new_host) {
  drupal_set_message($new_host);
}

/**
 * Implements hook 'zenodo_host_changed'.
 *
 * Called from ZenodoHost::search().
 *
 * Note: Will be called once during host search if deposition was found on
 * non-default host.
 *
 * @param string $new_host
 *   New Zenodo Host which was switched to.
 *   Possible values: ZenodoHost::LIVE_HOST and ZenodoHost::SANDBOX_HOST.
 */
function hook_zenodo_host_changed($new_host) {
  drupal_set_message($new_host);
}

/**
 * Implements hook 'zenodo_deposit_validation_failed'.
 *
 * @param array $response
 *   Response of the Zenodo API of 'retrieve' requiest.
 */
function hook_zenodo_deposit_validation_failed(array $response) {
  drupal_set_message($reponse);
}

/**
 * Implements hook 'zenodo_deposit_new_version_created'.
 *
 * See: https://developers.zenodo.org/?shell#new-version.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_deposit_new_version_created(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_deposit_published'.
 *
 * See: https://developers.zenodo.org/?shell#publish.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_deposit_published(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'rest_api_request_execution_time'.
 *
 * @param float $execution_time
 *   Request execution time in msec.
 * @param string $data
 *   Request meta-data to be shown in log messages.
 */
function hook_rest_api_request_execution_time($execution_time, array $data = []) {
}

/**
 * Implements hook 'zenodo_bucket_create_alter'.
 *
 * @param array $data
 *   Context for ZenodoBucket::create().
 */
function hook_zenodo_bucket_create_alter(array &$data) {
}

/**
 * Implements hook 'zenodo_deposition_before_create'.
 *
 * @param array $data
 *   Context for ZenodoBucket::create().
 */
function hook_zenodo_deposit_before_create(array $data) {
}

/**
 * Implements alter-hook 'rest_api_exception'.
 *
 * Called from RestAPI::doRequest() when API responded with HTTP code different
 * from 200..204.
 *
 * @param string $message
 *   Exception message which could be altered.
 * @param array $exception_context
 *   Request context data. Keys are:
 *    'payload' - mixed.
 *    'url' - string. API Endpoint URL.
 *    'method' - string. HTTP-method in uppercase.
 *    'code' - int. HTTP response code.
 *    'header' - array. List of request headers. Each element is a string.
 *    'response' - mixed. API server response.
 *    'curl_info' - array. Data provided by curl_info().
 *    'backtrace' - array. Stack of the callers.
 *    'message' - string. Clone of the $message to always have original string.
 */
function hook_rest_api_exception($message, array $exception_context) {
}

/**
 * Implements 'submission_dataset_doi_updated'.
 *
 * Called from Submission::updateDatasetDOI().
 *
 * @param array $data
 *   Context data related to the event. Keys are:
 *     'dataset_doi' - string.
 *       New Dataset DOI.
 *     'shared_state_dataset_doi' - string.
 *       DOI found in Submission's Shared State.
 *     'state_dataset_doi' - string.
 *       DOI found in Submission's State array.
 *     'update_result' - array. Result of updating.
 *       Keys are: 'State' and 'Shared State'. Value could be:
 *       TRUE - was updated and FALSE when wasn't updated.
 */
function hook_submission_dataset_doi_updated(array $data) {
  // New.
  $dataset_doi = $data['dataset_doi'];
  // Stored values of Dataset DOI.
  $shared_state_dataset_doi = $data['shared_state_dataset_doi'];
  $state_dataset_doi = $data['state_dataset_doi'];
  $update_result = $data['update_result'];
}

/**
 * Implements 'submission_form_version_changed'.
 *
 * @param array $context
 *   Keys are: 'previous_version' and 'new_version'.
 */
function hook_submission_form_version_changed(array $context) {
  $old = $context['previous_version'];
  $new = $context['new_version'];
}

/**
 * Impleemnts 'submission_deleted'.
 *
 * @param string $accession
 *   TPPS Submission accession. E.g., 'TGDRxxxx'.
 */
function hook_submission_deleted($accession) {
  $message = t('Removed study @accession.', ['@accession' => $accession]);
  drupal_set_message($message, 'warning');
}
