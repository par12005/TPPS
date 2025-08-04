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
 * Note:
 * - v1 is SharedState.
 * - SharedState is missing some elements which not used by pipeline processing script.
 * - Changes from current version.
 * - Original array won't be updated. Changed array will be returned.
 * - Record in DB won't be updated.
 * WARNING: Submission object and records in DB won't be updated.
 *
 * ----------------------------------------------------------------------------
 * What is Submission Version?
 * 1. Versions are integers and start from '1' which is
 *    defined as SubmissionVersion::VERSION_1 constant.
 * 2. Each version changes must be listed in function
 *    tpps_submission_interface_get_change_list().
 * 3. Version's value is stored under $submission_state['saved_values'] because
 *    'saved_values' automatically stored/restored by TPPS module for each step.
 * 4. Version change list describes upgrade to given verion from previous. E.g.,
 *    1: nothing
 *    2: from v1 to v2.
 *    v1 -> v2: Current version is 1. Get changes for v2 and apply.
 *    To downgrade we need to use higher version changes  in reverse mode. E.g.,
 *    v2 -> v1: Current version is 2. Get changes for v2, reverse and apply.
 * 5. To upgrade/downgrade must be used desired final version number
 *    (not increased or decreased).
 *
 * ----------------------------------------------------------------------------
 * Downgrade or upgrade?
 *
 * - Direction is from lower version to higher.
 *   Upgrade: v1 -> v2,
 *   Downgrade: v2 -> v1.
 * - To avoid detection of migration direction use
 *   SubmissionVersion::set($submission_state, $version);
 *   or
 *   SubmissionVersion::upgrade($submission_state, $version);
 *   SubmissionVersion::downgrade($submission_state, $version);
 *
 * - Submission's SharedState is version 1. To get:
 *   SubmissionVersion::set($submission_state, 1);
 *   or
 *   SubmissionVersion::downgrade($submission_state);
 * - Created new submission's must have latest version (see static::getLatest()).
 * - Imported submissions could have not latest version. Their version could
 *   be changed during import process (depends on settings on import form).
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
 *    SubmissionVersion::get($submission_state);
 * To upgrade/downgrade to version:
 *    SubmissionVersion::set($submission_state, $version);
 * To set version (without upgrade/downgrade):
 *    SubmissionVersion::set($submission_state, $version, $no_migration = TRUE);
 *    or
 *    $submission->setFormVersion($version);
 *
 * ----------------------------------------------------------------------------
 * Predefined actions:
 * 1. 'unset' - removes item from resulting (not original) array.
 * 2. 'move' - relocates item (and it's sub-elements) to new place in resulting
 *             (not original) array.
 * 3. 'rename' = move where target has new field name (last element).
 * 4. 'create' - do nothing but allows to track changes.
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
 *  Version changes:
 *  1: Only removed data which is useless for processing study.
 *  2: #86ayw8a7k Page 4 Genotype Logic Adjustments.
 *  3: Relocated 'Assay Design File' field.
 *
 *  See tpps_submission_form_get_change_list() for changes per version.
 * ----------------------------------------------------------------------------
 * @todo Minor. If number of versions will be big enough and there is
 *       no old versions left we could remove those versions from
 *       processing to speed-up interface creation.
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
   * Checks if current version of the Submission Form State array is outdated.
   *
   * @param array $submission_state
   *   Submission's Form State array.
   *
   * @return bool
   *   Returns TRUE if version of the Submission Form State array lower then
   *   latest version defined in tpps_submission_form_version_changes().
   *   Returns FALSE if current version equals to the latest version.
   */
  public static function isOutdated(array $submission_state) {
    $current_version = static::get($submission_state);
    // Not use current version to assume form is outdated and need to be
    // upgraded. So it's '1' version.
    return ($current_version != static::getLatest());
  }

  /**
   * Gets version of the  Submission Form State array.
   *
   * See also $submission->getFormVersion().
   *
   * @param array $submission_state
   *   Submission's Form State array.
   *
   * @return int
   *   Returns Submission's Form version.
   *   Returns latest possible version if version wasn't set.
   */
  public static function get(array $submission_state) {
    return $submission_state['saved_values'][static::FORM_KEY] ?? static::getLatest();
  }

  /**
   * Sets Submission Form Version.
   *
   * Note: Form State in database won't be updated.
   * To set version without upgrade/downgrade:
   *   $submission->setFormVersion($version);
   *
   * @param array $submission_state
   *   Submission Form State array.
   * @param int $version
   *   Submission Form Version.
   * @param bool $migrate
   *   Defines if state array should be upgraded/downgraded. Default is TRUE.
   *   When FALSE then only version number will be changed. This could be used
   *   for troubleshooting and debugging.
   *
   * @return array
   *   Returns form state array upgraded/downgrated to desired version.
   */
  public static function set(array $submission_state, $version = NULL, $migrate = TRUE) {
    if (is_null($version)) {
      // @TODO
      $message = "Version can't be empty.";
      $params = [
        '@stack' => print_r(debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT
          | DEBUG_BACKTRACE_IGNORE_ARGS, 5), 1),
      ];
      watchdog('tpps', __METHOD__ . ' ' . $message, $params, WATCHDOG_ERROR);
      throw new Exception($message);
    }
    $current_version = static::get($submission_state);
    if ($current_version == $version) {
      return $submission_state;
    }
    if ($migrate) {
      if ($current_version > $version) {
        return static::downgrade($submission_state, $version);
      }
      else {
        return static::upgrade($submission_state, $version);
      }
    }
    else {
      $submission_state['saved_values'][static::FORM_KEY] = $version
        ?? static::getLatest();
      return $submission_state;
    }
  }

  /**
   * Downgrades current version of the Submission Form.
   *
   * Usage:
   *
   * To downgrade to 1st (initial) version:
   *   $result = SubmissionVersion::downgrade($submission_state);
   *   or
   *   $result = SubmissionVersion::set($submission_state, $version = 1);
   *
   * To downgrade to specific versions:
   *   $result = SubmissionVersion::downgrade($submission_state, $version = 2);
   *   or
   *   $result = SubmissionVersion::set($submission_state, $version = 2);
   *
   * @param array $submission_state
   *   Submission Form State array.
   * @param int $final_version
   *   Desired version.
   *
   * @return array
   *   Returns form state array downgrated to desired version.
   */
  public static function downgrade(array $submission_state, $final_version = NULL) {
    // Skip if current version is '1'.
    $result = $submission_state;
    $current_version = static::get($submission_state);
    $final_version = $final_version ?? static::VERSION_1;

    if ($current_version == $final_version) {
      return $result;
    }
    $accession = $result['accession'] ?? NULL;
    // Note: We use '>' (not '>=') because version 1 has no changes.
    // and we use $version+1 changes in reverse to downgrade.
    for ($version = $current_version; $version > $final_version; $version--) {
      // Problem:
      // Line below updates not only SharedState array but 'state' property
      // of Submission object which called generateSharedState() method.
      // So $submission->state became downgraded to 1st version.
      $result = static::migrate($result, $version, 'downgrade');
      if (function_exists('module_invoke_all')) {
        // tpps_submission_version_downgraded().
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
   * Upgrades current version of Submission Form Version to latest version.
   *
   * Usage:
   *
   * To upgrade to the latest version:
   *   $result = SubmissionVersion::upgrade($submission_state);
   *   or
   *   $result = SubmissionVersion::set($submission_state, $version = 2);
   *
   * To upgrade to specific versions (from 3 to 4):
   *   $result = SubmissionVersion::upgrade($submission_state, $version = 2);
   *   or
   *   $result = SubmissionVersion::set($submission_state, $version = 2);
   *
   * @param array $submission_state
   *   Submission Form State array.
   * @param int $final_version
   *   Desired version.
   *
   * @return array
   *   Returns form state array upgraded/downgrated to desired version.
   */
  public static function upgrade(array $submission_state, $final_version = NULL) {
    $current_version = static::get($submission_state);
    $final_version = $final_version ?? static::getLatest();
    $result = $submission_state;

    if ($current_version > $final_version) {
      // @TODO Log that version wasn't updated because $final_version lower.
      return $result;
    }

    $accession = $result['accession'] ?? NULL;
    for ($version = $current_version + 1; $version <= $final_version; $version++) {
      // Note: $version is a next version.
      $result = static::migrate($result, $version, 'upgrade');
      if (function_exists('module_invoke_all')) {
        // tpps_submission_version_upgraded().
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
   * @param array $submission_state
   *   Submission Form State array.
   * @param int $final_version
   *   Desired version.
   * @param string $direction
   *   Direction of changes. Could be 'upgrade' (default) and 'downgrade'.
   *   Use 'upgrade' to get latest version.
   *   Use 'downgrade' to get older version:
   *   1. 'unset' and 'create' actions will be skipped.
   *   2. 'move' action will be  reverted ('source' and 'target' will be swapped).
   *
   * @return array
   *   Returns form state array upgraded/downgrated to desired version.
   */
  public static function migrate(array $submission_state, $final_version, $direction = 'upgrade') {
    $result = $submission_state;
    // Upgrade to the same version must be skipped.
    // Note: Downgrade to the same version must be processed because we use
    // previous version's changes in reverse.
    $current_version = static::get($result);
    if ($direction == 'upgrade' && $final_version == $current_version) {
      // @TODO Check if we should skip processing if it's 'downgrade' direction.
      return $result;
    }

    $accession = $result['accession'] ?? NULL;

    $change_list = tpps_submission_form_version_changes($result, $final_version);
    if (is_null($change_list)) {
      $message = 'SubmissionVersion: Got empty change list.';
      throw new Exception($message);
    }
    // Prepare list of changes. Downgrade and upgrade requires different set of
    // action's data.
    if ($direction == 'downgrade') {
      // Revert data for 'downgrade'.
      foreach ($change_list as $action => $action_data) {
        switch ($action) {
          case 'unset':
          case 'create':
            // @todo For 'create' we could revert and unset field.
            unset($change_list[$action]);
            break;

          case 'move':
            foreach ($action_data as $key => $item) {
              $new_change_list[$action][$key] = [
                'source' => $item['target'],
                'target' => $item['source'],
              ];
            }
            // @TODO Try to avoid extra array here.
            $change_list[$action] = $new_change_list[$action];
            break;

          default:
            $message = 'Reverted action is required for "@action" action to '
              . 'downgrade Submission Version.';
            tpps_message($message, ['@action' => $action], 'error');
            break;
        }
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if (!empty($change_list)) {
      $result = tpps_array_apply_changes($change_list, $result);
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
    $change_list = tpps_submission_form_version_changes($submission_state = []);
    // @todo Minor. Sort and get key of the latest element. This will protect
    // from gaps in versions or extra elements in change list array.
    return count($change_list);
  }

}
