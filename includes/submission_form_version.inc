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
 * - To upgrade use exact version (to get '2' use '2').
 * - To downgrade use previous upper (higher) version (to get '1' use '2').
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
function tpps_submission_form_version_is_outdated(array $form_state) {
  $form_version = $form_state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY]
    // Not use current version to assume form is outdated and need to be
    // upgraded. So it's '1' version.
    ?? 1;
  return ($form_version != TPPS_SUBMISSION_FORM_CURRENT_VERSION);
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
 * Note: Form State in database will be updated.
 *
 * @param array $form_state
 *   Form State array.
 * @param int $version
 *   Submission Form Version starting from 1.
 * @param bool $force
 *   Force setting of given version.
 */
function tpps_submission_form_version_set(array &$form_state, $version, $force = FALSE) {
  if ($force) {
    $form_state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] = $version;
  }
  else {
    $form_state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] =
      // Do not overwrite if version was set.
      $form_state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY]
      ?? $version
      ?? TPPS_SUBMISSION_FORM_START_VERSION;
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
 * To downgrade from latest version to 1st (initial) version:
 *   tpps_submission_form_version_downgrade_to_latest($form_state, $result);
 *
 * To downgrade to specific versions (4 to 3):
 *   $current_version = tpps_submission_form_version_get($form_state);
 *   $desired_version = TPPS_SUBMISSION_FORM_START_VERSION;
 *   for ($i = $current_version; $i >= $desired_version; $i--) {
 *     tpps_submission_form_version_downgrade($form_state, $i, $interface);
 *   }
 *
 * @param array $form_state
 *   Submission Form State. Used to build dynamic mapping.
 * @param int $version
 *   Desired version.
 * @param array $result
 *   Resulting array.
 *
 * @return bool
 *   Returns TRUE in case of success and FALSE otherwise.
 */
function tpps_submission_form_version_downgrade(array $form_state, $version, array &$result) {
  if (tpps_submission_form_version_get($result) == 1) {
    return FALSE;
  }
  $direction = 'downgrade';
  $success = tpps_submission_form_version_migrate($form_state, $version, $result, $direction);
  if (tpps_is_debug_mode()) {
    tpps_message(
      '@type of @accession was @directiond from @from_version to @to_version '
        . 'version: @status',
      [
        '@type' => (tpps_form_is_submission_interface($result)
          ? t('Interface') : t('State')),
        // Downgrade to version 1 but use change list for update to version 2.
        '@from_version' => $version,
        '@to_version' => $version - 1,
        '@direction' => $direction,
        '@accession' => tpps_form_get_accession($result),
        '@status'  => ($success ? 'Success' : 'Failed'),
      ]
    );
  }
  return $success;
}

/**
 * Downgrades Submission Form Version to 1st (initial) version.
 *
 * WARNING:
 * $result will be updated but not saved to database.
 *
 * Note; Submission From Verion 1 = Submission Interface.
 * But Submission Interface is missing some elements which not used by pipeline
 * processing script.
 *
 * @param array $form_state
 *   Submission Form State array.
 * @param array $result
 *   Resulting array.
 *
 * @return bool
 *   Returns TRUE in case of success and FALSE otherwise.
 *
 * @TODO Rename because it can't be 'downgrade to the latest'.
 * Revert.
 */
