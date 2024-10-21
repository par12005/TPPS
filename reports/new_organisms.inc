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
function tpps_admin_new_organisms_report() {
  $name = 'new_organisms';
  $page_title = variable_get('tpps_report_new_organisms_title', 'New Organisms');

  tpps_add_css_js('main');

  $debug_mode = FALSE;
  $table = 'chado.organism';
  $query = db_select($table, 'o')->fields('o', ['organism_id']);
  if ($debug_mode) {
    $query->addTag('debug');
  }
  $query->leftJoin('tpps_ncbi_taxonomy', 'nt', 'o.organism_id = nt.organism_id');
  $query->isNull('nt.taxonomy_id');
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
      $item['organism_id'],
      tpps_entity_link($item['organism_id'], $item['organism_name'], 'Organism'),
      theme('study_accession', ['accession' => $item['accession']]),
      implode(' | ', $actions),
    ];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $links = [
    l(t('NCBI Taxonomy Id Batch Update page'), 'admin/config/tpps/ncbi_batch'),
  ];
  if (!is_live_site()) {
    $links[] = l(
      t('"tpps_ncbi_taxonomy" table'), 'admin/reports/db/tpps_ncbi_taxonomy'
    );
  }
  $header = [
    t('Organism Id'), t('Organism Name'), t('Study'), t('Useful Links')
  ];
  return implode(' | ', $links) . '<br />' . tpps_report_show_table([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
  ]);
}
