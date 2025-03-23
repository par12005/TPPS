<?php

/**
 * @file
 * This file contains miscellaneous API functions and Drupal hooks.
 */

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Zenodo Deposition's Events: created, updated, deleted, retrieved.

/**
 * Implements hook 'zenodo_deposit_deleted'.
 *
 * Note: Called only in successful removement of the Zenodo Deposition.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_deposit_deleted(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_deposit_created'.
 *
 * Note: Called only in successful Zenodo Deposition creation.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_deposit_created(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_deposit_updated'.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_deposit_updated(array $data, array $response) {
  dpm($data);
}

/**
 * Implements hook 'zenodo_deposit_retrieve'.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_deposit_retrieved(array $data, array $response) {
  dpm($data);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Zenodo Deposition File's Events: created, updated, deleted, retrieved.

/**
 * Implements hook 'zenodo_files_deleted'.
 *
 * Note: Called only in successful removement of the Zenodo Deposition.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id', 'file_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_files_deleted(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_files_created'.
 *
 * Note: Called only in successful Zenodo Deposition creation.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id', 'file_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_files_created(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_files_updated'.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id', 'file_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_files_updated(array $data, array $response) {
  tpps_admin_zenodo_submissions_report_clear_cache();
}

/**
 * Implements hook 'zenodo_files_retrieve'.
 *
 * @param array $data
 *   Request Data. Required keys are: 'deposit_id', 'file_id'.
 * @param array $response
 *   API Response.
 */
function hook_zenodo_files_retrieved(array $data, array $response) {
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
 * Perform alterations before requiest in ZenodoDeposit::list().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $query
 *   Data for request. Required keys are: 'query'.
 *   See https://developers.zenodo.org/?python#list.
 */
function hook_zenodo_deposit_list_alter(array &$query) {
  dpm($query);
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
  dpm($data['deposit_id']);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Zenodo Deposition File's Actions.

/**
 * Perform alterations before requiest in ZenodoFiles::delete().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request.
 *   Required item is 'deposit_id'.
 *   See https://developers.zenodo.org/?python#delete.
 */
function hook_zenodo_files_delete_alter(array &$data) {
  dpm($data['deposit_id']);
}

/**
 * Perform alterations before requiest in ZenodoFiles::create().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request. Could be empty.
 */
function hook_zenodo_files_create_alter(array &$data) {
  dpm($data);
}

/**
 * Perform alterations before requiest in ZenodoFiles::update().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request.
 *   Required item is 'deposit_id', 'metadata'.
 */
function hook_zenodo_files_update_alter(array &$data) {
  dpm($data['deposit_id']);
}

/**
 * Perform alterations before requiest in ZenodoFiles::list().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $query
 *   Data for request. Required keys are: 'query'.
 *   See https://developers.zenodo.org/?python#list.
 */
function hook_zenodo_files_list_alter(array &$query) {
  dpm($query);
}

/**
 * Perform alterations before requiest in ZenodoFiles::retrieve().
 *
 * Note: Called before API Endpoint URL will be generated.
 *
 * @param array $data
 *   Data for Zenodo API request.
 *   Required item is 'deposit_id'.
 *   See https://developers.zenodo.org/?python#retrieve.
 */
function hook_zenodo_files_retrieve_alter(array &$data) {
  dpm($data['deposit_id']);
}
