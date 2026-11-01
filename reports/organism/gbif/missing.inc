<?php

/**
 * @file
 * Shows list of the organism which are missing GBIF Taxonomy Id.
 */

/**
 * Menu callback.
 */
function tpps_report_organism_gbif_missing() {
  module_load_include('inc', 'tpps', 'reports/organism/ncbi/missing');
  return tpps_report_organism_missing_helper(
    'organism_gbif_missing',
    GbifSpeciesApi::KEY
  );
}
