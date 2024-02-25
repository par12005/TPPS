<?php

/**
 * @file
 * Manages Submission Form Version.
 */

/**
 * Overview.
 *
 * We need to have version of the Submission's Form to be able to convert
 * Form State Array into Submission Interface used by pipeline processing
 * (see form/submit/submit_all.php script).
 *
 * ----------------------------------------------------------------------------
 * What is Submission Form Version?
 * 1. Versions are integers and start from '1' which is
 *    defined as constant TPPS_SUBMISSION_INTERFACE_START_VERSION.
 * 2. Each version changes must be listed in function
 *    tpps_submission_interface_get_change_list().
 * 3. Version's value is stored under $form_state['saved_values'] because
 *    'saved_values' automatically stored/restored by TPPS module for each step.
 *
 * ----------------------------------------------------------------------------
 * Downgrade or upgrade?
 *
 * - Submission Interface has NO version. It equals to Submission Form Version 1.
 * - Submission Form (not $form_state) has ALWAYS latest version
 *   (TPPS_SUBMISSION_FORM_CURRENT_VERSION).
 * - Submission Form State of imported (usually old, version 1 or lower then
 *   latest verstion) studies must be UPgranded.
 * - To generate Submission Interface from current Submission Form State
 *   (just submitted, updated after import or updated on edit old study)
 *   we need to DOWNgrade.
 *
 * ----------------------------------------------------------------------------
 * How to relocate field?
 *
 * 1. Change form (see /form/build/*)
 * 2. Update Drupal State conditions.
 * 3. Update JS to use new 'name' params.
 * 5. Search module's code for a field name and update if necessary.
 * 6. Add new items to the Version Change List below.
 *
 * There is no need to update function tpps_flatten() because it expects
 * Submission Interface Array.
 *
 * How to update Submission Interface?
 * 1. Update submit_all.php
 * 2. Update this file.
 * 3. tpps_table_display().
 * 4. tpps_flatten().
 * 5. Other code which uses Submission Interface.
 *
 * ----------------------------------------------------------------------------
 * API.
 *
 * To use API:
 *   module_load_include('inc', 'tpps', 'includes/submission_form_version');
 * To get version:
 *    tpps_submission_interface_version_get($form_state);
 * To set version:
 *    tpps_submission_interface_version_set($form_state);
 *
 * ----------------------------------------------------------------------------
 * Predefined actions:
 * 1. 'unset' - removes item from resulting (not original) array.
 * 2. 'move' - relocates item (and it's sub-elements) to new place in resulting
 *             (not original) array.
 * 3. 'rename' = move where target has new field name (last element).
 *
 * What is changelist?
 *
 * Change List for each version is a list of data to perform actions
 * (unset, move). For 'unset' it will be path to elements (list of parents
 * including field which must be removed).
 * For 'move' it will be 'source' (where to get) and 'target' (where to put).
 *
 * ----------------------------------------------------------------------------
 * @todo Minor. If number of versions will be big enough and there is
 *       no old versions left we could remove those versions from
 *       processing to speed-up interface creation.
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
// @TODO Minor. Rename to 'latest_version'?
define('TPPS_SUBMISSION_FORM_CURRENT_VERSION', 2);

/**
 * Checks if 'Submission Interface Debug Mode' is enabled.
 *
 * 'Submission Interface Debug Mode' allows to get debug messages in browser
 * and is always enabled on dev-server (not live site) and could be enabled
 * on live site at admin settings page (admin/config/tpps/form/main).
 *
 * @return bool
 *   Returns TRUE if 'Submission Interface Debug Mode' is enabled.
 *   Returns FALSe otherwise.
 */
function tpps_is_si_debug_mode() {
  module_load_include('inc', 'tpps', 'includes/common');
  return !is_live_site() || variable_get('tpps_si_debug_mode', FALSE);
}

/**
 * Checks if Debug Mode is enabled.
 *
 * @return bool
 *   Returns TRUE if 'Submission Interface Debug Mode' is enabled.
 *   Returns FALSe otherwise.
 */
function tpps_is_debug_mode() {
  return variable_get('tpps_debug_mode', FALSE);
}

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
 * @param array $form
 *   Submission's Form array.
 *
 *   @todo Replace with *_add_fiedl().
 *   @todo See tpps_main_submit() to create new function.
 */
function tpps_submission_form_version_set(array &$form, array $form_state) {
  $version = $form_state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY]
    ?? TPPS_SUBMISSION_FORM_START_VERSION;
  // We use $form_state['saved_values'] as parent because it will be restored
  // and saved on each load/submit of multistep form.
  $form[TPPS_SUBMISSION_FORM_VERSION_KEY] = [
    '#type' => 'hidden',
    '#value' => $version,
  ];
}

