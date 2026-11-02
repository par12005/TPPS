<?php

/**
 * @file
 * Shows list of the organism which are missing GBIF Taxonomy Id.
 */

/**
 * Menu callback.
 */
function tpps_report_organism_gbif_missing() {
  $report_name = 'organism_gbif_missing';
  $source = GbifSpeciesApi::KEY;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
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
  $query->addField('t', 'organism_id', 'counter');
  // Add the row number expression
  $query->addExpression('ROW_NUMBER() OVER ()', 'counter');

  $query->fields('t', [
    'organism_id',
    'genus',
    'species',
  ]);
  // Infraspecific Rank.
  $query->fields('cvt', ['name']);
  $query->fields('t', ['infraspecific_name']);
  $query->isNull('ot.taxonomy_id');

  $header = [
    ['data' => '#', 'field' => 'counter'],
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
