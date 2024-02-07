<?php

/**
 * @file
 * Manages Submission Form Version.
 *
 * ----------------------------------------------------------------------------
 * Overview.
 *
 * We need to have version of the Submission's Form to be able to convert
 * Form State Array into Submission Interface used by pipeline processing
 * (see form/submit/submit_all.php script).
 *
 * Note: There is no sense to store version in $form as hidden value.
 *
 * 1. Versions are integers and start from '1' which is
 *    defined as constant TPPS_SUBMISSION_INTERFACE_START_VERSION.
 * 2. Each version changes must be listed in function
 *    tpps_submission_interface_get_change_list().
 * 3. Version's value is stored under $form_state['saved_values'] because
 *    'saved_values' automatically stored/restored by TPPS module for each step.
 *
 * ----------------------------------------------------------------------------
 * Usage.
 *   module_load_include('inc', 'tpps', 'includes/submission_form_version');
 *
 * ----------------------------------------------------------------------------
 * API:
 * 1. To get version:
 *    tpps_submission_interface_version_get($form_state);
 * 2. To set version:
 *    tpps_submission_interface_version_set($form_state);
 *
 *
 * @todo If number of versions will be big enough and there is no old versions
 * left we could remove those versions from processing to speed-up interface
 * creation.
 */

// Number of the initial version of Form State Array.
// This value will be used to apply incremental changes (patches).
define('TPPS_SUBMISSION_FORM_START_VERSION', 1);

// Key in $form_state which used to store Submission Interface version.
define('TPPS_SUBMISSION_FORM_VERSION_KEY', 'submission_form_version');

// Version of Submission Form State Array.
// See tpps_submission_form_get_change_list() for changes per version.
// Version 1: Only removed data which is useless for processing study.
// Version 2: #86ayw8a7k Page 4 Genotype Logic Adjustments.
define('TPPS_SUBMISSION_FORM_CURRENT_VERSION', 2);

/**
 * Gets Submission Form Version.
 *
 * @param array $form_state
 *   Submission's Form State array.
 *
 * @return int
 *   Returns Submission's Form version.
 *   Returns 1 (default value) if version wasn't set.
 */
function tpps_submission_form_version_get(array $form_state) {
  return $form_state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] ?? 1;
}

/**
 * Sets Submission Form Version.
 *
 * @param array $form_state
 *   Submission's Form State array.
 */
function tpps_submission_form_version_set(array &$form_state) {
  // We use $form_state['saved_values'] as parent because it will be restored
  // and saved on each load/submit of multistep form.
  $form_state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY]
    = TPPS_SUBMISSION_FORM_CURRENT_VERSION;
}

/**
 * Gets specific version or all version's changes.
 *
 * @param int $version
 *   Version of the Submission Form.
 *   This version will be used to create Submission Interface.
 *
 * @return mixed
 *   Returns list of changes for particular version or NULL if there is no
 *   changes for given version number.
 *   Returns all version's changes.
 */
function tpps_submission_form_get_change_list($version = NULL) {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Remove useless elemLnts to reduce size.
  $change_list = [
    '1' => [
      // We unset not used items instead of copy required items from
      // $form_state because if new elements will be found they will be
      // left for sure. It's safer.
      'unset' => [
        // List of path to the items in $interface at the top (1st) level.
        'build_info',
        'temporary',
        'programmed',
        'programmed_bypass_access_check',
        'cache',
        'rebuild',
        'rebuild_info',
        'redirect',
        'submitted',
        'executed',
        'method',
        'groups',
        'buttons',
        'input',
        'values',
        'complete form',
        'process_input',
        'invalid_token',
        'triggering_element',
        'clicked_button',
        ['saved_values', 'form_build_id'],
        'tpps_submission_export_files',
        // @TODO Submission Form Version is useless for pipeline processing
        // but could be useful for debugging.
        ['saved_values', TPPS_SUBMISSION_FORM_VERSION_KEY],

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // List of the fields which must be left.
        // Required by submit_all() and tpps_table_display() and sub-functions.
        // 'saved_values',
        // 'dbxref_id',
        // 'created',
        // 'accession',
        // 'status',
        // 'stats',
        // 'ids',
        // 'tpps_type',
        // 'file_info',
        // 'updated',
        // 'approved',
        // 'completed',
        // 'loaded',
        // 'submitting_uid',
        // 'job_id',
        // 'revised_files',
        // 'admin_comments',
        // 'alternative_accessions',
        // 'data',
        // 'tree_info'.
        //
        //  Probably could be unset:
        // 'stage',
      ],
    ],
    // #86ayw8a7k Genotype Logic changes.
    '2' => [],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Version 1. Dynamic changes.
  //
  // Remove Drupal's internal fields used for form building.
  // Note: Those fields are stored in column tpps_submission.submission_state.
  $page_list = [
    'frontpage', TPPS_PAGE_1, TPPS_PAGE_2, TPPS_PAGE_3, TPPS_PAGE_4, 'summarypage',
  ];
  $drupal_internal_fields = [
    // Note: Page 1 has not 'Back' button.
    'Back',
    'Save',
    'Next',
    'form_build_id',
    'form_token',
    'form_id',
    'op',
    '#cleared',
    // Only for Page 4.
    'submit',
    'diagnostics-curation',
  ];
  foreach ($page_list as $page) {
    foreach ($drupal_internal_fields as $field) {
      $change_list['1']['unset'][] = ['saved_values', $page, $field];
    }
  }

  return ($version ? ($change_list[$version] ?? NULL) : $change_list);
}
