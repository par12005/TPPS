<?php

/**
 * @file
 * Shows list of the organism which are missing NCBI Taxonomy Id.
 */

/**
 * Menu callback.
 */
function tpps_report_organism_ncbi_missing() {
  return tpps_report_organism_missing_helper(
    'organism_ncbi_missing',
    NcbiTaxonomy::KEY
  );
}

/**
 * Helper to build 'organism_N_missing' report content.
 *
 * @param string $report_name
 *   Report name.
 * @param string $source
 *   Taxonomy data source. Possible values: 'ncbi' and 'gbif'.
 *
 * @return string
 *   Returns rendered report.
 */
function tpps_report_organism_missing_helper($report_name, $source) {

  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $query = db_select('chado.organism', 't');
  //Get taxonomy id.
  $query->leftJoin(
    'public.' . OrganismTaxonomy::TABLE,
    'ot',
    't.organism_id = ot.organism_id AND ot.source = :source',
    [':source' => $source]
  );
  // Infraspecific Rank.
  $query->leftJoin('chado.cvterm', 'cvt', 'cvt.cvterm_id = t.type_id');
  $query->fields('t', ['organism_id', 'genus', 'species']);
  // Infraspecific Rank.
  $query->fields('cvt', ['name']);
  $query->fields('t', ['infraspecific_name']);
  $query->isNull('ot.taxonomy_id');
  $header = [
    ['data' => '#', 'field' => '#'],
    ['data' => t('Organism Id'), 'field' => 't.organism_id', 'sort' => 'asc'],
    ['data' => t('Genus'), 'field' => 't.genus'],
    ['data' => t('Species'), 'field' => 't.species'],
    ['data' => t('Infraspecific Rank'), 'field' => 'cvt.name'],
    ['data' => t('Infraspecific Name'), 'field' => 't.infraspecific_name'],
  ];
  return Report::print([
    'name' => $report_name,
    'header' => $header,
    'query' => $query,
  ], $reset);
}
