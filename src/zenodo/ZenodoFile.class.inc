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
    $md5 = md5(file_get_contents(tpps_realpath($data['drupal_file']->uri)));

    // Get appropriate Zenodo File Id (if any).
    foreach ($deposition['files'] as $zenodo_file) {
      if ($zenodo_file['checksum'] == $md5) {
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

}
