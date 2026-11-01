<?php

/**
 * @file
 * Gets taxonomy id for existing organisms from GBIF using Batch API.
 *
 * Admnin theme was enabled for this page in tpps_admin_paths().
 */

module_load_include('inc', 'tpps', 'batch/ncbi/taxonomy_id');

// Name of the update button.
// Only organisms without taxonomy ids will be processed.
define('TPPS_GBIF_BATCH_UPDATE_BUTTON_NAME', 'gbif_batch_update');

// Name of the button 'Recreate'.
// All existing taxonomy ids will be removed, list of the accepted synonym
// cleared and taxonomy ids requested again.
define('TPPS_GBIF_BATCH_RECREATE_BUTTON_NAME', 'gbif_batch_recreate');

/**
 * Menu callback for 'batch/gbif/taxonomy_id'.
 *
 * Form 'tpps_batch_gbif_taxonomy_id_form'.
 */
function tpps_batch_gbif_taxonomy_id_form(array $form, array &$form_state) {
  $source = GbifSpeciesApi::KEY;
  return tpps_batch_common_taxonomy_id_form($source, $form, $form_state);
}

/**
 * Form submitter for 'tpps_batch_gbif_taxonomy_id_form'.
 */
function tpps_batch_gbif_taxonomy_id_form_submit(array $form, array &$form_state) {
  $source = GbifSpeciesApi::KEY;
  tpps_batch_common_taxonomy_id_form_submit($source, $form, $form_state);
}

/**
 * Batch callback. Updates GBIF Taxonomy Id for a single organism.
 *
 * @param int $organism_id
 *   Orgaanism Id. See chado.orgnism.organism_id.
 * @param int $total
 *   Total amount of organisms to be processed.
 * @param array $context
 *   Batch context.
 */
function tpps_batch_gbif_taxonomy_id_callback($organism_id, $total, array &$context) {
  tpps_batch_common_taxonomy_id_callback(
    GbifSpeciesApi::KEY,
    $organism_id,
    $total,
    $context
  );
}

/**
 * Batch callback. Shows summary of batch process or error message.
 *
 * @param bool $success
 *   TRUE when batch completed with success.
 * @param array $results
 *   List of output of each callback call.
 */
function tpps_batch_gbif_taxonomy_id_finished_callback($success, array $results) {
  $source = GbifSpeciesApi::KEY;
  tpps_batch_common_taxonomy_id_finished_callback($source, $success, $results);
}
