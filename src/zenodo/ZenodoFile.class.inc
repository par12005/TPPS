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

}