function tpps_submission_form_version_downgrade_to_latest(array $form_state, array &$result) {
  $form_version = tpps_submission_form_version_get($result);
  if ($form_version == 1) {
    // @TODO Should it be TRUE or FALSE?
    return TRUE;
  }
  $success = TRUE;
  for ($version = $form_version; $version > 1; $version--) {
    $success = tpps_submission_form_version_downgrade($form_state, $version, $result) && $success;
  }
  // Returns TRUE only if each calls of the function in the loop was successful.
  return $success;
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
 * To upgrade to the latest version:
 *   tpps_submission_form_version_upgrade_to_latest($form_state, $result);
 *
 * To upgrade to specific versions (from 3 to 4):
 *   $start_version = TPPS_SUBMISSION_FORM_START_VERSION;
 *   $desired_version = tpps_submission_form_version_get($form_state);
 *   for ($i = $start_version; $i <= $desired_version; $i++) {
 *     tpps_submission_form_version_upgrade($form_state, $i, $interface);
 *   }
 *
 * @param array $form_state
 *   Submission Form State. Used to build dynamic mapping.
 * @param int $version
 *   Desired version.
 * @param array $result
 *   Resulting array.
 *
 * @return bool
 *   Returns TRUE in case of success and FALSE otherwise.
 *
 * @TODO Merge with tpps_submission_form_version_downgrade().
 */
function tpps_submission_form_version_upgrade(array $form_state, $version, array &$result) {
  $form_version = tpps_submission_form_version_get($result);
  if ($form_version == $version) {
    return FALSE;
  }
  $direction = 'upgrade';
  $success = tpps_submission_form_version_migrate($form_state, $version, $result, $direction);
  if (tpps_is_debug_mode()) {
    tpps_message(
      '@type of @accession was @directiond from @from_version to @to_version '
        . 'version: @status',
      [
        '@type' => (tpps_form_is_submission_interface($result)
          ? t('Interface') : t('State')),
        '@from_version' => $form_version,
        '@to_version' => $version,
        '@direction' => $direction,
        '@accession' => tpps_form_get_accession($result),
        '@status'  => ($success ? 'Success' : 'Failed'),
      ]
    );
  }
  return $success;
}

/**
 * Upgrades Submission Form Version to latest.
 *
 * WARNING:
 * $result will be updated but not saved to database.
 *
 * @param array $form_state
 *   Submission Form State array.
 * @param array $result
 *   Resulting array.
 *
 * @return bool
 *   Returns TRUE in case of success and FALSE otherwise.
 */
function tpps_submission_form_version_upgrade_to_latest(array $form_state, array &$result) {
  $form_version = tpps_submission_form_version_get($result);
  // Skip 1st version.
  $start_version = ($form_version == 1) ? 2 : $form_version;
  $latest_version = TPPS_SUBMISSION_FORM_CURRENT_VERSION;
  if ($start_version > $latest_version) {
    return FALSE;
  }
  $success = TRUE;
  for ($version = $start_version; $version <= $latest_version; $version++) {
    // Note: $version is a next version.
    $success = tpps_submission_form_version_upgrade($form_state, $version, $result) && $success;
  }
  // Returns TRUE only if each calls of the function in the loop was successful.
  return $success;
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
 * @param string $direction
 *   Direction of changes. Could be 'upgrade' (default) and 'downgrade'.
 *   Use 'upgrade' to get latest version.
 *   Use 'downgrade to get older version:
 *   1. 'unset' action will be skipped
 *   2. 'move' action will be  reverted ('source' and 'target' will be swapped).
 *
 * @return bool
 *   Returns TRUE in case of success and FALSE otherwise.
 */
function tpps_submission_form_version_migrate(array $form_state, $version, array &$result, $direction = 'upgrade') {
  // Downgrade with the same version must be processed. It's a magic of versions.
  if (
    $direction == 'upgrade'
    && $version == tpps_submission_form_version_get($result)
  ) {
    return TRUE;
  }

  $change_list = tpps_submission_form_version_changes($form_state, $version);
  if (empty($change_list)) {
    return FALSE;
  }
  // Prepare list of changes. Downgrade and upgrade requires different set of
  // action's data.
  if ($direction == 'downgrade') {
    // Revert data for 'downgrade'.
    foreach ($change_list as $action => $action_data) {
      if ($action == 'unset') {
        continue;
      }
      if ($action == 'move') {
        foreach ($action_data as $key => $item) {
          $new_change_list[$action][$key] = [
            'source' => $item['target'],
            'target' => $item['source'],
          ];
        }
        // @TODO Try to avoid extra array here.
        $change_list = $new_change_list;
      }
      else {
        $message = 'Reverted action data is required for @action.';
        tpps_message($message, [], 'error');
        continue;
      }
    }
  }
  if (empty($change_list)) {
    return FALSE;
  }
  tpps_array_apply_changes($change_list, $result);

  if ($direction == 'upgrade') {
    $result['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY] = $version;
  }
  elseif ($direction == 'downgrade') {
    // @TODO Minor. Remove after testing those extra check
    // cause it's overkill but left to be sure.
    $result['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY]
      = (($version > 1) ? ($version - 1) : 1);
  }
  return TRUE;
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
  $version_change_list = [
    // Version 1 is a initial version.
    // Submission Interface is a version 1 without some unnecessary elements.
    // See tpps_submission_interface_purify();
    '1' => [],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // #86ayw8a7k Genotype Logic changes.
    // Define how to get Submission Form Version #2.
    '2' => [
      // v1 -----> v2.
      'move' => [
        // Page 1. 'DOI' field.
        'publication_doi' => [
          // v1.
          'source' => ['saved_values', TPPS_PAGE_1, 'doi'],
          // v2.
          'target' => [
            'saved_values', TPPS_PAGE_1, 'publication', 'publication_doi',
          ],
        ],
        'dataset_doi' => [
          'source' => ['saved_values', TPPS_PAGE_1, 'dataset_doi'],
          'target' => [
            'saved_values', TPPS_PAGE_1, 'publication', 'dataset_doi',
          ],
        ],
      ],
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Version 2. Dynamyc changes.
  // #86ayw8a7k Relocate genotype fields.
  if (!empty($form_state['saved_values'][TPPS_PAGE_1])) {
    $page1_values = $form_state['saved_values'][TPPS_PAGE_1];
    $organism_number = $page1_values['organism']['number'];
    // List of relocated genotype fields from 'files' to 'SNPs' fieldset.
    // ['files'] -> ['SNPs'].
    $snps_fieldset = 'SNPs';
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
      // Yes/No field.
      'upload_snp_association',
    ];
    $to_other_fieldset = [
      // Textfield (must be select).
      ['files', 'other-marker'], // Select.
      ['files', 'other'], // Managed file.
      ['files', 'other-columns'],
      // Other file-field subfields.
      ['files', 'other', 'dynamic'],
      ['files', 'other', 'columns'],
      ['files', 'other', 'columns-options'],
      ['files', 'other', 'no-header'],
      // Files.
    ];

    $ssrs_fieldset = 'ssrs_cpssrs';
    $to_ssr_fieldset = [
      // Dropdown menu.
      ['SSRs/cpSSRs'],
      // Managed files.
      ['files', 'ploidy'],
      ['files', 'ssrs'],
      ['files', 'ssrs_extra'],
    ];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    for ($i = 1; $i <= $organism_number; $i++) {
      $genotype_parents = [
        'saved_values', TPPS_PAGE_4, 'organism-' . $i, 'genotype',
      ];
      // 'ref-genome' field.
      $field_name = 'ref-genome';
      $version_change_list['2']['move'][] = [
        'source' => array_merge($genotype_parents, [$field_name]),
        'target' => array_merge(
          $genotype_parents,
          [$snps_fieldset, $field_name]
        ),
      ];
      // 'files' to 'SNPs' fields.
      foreach ($files_to_snps_fields as $field_name) {
        $version_change_list['2']['move'][] = [
          'source' => array_merge($genotype_parents, ['files', $field_name]),
          'target' => array_merge($genotype_parents, ['SNPs', $field_name]),
        ];
      }
      // Move to fieldset 'SSRs/cpSSRs Information'.
      // 'source' new location, 'target' - original location.
      foreach ($to_ssr_fieldset as $item) {
        $version_change_list['2']['move'][] = [
          'source' => array_merge($genotype_parents, $item),
          'target' => array_merge(
            $genotype_parents,
            [$ssrs_fieldset, end($item)]
          ),
        ];
      }
      // ['genotype'] -> ['genotype', 'other']
      foreach ($to_other_fieldset as $item) {
        $version_change_list['2']['move'][] = [
          'source' => array_merge($genotype_parents, $item),
          'target' => array_merge($genotype_parents, ['other', end($item)]),
        ];
      }
    }
  }
  // End of Verion 2 Dynamic Changes.

  return ($version ? ($version_change_list[$version] ?? NULL) : $version_change_list);
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
  drupal_set_message(var_export($changes, 1));
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
  drupal_set_message(print_r($item, 1), 'item');

  $type_list = ['source', 'target'];
  foreach ($type_list as $type) {
    drupal_set_message($type);
    $result = tpps_form_get_path($state, $item[$type]);
    drupal_set_message(print_r($result, 1), 'Result ' . $type);
  }
}
