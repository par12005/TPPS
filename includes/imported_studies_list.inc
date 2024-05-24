<?php

/**
 * @file
 * Manages List of the Imported Studies.
 */

/**
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
 *
 * Usage:
 * module_load_include('inc', 'tpps', 'includes/imported_studies_list');
 */

/**
 * Gets whole list of imported studies and their source studies.
 *
 * @return array
 *   Returns list where keys are accessions for the new (imported) studies
 *   and values are source study accessions.
 */
function tpps_imported_studies_list_get() {
  return variable_get('tpps_imported_studies_list', []);
}

/**
 * Adds new item to the Submission Import List.
 *
 * @param string $local_accession
 *   Accession of study imported on local site. E.g., 'TGDR1234'.
 * @param string $remote_accession
 *   Accession of study exported on remote site. E.g., 'TGDR1234'.
 */
function tpps_imported_studies_list_add_item($local_accession, $remote_accession) {
  $import_list = variable_get('tpps_imported_studies_list', []);
  $import_list[$local_accession] = $remote_accession;
  variable_set('tpps_imported_studies_list', $import_list);
}

/**
 * Removed given new (imported) accession from Submission Import List.
 *
 * @param string $local_accession
 *   Accession of study imported on local site. E.g., 'TGDR1234'.
 */
function tpps_imported_studies_list_remove_item($local_accession) {
  $import_list = variable_get('tpps_imported_studies_list', []);
  unset($import_list[$local_accession]);
  variable_set('tpps_imported_studies_list', $import_list);
}
