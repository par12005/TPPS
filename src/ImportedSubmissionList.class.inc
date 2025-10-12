<?php

/**
 * @file
 * Contains 'ImportedSubmissionList' class.
 *
 * Overview.
 *
 * This list is stored in Drupal's variables instead of DB table because:
 * 1. It should be used only in dev environment.
 * 2. Imported Studies must be used only for debugging and testing.
 * 3. This list must be small.
 *
 * This list is used to build report where listed all imported studies and
 * their related source studies to make manage of imported studies easier.
 *
 * Terms:
 * Local Study - imported study on local site.
 * Remote Study - exported on remote site study.
 */

/**
 * Manages List of the Imported Submissions.
 *
 * @package tpps
 * @author Vlad Savistky <vlad.savitsky@gmail.com>
 */
class ImportedSubmissionList {

  // Name of variable which stores list of imported studies.
  // @todo Find better ways to store this list.
  const NAME = 'tpps_imported_studies_list';

  /**
   * Gets list of all imported submissions and their source studies.
   *
   * @return array
   *   Returns list where keys are accessions for the new (imported) studies
   *   and values are source study accessions.
   */
  public static function get() {
    return variable_get(static::NAME, []);
  }

  /**
   * Adds new item to the Submission Import List.
   *
   * @param string $local_accession
   *   Accession of study imported on local site. E.g., 'TGDR1234'.
   * @param string $remote_accession
   *   Accession of study exported on remote site. E.g., 'TGDR1234'.
   */
  public static function add($local_accession, $remote_accession) {
    $import_list = variable_get(static::NAME, []);
    $import_list[$local_accession] = $remote_accession;
    variable_set(static::NAME, $import_list);
  }

  /**
   * Removes given new (imported) accession from Submission Import List.
   *
   * @param string $local_accession
   *   Accession of study imported on local site. E.g., 'TGDR1234'.
   */
  public static function remove($local_accession) {
    $import_list = variable_get(static::NAME, []);
    unset($import_list[$local_accession]);
    variable_set(static::NAME, $import_list);
  }

}
