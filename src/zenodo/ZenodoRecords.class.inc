<?php

/**
 * @file
 * Contains 'ZenodoFiles' class.
 */

/**
 * Zenodo API for Records.
 *
 * See: https://developers.zenodo.org/?shell#records.
 *
 * Actions are:
 *   list(array $data)
 *   retrieve(array $data)
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class ZenodoRecords extends Zenodo {

  // Prefix of the API URL.
  const BASE_PATH = '/api/records';

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // ZenodoRecords API doesn't have those commands so just disable them.

  /**
   * Creates a blank Zenodo DOI Deposit/Files.
   */
  public static function create() {
    throw new Exception('Called non-existing Zenodo Records action: "create".');
  }

  /**
   * Deletes deposition/files resource.
   */
  public static function delete() {
    throw new Exception('Called non-existing Zenodo Records action: "delete".');
  }

  /**
   * Updates an existing deposition/file.
   */
  public static function update() {
    throw new Exception('Called non-existing Zenodo Records action: "update".');
  }

}
