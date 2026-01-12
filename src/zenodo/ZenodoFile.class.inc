<?php

/**
 * @file
 * Contains 'ZenodoFile' class.
 */

/**
 * Zenodo API for Deposition Files.
 *
 * See: https://developers.zenodo.org/#deposition-files.
 *
 * Inherits CRUD actions for files from parent 'Zenodo' class:
 *   create()
 *   delete(array $data)
 *   list(array $data)
 *   retrieve(array $data)
 *   update(array $data)
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class ZenodoFile extends Zenodo {

  // Prefix of the API URL.
  const BASE_PATH = '/api/deposit/depositions';

  // Limit of the single file published using Zenodo service.
  // 20Gb.
  const FILE_SIZE_LIMIT = 20000000000;

  // Name of the key in Zenodo File Entity which holds checksum.
  const CHECKSUM_KEY = 'checksum';

  /**
   * Deletes single file.
   *
   * If the Deposition is a Draft (Unpublished)
   *
   * You do not need to remove files manually if your goal is to delete
   * the entire deposition.
   *  - Total Deletion:
   *    Simply delete the draft deposition via the API or web interface.
   *    This automatically removes all associated files.
   *    https://help.zenodo.org/docs/deposit/create-new-upload/
   *  - Specific File Correction:
   *    If you only want to replace or remove one file, you should delete that
   *    specific file manually using the Zenodo REST API or the "Remove"
   *    icon in the web UI.
   *
   * If the Deposition is Published
   *
   *   As of January 2026, you have a limited window to manage published
   *   files yourself without a support ticket:
   *   - The 30-Day Window:
   *     You can now modify or delete your own record—including its
   *     files—within 30 days after publication.
   *   - After 30 Days:
   *     You cannot remove files manually. Deletion requires submitting a
   *     formal request to Zenodo Support with a valid justification (e.g.,
   *     copyright issues or sensitive data exposure).
   *     https://help.zenodo.org/docs/deposit/manage-records/
   *   - The "Tombstone" Policy:
   *     Even if you delete a record within the 30-day window, Zenodo will
   *     leave a "tombstone" page to ensure existing citations do not lead
   *     to a broken link.
   *
   * @param array $data
   *   Data for request. Required keys are:
   *   - 'deposit_id' - int. Zenodo Deposit Id.
   *   - 'file_id' - string. Zenodo File Id.
   *   - 'drupal_file' - object. Fully loaded Drupal File object.
   */
  public static function delete(array $data) {
    Zenodo::delete($data);
  }

  /**
   * Updates an existing deposition/deposition file resource.
   *
   * Zenodo API doesn't allow PUT- and POST-methods to update files.
   * update-request doesn't work and allows to update only filename.
   * So we use delete-and-create to update update as Zenodo documentation
   * suggests because they switched to the new Bucket API.
   *
   * Scope (permissions): deposit:write.
   *
   * @param array $data
   *   Data for request. Required keys are:
   *   - 'deposit_id' - int. Zenodo Deposit Id.
   *   - 'file_id' - string. Zenodo File Id.
   *   - 'drupal_file' - object. Fully loaded Drupal File object.
   *
   * @return array
   *   Returns Zenodo Deposition data or empty array.
   *
   * @throws Exception
   *   When Zenodo Deposition was not found.
   */
  public static function update(array $data) {
    $data['payload'] = $data['payload'] ?? [];
    static::validateRequired(['deposit_id', 'drupal_file'], $data);

    if (function_exists('drupal_alter')) {
      // hook_zenodo_file_update_alter().
      drupal_alter(static::getHookName(__FUNCTION__), $data);
    }

    // Get Deposition.
    if (variable_get('tpps_zenodo_host_greedy_search', FALSE)) {
      ZenodoHost::search(['deposit_id' => $data['deposit_id']]);
    }
    try {
      $deposition = ZenodoDeposit::retrieve(['deposit_id' => $data['deposit_id']]);
    }
    catch (Exception $e) {
      // Can't get deposition.
      throw new Exception(t('Deposition "@deposit_id" not found.',
        ['@deposit_id' => $deposit_id]
      ));
    }

    // Get appropriate Zenodo File Id (if any).
    foreach ($deposition['files'] as $zenodo_file) {
      if (
        ZenodoFile::getChecksum(['drupal_file' => $data['drupal_file']])
        == ZenodoFile::getChecksum(['zenodo_file' => $zenodo_file])
      ) {
        $data['file_id'] = $zenodo_file['id'];
      }
    }
    // Remove existing Zenodo File (if any).
    if (!empty($data['file_id'])) {
      static::delete($data);
    }
    unset($data['file_id']);
    // Create new Zenodo File using Drupal File.
    static::create($data);

    if (function_exists('module_invoke_all')) {
      // hook_zenodo_file_updated().
      module_invoke_all(static::getHookName('updated'), $data);
    }
    return $data['response'] ?? NULL;
  }

  /**
   * Gets file's checksum.
   *
   * Note: To avoid extra Zenodo API request you could provide 'zenodo_deposit'
   * or 'zenodo_file' if Zenodo Deposit/File was previously loaded.
   *
   * @param array $context
   *   Required keys are:
   *   Drupal: 'fid' or 'zenodo_fid'.
   *   Data to get file's checksum. Keys are:
   *   Drupal File:
   *     'fid' - int. Drupal File Id.
   *     'drupal_file' - object. Fully loaded Drupal File Object.
   *   Zenodo File:
   *     'zenodo_deposit' - array. Zenodo Deposition.
   *       Note 'zenodo_fid' is required.
   *     'zenodo_file' - array. Zenodo File.
   *       Note: 'zenodo_fid' is optional.
   *     'zenodo_fid' - string. Zenodo File Id.
   *       Is required when 'zenodo_deposit' is not empty.
   *       If 'zenodo_deposit' is empty then 'deposit_id' must be set to get
   *       Zenodo Deposition using API.
   *
   * @return mixed
   *   Returns NULL in case of errors and checksum in case of success.
   */
  public static function getChecksum(array $context) {
    // Drupal File Id.
    if (!empty($context['fid'])) {
      $context['drupal_file'] = tpps_file_load($context['fid']);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Drupal File Object.
    if (!empty($context['drupal_file'])) {
      if (
        !is_object($context['drupal_file'])
        // Fully loaded Drupal File object.
        || empty($context['drupal_file']->fid)
        // Real path exists.
        || empty($real_path = tpps_realpath($context['drupal_file']->uri))
      ) {
        return NULL;
      }
      return md5(file_get_contents($real_path));
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Zenodo File Id.
    else {
      if (
        empty($context['zenodo_file'])
        && (
          // No Zenodo Deposit.
          (
            empty($context['zenodo_deposit'])
            && empty($context['deposit_id'])
          )
          || (
            // Empty Zenodo File Id but Deposit exists.
            empty($context['zenodo_fid'])
            && !(
              empty($context['deposit_id'])
              && empty($context['zenodo_deposit'])
            )
          )
        )
      ) {
        $message = t("Can't get Zenodo File checksum because 'zenodo_deposit', "
          . "'zenodo_file' and 'deposit_id' are empty.");
        throw new Exception($message);
      }
      // Case 1. Zenodo File - fastest case.
      if (
        !empty($context['zenodo_file'])
        // Allow to use greedy search when provided other data.
        && !empty($context['zenodo_file'][static::CHECKSUM_KEY])
      ) {
        return $context['zenodo_file'][static::CHECKSUM_KEY];
      }
      // Case 2. Loaded Zenodo Deposit.
      if (
        !empty($context['zenodo_deposit'])
        && !empty($context['zenodo_deposit']['files'])
      ) {
        foreach ($context['zenodo_deposit']['files'] as $zenodo_file) {
          if ($zenodo_file['id'] == $context['zenodo_fid']) {
            return $zenodo_file[static::CHECKSUM_KEY];
          }
        }
      }
      // Case 3. Get Zenodo Deposit from Zenodo API.
      if (!empty($context['deposit_id'])) {
        $zenodo_file = ZenodoFile::retrieve(
          [
            // Note: We can't get Zenodo File without 'deposit_id'.
            'deposit_id' => $context['deposit_id'],
            'file_id' => $context['zenodo_fid'],
          ]
        );
        return $zenodo_file[static::CHECKSUM_KEY];
      }
      return NULL;
    }
  }

}
