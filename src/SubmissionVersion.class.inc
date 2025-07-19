<?php

/**
 * @file
 * Contains 'SubmissionVersion' class.
 */

// Get version's changes.
module_load_include('inc', 'tpps', 'includes/submission_form_version_changes');

/**
 * Overview.
 *
 * We need to have version of the Submission's Form to be able to convert
 * Form State Array into SharedState used by pipeline processing
 * (see form/submit/submit_all.php script).
 *
 * ----------------------------------------------------------------------------
 * What is Submission Version?
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
 * - Submission SharedState has NO version. It equals to Submission Form Version 1.
 * - Submission Form (not $form_state) has ALWAYS latest version
 *   (see static::getLatest()).
 * - Submission Form State of imported (usually old, version 1 or lower then
 *   latest verstion) studies must be UPgranded.
 * - To generate Submission Shared Statee from current Submission Form State
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
 * There is no need to update Submission::flatten() because it
 * uses Submission Shared State.
 *
 * How to change/modify structure of the Submission Shared State?
 * 1. Update submit_all.php
 * 2. Update this file.
 * 3. tpps_table_display().
 * 4. Submission::flatten().
 * 5. Other code which uses Submission Interface.
 *
 * ----------------------------------------------------------------------------
 * API.
 *
 * To get version:
 *    static::get($form_state);
 * To set version:
 *    static::set($form_state);
 *
 * ----------------------------------------------------------------------------
 * Predefined actions:
 * 1. 'unset' - removes item from resulting (not original) array.
 * 2. 'move' - relocates item (and it's sub-elements) to new place in resulting
 *             (not original) array.
 * 3. 'rename' = move where target has new field name (last element).
 *
 * ----------------------------------------------------------------------------
 * What is changelist?
 *
 * Change List for each version is a list of data to perform actions
 * (unset, move). For 'unset' it will be path to elements (list of parents
 * including field which must be removed).
 * For 'move' it will be 'source' (where to get) and 'target' (where to put).
 *
 * ----------------------------------------------------------------------------
 * When new version must be created?
 *
 * 1. Existing field relocated/renamed.
 *
 * No version for:
 * 1. New fields.
 * 2. Removed fields.
 *
 * ----------------------------------------------------------------------------
 *  Versions:
 *  See tpps_submission_form_get_change_list() for changes per version.
 *  Version 1: Only removed data which is useless for processing study.
 *  Version 2: #86ayw8a7k Page 4 Genotype Logic Adjustments.
 *  Version 3: Relocated 'Assay Design File' field.
 * ----------------------------------------------------------------------------
 * @todo Minor. If number of versions will be big enough and there is
 *       no old versions left we could remove those versions from
 *       processing to speed-up interface creation.
 * @todo downgrade()/upgrade() returns result of the processing now but it's
 * better to use hooks for logging and return resulting array instead.
 */

/**
 * Manages Submission Form Version.
 *
 * @package tpps
 * @author Vlad Savistky <vlad.savitsky@gmail.com>
 */
class SubmissionVersion {

  // Name of the key which stores Submission Form version.
  const FORM_KEY = 'submission_form_version';

  // Number of the initial version of Form State Array.
  // This value will be used to apply incremental changes (patches).
  // Need to avoid 'magic numbers'.
  const VERSION_1 = 1;

