<?php

/**
 * @file
 * Shows list of the NCBI Taxonomy Ids.
 */

/**
 * Menu callback. Shows list of the all organisms and their NCBI Taxonomy Ids.
 */
function tpps_report_organism_ncbi_all() {
  $report_name = 'organism_ncbi_all';
  $table = 'public.' . OrganismTaxonomy::TABLE;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $query = db_select('chado.organism', 't');
  // Taxonomy Id.
  $query->leftJoin($table, 'ot', 't.organism_id = ot.organism_id');
  $query->condition('ot.source', NcbiTaxonomy::KEY);
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
  $query->fields('ot', ['taxonomy_id']);

  $header = [
    ['data' => t('Organism Id'), 'field' => 't.organism_id'],
    ['data' => t('Genus'), 'field' => 't.genus'],
    ['data' => t('Species'), 'field' => 't.species'],
    ['data' => t('Infraspecific Rank'), 'field' => 'cvt.name'],
    ['data' => t('Infraspecific Name'), 'field' => 't.infraspecific_name'],
    ['data' => t('Taxonomy Id'), 'field' => 'ot.taxonomy_id'],
  ];

  return Report::print([
    'name' => $report_name,
    'header' => $header,
    'query' => $query,
    'pager' => 'both',
  ], $reset);
}
