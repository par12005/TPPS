<?php

/**
 * @file
 * Shows list of studies without DOI attached.
 */

// Id of the 'TreeGenes database' term.
// Used as 'db_id' in 'chado.dbxref' table.
define('TPPS_CVTERM_TREEGENES_DATABASE', 95);

/**
 * Menu callback. Shows studies without DOI attached.
 */
function tpps_report_organism_new() {
  $page_title = variable_get('tpps_report_organism_new_title', 'New Organisms');

  tpps_add_css_js('main');

  $debug_mode = FALSE;
  $table = 'chado.organism';
  $query = db_select($table, 'o')->fields('o', ['organism_id']);
  if ($debug_mode) {
    $query->addTag('debug');
  }
  $query->leftJoin(NcbiTaxonomy::TABLE, 'nt', 'o.organism_id = nt.organism_id');
  $query->isNull('nt.taxonomy_id');
  $query->leftJoin(GbifSpeciesApi::TABLE, 'gsa', 'o.organism_id = gsa.organism_id');
  $query->isNull('gsa.taxonomy_id');
  // Get project id.
  $query->join('chado.project_organism', 'cpo', 'cpo.organism_id = o.organism_id');
  // Get dbxref_if for projejct_id.
  $query->join('chado.project_dbxref', 'cpd', 'cpd.project_id = cpo.project_id');
  // Get study accession number.
  $query->join('chado.dbxref', 'cd', 'cpd.dbxref_id = cd.dbxref_id');

  $query->condition('cd.db_id', TPPS_CVTERM_TREEGENES_DATABASE);
  $query->addField('cd', 'accession');
  // Get full organims name.
  $query->addExpression("CONCAT(o.genus, ' ', o.species)", 'organism_name');
  $query->orderBy('organism_id');
  if ($debug_mode) {
    $query->range(0, 5);
  }
  $result = $query->execute()->fetchAllAssoc('organism_id', PDO::FETCH_ASSOC);

  $rows = [];
  foreach ($result as $item) {
    // @TODO Convert to theme-function.
    $actions = [
      l(t('Admin Panel'), 'tpps-admin-panel/' . $item['accession']),
      l(t('Edit'), 'tppsc/' . $item['accession']),
      l(t('Details'), 'tpps/details/' . $item['accession']),
      l(t('Files'), 'tpps-admin-panel/file-diagnostics/' . $item['accession']),
      l(t('View JSON'), 'api/submission/' . $item['accession'] . '/view/state'),
    ];
    $rows[] = [
      'organism_id' => $item['organism_id'],
      'organism_name' => tpps_entity_link(
        $item['organism_id'],
        $item['organism_name'],
        'Organism'
      ),
      'study' => theme('study_accession', ['accession' => $item['accession']]),
      'links' => implode(' | ', $actions),
    ];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $links = [
    l(t('NCBI Taxonomy Id Batch Update page'), 'admin/config/tpps/ncbi_batch'),
    l(t('GBIF Taxonomy Id Batch Update page'), 'admin/config/tpps/gbif_batch'),
  ];
  if (!is_live_site()) {
    $links[] = l(
      t('"@table_name" table', ['@table_name' => NcbiTaxonomy::TABLE]),
      'admin/reports/db/' . NcbiTaxonomy::TABLE
    );
    $links[] = l(
      t('"@table_name" table', ['@table_name' => GbifSpeciesApi::TABLE]),
      'admin/reports/db/' . GbifSpeciesApi::TABLE
    );
  }
  $header = [
    ['data' => t('Organism Id'), 'field' => 'organism_id'],
    ['data' => t('Organism Name'), 'field' => 'organism_name'],
    ['data' => t('Study'), 'field' => 'study'],
    ['data' => t('Useful Links'), 'field' => 'links'],
  ];
  $content = Report::print([
    'name' => 'organism_new',
    'header' => $header,
    'rows' => $rows,
  ]);
  return implode(' | ', $links) . '<br />' . $content;

}