  /**
   * Checks if Debug Mode is enabled.
   *
   * @return bool
   *   Returns TRUE if 'Submission Interface Debug Mode' is enabled.
   *   Returns FALSe otherwise.
   */
  public static function isDebugMode() {
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
  public static function isOutdated(array $form_state) {
    $form_version = static::get($form_state);
    // Not use current version to assume form is outdated and need to be
    // upgraded. So it's '1' version.
    return ($form_version != static::getLatest());
  }

  /**
   * Gets Submission Form Version.
   *
   * See also $submission->getFormVersion().
   *
   * @param array $form_state
   *   Submission's Form State array.
   *
   * @return int
   *   Returns Submission's Form version.
   *   Returns 1 (default value) if version wasn't set.
   */
  public static function get(array $form_state) {
    return $form_state['saved_values'][static::FORM_KEY] ?? static::VERSION_1;
  }

  /**
   * Sets Submission Form Version.
   *
   * Note: Form State in database will be updated.
   * See Submission::setFormVersion().
   *
   * @param array $form_state
   *   Form State array.
   * @param int $version
   *   Submission Form Version starting from 1.
   * @param bool $force
   *   Force setting of given version.
   */
  public static function set(array &$form_state, $version, $force = FALSE) {
    if ($force) {
      $form_state['saved_values'][static::FORM_KEY] = $version;
    }
    else {
      $form_state['saved_values'][static::FORM_KEY] =
        // Do not overwrite if version was set.
        $form_state['saved_values'][static::FORM_KEY]
        ?? $version
        ?? static::VERSION_1;
    }
  }

  /**
   * Downgrades Submission Form Version.
   *
   * From current version to given (or 1st version if $version wan't specified).
   * WARNING: Submission won't be updated.
   *
   * Note:
   * - v1 is SharedState.
   * - SharedState is missing some elements which not used by pipeline processing script.
   *
   * ----------------------------------------------------------------------------
   * Usage:
   *
   * To downgrade from current version to 1st (initial) version:
   *   $result = SubmissionVersion::downgrade($form_state);
   * To downgrade to specific versions (4 to 3):
   *   $result = SubmissionVersion::downgrade($form_state, $final_version = 2);
   *
   * @param array $form_state
   *   Submission Form State array.
   * @param int $final_version
   *   Desired version.
   *
   * @return array
   *   Returns form state array upgraded/downgrated to desired version.
   */
  public static function downgrade(array $form_state, $final_version = NULL) {
    $result = $form_state;
    $accession = tpps_form_get_accession($result);

    $final_version = $final_version ?? static::VERSION_1;
    $form_version = static::get($result);
    if ($form_version == static::VERSION_1) {
      return $result;
    }

    // Note: We use '>' (not '>=') because version 1 has no changes.
    for ($version = $form_version; $version >= $final_version; $version--) {
      $direction = 'downgrade';
      $result = static::migrate($form_state, $version, $direction);
      // tpps_submission_version_downgraded().
      if (function_exists('module_invoke_all')) {
        module_invoke_all('submission_version_downgraded', [
          'from_version' => $version,
          'to_version' => $version - 1,
          'accession' => $accession,
        ]);
      }
    }
    return $result;
  }

  /**
   * Upgrades Submission Form Version to latest.
   *
   * WARNING: Submission and DB won't be updated.
   * Note:
   * - Direction is from lower version to higher.
   * - Submsision Form Version 1 = Submission Interface.
   * - Current Submission Form Version is >= 2.
   *
   * ----------------------------------------------------------------------------
   * Usage:
   *
   * To upgrade to the latest version:
   *   $result = SubmissionVersion::upgrade($form_state);
   * To upgrade to specific versions (from 3 to 4):
   *   $desired_version = SubmissionVersion::get($form_state);
   *   $result = SubmissionVersion::upgrade($form_state, $desired_version);
   *
   * @param array $form_state
   *   Submission Form State array.
   * @param int $final_version
   *   Desired version.
   *
   * @return array
   *   Returns form state array upgraded/downgrated to desired version.
   */
  public static function upgrade(array $form_state, $final_version = NULL) {
    $result = $form_state;
    $accession = tpps_form_get_accession($result);
    $final_version = $final_version ?? static::getLatest();
    $form_version = static::get($result);
    // Skip 1st version because it's empty. 1st version is initial state which
    // do not require any updates.
    $start_version = ($form_version == static::VERSION_1) ? 2 : $form_version;
    if ($start_version > $final_version) {
      return $result;
    }
    for ($version = $start_version; $version <= $final_version; $version++) {
      // Note: $version is a next version.
      $direction = 'upgrade';
      $result = static::migrate($form_state, $version, $direction);
      // tpps_submission_version_upgraded().
      if (function_exists('module_invoke_all')) {
        module_invoke_all('submission_version_upgraded', [
          'from_version' => $version - 1,
          'to_version' => $version,
          'accession' => $accession,
        ]);
      }
    }
    return $result;
  }

  /**
   * Changes version of given array $result.
   *
   * @param array $form_state
   *   Submission Form State. Used to build dynamic mapping.
   * @param int $final_version
   *   Desired version.
   * @param string $direction
   *   Direction of changes. Could be 'upgrade' (default) and 'downgrade'.
   *   Use 'upgrade' to get latest version.
   *   Use 'downgrade to get older version:
   *   1. 'unset' action will be skipped
   *   2. 'move' action will be  reverted ('source' and 'target' will be swapped).
   *
   * @return array
   *   Returns form state array upgraded/downgrated to desired version.
   */
  public static function migrate(array $form_state, $final_version, $direction = 'upgrade') {
    $result = $form_state;
    $accession = tpps_form_get_accession($result);
    if (
      // Downgrade with the same version must be processed.
      // It's a magic of versions.
      $direction == 'upgrade'
      // Check if final and current versions are the same.
      && $final_version == static::get($result)
    ) {
      return $result;
    }

    $change_list = tpps_submission_form_version_changes($form_state, $final_version);
    if (empty($change_list)) {
      $message = 'SubmissionVersion: Got empty change list.';
      throw new Exception($message);
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
    if (!empty($change_list)) {
      tpps_array_apply_changes($change_list, $result);
      switch ($direction) {
        case 'upgrade':
          $version = $final_version;
          break;

        case 'downgrade':
          // @TODO Minor. Remove after testing those extra check
          // cause it's overkill but left to be sure.
          $version = static::VERSION_1;
          if ($final_version > static::VERSION_1) {
            $version = $final_version - 1;
          }
          break;
      }
      $result['saved_values'][static::FORM_KEY] = $version;
    }
    return $result;
  }

  /**
   * Get latest Submission Form Version.
   *
   * WARNING:
   * Depends on tpps_submission_form_version_changes() so be sure to create
   * empty version's element even if all items created dynamically (in loop).
   *
   * @return int
   *   Returns latest existing version.
   */
  public static function getLatest() {
    $form_state = [];
    $change_list = tpps_submission_form_version_changes($form_state);
    dpm($change_list);
    return count($change_list);
  }

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
  $submission = new Submission($accession);
  $changes = tpps_submission_form_version_changes($submission->state, $version);
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
  module_load_include('inc', 'tpps', 'includes/form');

  $submission = new Submission($accession);
  $changes = tpps_submission_form_version_changes($submission->state, $version);
  $item = $changes['move'][14];
  drupal_set_message(print_r($item, 1), 'item');

  $type_list = ['source', 'target'];
  foreach ($type_list as $type) {
    drupal_set_message($type);
    $result = tpps_form_get_path($state, $item[$type]);
    drupal_set_message(print_r($result, 1), 'Result ' . $type);
  }
}