/**
 * Submission Interface Action 'unset'.
 *
 * Given list of fields will be removed/unset in Submission Interface.
 *
 * @param array $interface
 *   Submission Interface passed by reference so no need to return it.
 * @param array $action_data
 *   Data required for this action.
 *   See tpps_submission_form_get_change_list().
 *   Each item is path to the single field.
 *   For more details about format see: tpps_submission_interface_action_path().
 */
function tpps_submission_form_version_action_unset(array &$interface, array $action_data) {
  module_load_include('inc', 'tpps', 'includes/form');
  foreach ($action_data as $key => $item) {
    if (empty($item)) {
      if (tpps_is_si_debug_mode()) {
        $message = 'SI Action "unset" #@key failed. One of the required '
          . 'fields is empty.<br />Value: <pre>@item</pre>.';
        $params = ['@key' => $key, '@item' => print_r($item, 1)];
        tpps_message($message, $params, 'error');
      }
      continue;
    }
    $target = &tpps_form_get_path($interface, $item);
    if (empty($target['parent_element']) || empty($target['field_name'])) {
      if (tpps_is_si_debug_mode()) {
        $message = "SIA Action 'unset' can't find element for #@key."
          . '<br />Value: <pre>@item</pre>. ';
        $params = ['@key' => $key, '@item' => print_r($item, 1)];
        tpps_message($message, $params, 'info');
      }
      continue;
    }
    // Note:
    // $target['parent_element'] is a reference to the element inside $interface.
    // Action!
    unset($target['parent_element'][$target['field_name']]);
  }
}

/**
 * Submission Interface Action 'move'.
 *
 * Given list of fields will be moved/relocate to the new location inside
 * Submission Interface Array.
 *
 * @param array $interface
 *   Submission Interface passed by reference so no need to return it.
 * @param array $action_data
 *   Data required for this action.
 *   See tpps_submission_form_get_change_list().
 *   Each item contains information for a single field relocation and
 *   represented by list of parents (including field name) with 2 keys:
 *   'source' and 'target'.
 *   'source' is a real element path. 'target' is a new (but actually it's
 *   location in verion 1).
 */
function tpps_submission_form_version_action_move(array &$interface, array $action_data) {
  module_load_include('inc', 'tpps', 'includes/form');
  foreach ($action_data as $key => $item) {
    if (empty($item['source']) || empty($item['target'])) {
      continue;
    }
    // Note:
    // 'parent_element' is not a list of parents but a reference to element.
    // To get value use 'field_name' as key inside 'parent_element'.
    $source = &tpps_form_get_path($interface, $item['source']);
    $target = &tpps_form_get_path($interface, $item['target']);
    if (
      empty($source['parent_element']) || empty($source['field_name'])
      || empty($target['parent_element']) || empty($target['field_name'])
    ) {
      if (tpps_is_debug_mode()) {
        $message = 'SIA Action "move" #@key failed. One of the required '
          . 'fields is empty.<br />Value: <pre>@item</pre>.';
        $params = ['@key' => $key, '@item' => print_r($item, 1)];
        tpps_message($message, $params, 'notice');
      }
      continue;
    }
    // Note: $source['parent_element'] and $target['parent_element']
    // are a references to the elements inside $interface.
    if (!isset($source['parent_element'][$source['field_name']])) {
      if (tpps_is_debug_mode()) {
        $message = 'SIA Action "move" #@key failed. Source field not found.'
          . '<br />Value: <pre>@item</pre>.';
        $params = ['@key' => $key, '@item' => print_r($item, 1)];
        tpps_message($message, $params, 'notice');
      }
      continue;
    }
    if (isset($target['parent_element'][$target['field_name']])) {
      if (tpps_is_debug_mode()) {
        $message = 'SIA Action "move": target field exists.'
          . '<br />Value: <pre>@item</pre>.';
        $params = ['@key' => $key, '@item' => print_r($item, 1)];
        tpps_message($message, $params, 'warning');
      }
    }
    // Action!
    $target['parent_element'][$target['field_name']]
      = $source['parent_element'][$source['field_name']];
    unset($source['parent_element'][$source['field_name']]);
  }
}

