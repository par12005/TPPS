<?php

/**
 * @file
 * Shows list of the organism which are missing NCBI Taxonomy Id.
 */

/**
 * Menu callback.
 */
function tpps_report_organism_ncbi_missing() {
  $report_name = 'organism_ncbi_missing';
  $table = 'public.' . NcbiTaxonomy::TABLE;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $query = db_select('chado.organism', 't');
  // Taxonomy Id.
  $query->leftJoin($table, 't2', 't.organism_id = t2.organism_id');
  // Infraspecific Rank.
  $query->leftJoin(
    'chado.cvterm',
    'cvt',
    'cvt.cvterm_id = t.type_id AND cvt.cv_id = :cv_id',
    [':cv_id' => InfraspecificRank::CV_ID]
  );
  $query->fields('t', [
    'organism_id',
    'genus',
    'species',
  ]);
  // Infraspecific Rank.
  $query->fields('cvt', ['name']);
  $query->fields('t', ['infraspecific_name']);
  $query->isNull('t2.taxonomy_id');

  $header = [
    ['data' => t('Organism Id'), 'field' => 't.organism_id'],
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