/**
 * Downgrades Form State/Submission Interface to given version.
 *
 * Note:
 * - Direction is from higher version to lower.
 * - Submsision Form Version 1 = Submission Interface.
 * - Current Submission Form Version is >= 2.
 *
 * ----------------------------------------------------------------------------
 * Usage:
 *
 * To downgrade from latest version to version 1:
 *
 * $interface = tpps_submission_interface_generate($state);
 *
 * To downgrade to specific versions:
 * $current_version = tpps_submission_form_version_get($form_state);
 * $desired_version = TPPS_SUBMISSION_FORM_START_VERSION;
 * for ($i = $current_version; $i >= $desired_version; $i--) {
 *   tpps_submission_form_version_downgrade($form_state, $i, $interface);
 * }
 *
 * @param array $form_state
 *   Submission Form State. Used to build dynamic mapping.
 * @param int $version
 *   Desired version.
 * @param array $result
 *   Resulting array. It could be a Submission Interface if $upgrade = FALSE
 *   and Form State array when $upgrade = TRUE (update old version of the form
 *   to latest).
 *
 * @TODO Check if multiple version could be use: 3 to 1. What about direction?
 */
function tpps_submission_form_version_downgrade(array $form_state, $version, array &$result = NULL) {
  $upgrade = FALSE;
  tpps_submission_form_version_migrate($form_state, $version, $result, $upgrade);
}

/**
 * Upgrades one step Form State/Submission Interface to given version.
 *
 * Note:
 * - Direction is from lower version to higher.
 * - Submsision Form Version 1 = Submission Interface.
 * - Current Submission Form Version is >= 2.
 *
 * ----------------------------------------------------------------------------
 * Usage:
 *
 * To upgrade to specific versions (from 1 to 2):
 *
 * $start_version = TPPS_SUBMISSION_FORM_START_VERSION;
 * $desired_version = tpps_submission_form_version_get($form_state);
 * for ($i = $start_version; $i <= $desired_version; $i++) {
 *   tpps_submission_form_version_upgrade($form_state, $i, $interface);
 * }
 *
 * @param array $form_state
 *   Submission Form State. Used to build dynamic mapping.
 * @param int $version
 *   Desired version.
 * @param array $result
 *   Resulting array. It could be a Submission Interface if $upgrade = FALSE
 *   and Form State array when $upgrade = TRUE (update old version of the form
 *   to latest).
 */
function tpps_submission_form_version_upgrade(array $form_state, $version, array &$result = NULL) {
  $upgrade = TRUE;
  tpps_submission_form_version_migrate($form_state, $version, $result, $upgrade);
}

/**
 * Changes version of given array $result.
 *
 * @param array $form_state
 *   Submission Form State. Used to build dynamic mapping.
 * @param int $version
 *   Desired version.
 * @param array $result
 *   Resulting array. It could be a Submission Interface or Form State array.
 * @param bool $upgrade
 *   When TRUE 'unset' action will be skipped and 'move' action will be
 *   reverted ('source' and 'target' will be swapped).
 *   Use FALSE (default) to get Submission Interface Array.
 *   Use TRUE to upgrade old version of Form State Array.
 */
function tpps_submission_form_version_migrate(array $form_state, $version, array &$result = NULL, $upgrade = FALSE) {
  if (is_null($result)) {
    $result = &$form_state;
  }

  $change_list = tpps_submission_form_version_changes($form_state, $version);
  foreach ($change_list as $action => $action_data) {
    // Note: $action_data is a list of all actions (not just single one).
    if ($upgrade && $action == 'unset') {
      continue;
    }
    $function = 'tpps_submission_form_version_action_' . $action;
    $params = [
      '@function_name' => $function,
      '@version' => $version,
      '@action' => $action,
    ];
    if (!function_exists($function)) {
      $message = 'Submission Interface Action Function not exists. '
        . '<br />Function name: @function_name. <br />Version: @version.';
      tpps_message($message, $params, 'error');
      continue;
    }
    if (empty($action_data)) {
      $message = 'There is no data for Submission Interface Action. '
        . '<br />Function name: @function_name. <br />Version: @version.';
      tpps_message($message, $params, 'error');
      continue;
    }
    if ($upgrade) {
      if ($action == 'move') {
        foreach ($action_data as $key => $item) {
          $reverted_action_data[$key] = [
            'source' => $item['target'],
            'target' => $item['source'],
          ];
        }
        $action_data = $reverted_action_data;
      }
      else {
        $message = 'Reverted action data is required for @action.'
        . '<br />Function name: @function_name. <br />Version: @version.';
        tpps_message($message, $params, 'error');
        continue;
      }
    }
    // Call an action-function.
    // 1. tpps_submission_form_version_action_unset();
    // 2. tpps_submission_form_version_action_move().
    // Note: $action_data also couldn't be empty here.
    call_user_func_array($function, [&$result, $action_data]);
  }
  $result['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] = $version;
}

/**
 * Gets specific version or all version's changes.
 *
 * Note:
 * Version 1 just converts $form_state into Submission Interface by
 * removing elements useless for pipeline processing.
 * Next versions are about change order of fields at form and could be used
 * to update old versions of $form_state to latest version.
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
function tpps_submission_form_version_changes(array $form_state, $version = NULL) {

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
        //['saved_values', TPPS_SUBMISSION_FORM_VERSION_KEY],

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
    '2' => [
      'move' => [
        // Page 1. 'DOI' field.
        'publication_doi' => [
          // Submission Form State. Current location.
          'source' => [
            'saved_values', TPPS_PAGE_1, 'publication', 'publication_doi',
          ],
          // Interface = Submission Form State v1.
          'target' => ['saved_values', TPPS_PAGE_1, 'doi'],
        ],
        'dataset_doi' => [
          'source' => [
            'saved_values', TPPS_PAGE_1, 'publication', 'dataset_doi',
          ],
          'target' => ['saved_values', TPPS_PAGE_1, 'dataset_doi'],
        ],
      ],
    ],
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Version 2. Dynamyc changes.
  // #86ayw8a7k Relocate genotype fields.
  if (!empty($form_state['saved_values'][TPPS_PAGE_1])) {
    $page1_values = $form_state['saved_values'][TPPS_PAGE_1];
    $organism_number = $page1_values['organism']['number'];
    // List of relocated genotype fields from 'files' to 'SNPs' fieldset.
    // ['files'] -> ['SNPs'].
    $files_to_snps_fields = [
      'genotyping-type',
      'file-type',
      'vcf',
      // @TODO process.
     // 'vcf_file-location',
      'snps-assay',
      'assay-design',
      'snps-association',
      'snps-association-tool',
      'snps-association-type',
      'snps-pop-struct',
      'snps-kinship',
      'assay-citation',
    ];

    $to_ssr_fieldset = [
      // Dropdown menu.
      ['SSRs/cpSSRs'],
      // Managed files.
      ['files', 'ploidy'],
      ['files', 'ssrs'],
      ['files', 'ssrs_extra'],
    ];
    for ($i = 1; $i <= $organism_number; $i++) {
      $genotype_parents = [
        'saved_values', TPPS_PAGE_4, 'organism-' . $i, 'genotype',
      ];
      // 'ref-genome' field.
      $field_name = 'ref-genome';
      $change_list['2']['move'][] = [
        'source' => array_merge($genotype_parents, ['SNPs', $field_name]),
        'target' => array_merge($genotype_parents, [$field_name]),
      ];
      // 'files' to 'SNPs' fields.
      foreach ($files_to_snps_fields as $field_name) {
        $change_list['2']['move'][] = [
          'source' => array_merge($genotype_parents, ['SNPs', $field_name]),
          'target' => array_merge($genotype_parents, ['files', $field_name]),
        ];
      }
      // Move to fieldset 'SSRs/cpSSRs Information'.
      // 'source' new location, 'target' - original location.
      foreach ($to_ssr_fieldset as $item) {
        $change_list['2']['move'][] = [
          'source' => array_merge($genotype_parents, ['ssrs_cpssrs', end($item)]),
          'target' => array_merge($genotype_parents, $item),
        ];
      }
    }
  }

  return ($version ? ($change_list[$version] ?? NULL) : $change_list);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// @TODO Create menu callback for testing:
// /tpps/test/<function_name>/arg/arg/arg.

/**
 * Debug of tpps_submission_form_version_changes().
 *
 * @param string $accession
 *   Submission Accession. E.g., 'TGDR12345'.
 * @param int $version
 *   Submission Form Version.
 */
function test_tpps_submission_form_version_changes($accession, $version = 2) {
  $state = tpps_load_submission_state($accession);
  $changes = tpps_submission_form_version_changes($state, $version);
  dpm(var_export($changes, 1));
}

/**
 * Unit Test for tpps_submission_form_version_action_move().
 *
 * @param string $accession
 *   Submission Accession. E.g., 'TGDR123'.
 * @param int $version
 *   Submission Form Version.
 */
function test_tpps_submission_form_version_action_move($accession, $version = 2) {
  module_load_include('inc', 'tpps', 'includes/submission_interface');
  module_load_include('inc', 'tpps', 'includes/form');

  $state = tpps_load_submission_state($accession);
  $changes = tpps_submission_form_version_changes($state, $version);
  $item = $changes['move'][14];
  dpm(print_r($item, 1), 'item');

  $type_list = ['source', 'target'];
  foreach ($type_list as $type) {
    dpm($type);
    $result = tpps_form_get_path($state, $item[$type]);
    dpm(print_r($result, 1), 'Result ' . $type);
  }
}
