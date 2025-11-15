<?php

/**
 * @file
 * Study Details page AJAX-callbacks.
 *
 * See /tpps.ajax.inc.
 *
 * To debug
 * $accession = 'TGDR1033';
 * module_load_include('inc', 'tpps', 'ajax_callbacks/study_details');
 * $content = tpps_ajax_get_phenotype_details_tab(['accession' => $accession, 'page' => 0]);
 * dpm($content);
 */

/**
 * Gets content of the 'Species' tab of Study Details page.
 *
 * @param array $data
 *   POST data.
 *   Required keys are:
 *   'accession' - string.
 *     Required. Study accession. E.g., 'TGDR1090'.
 *   'page' - int. Optional. TPPS Form page number.
 *
 * @return string
 *   The HTML table of species associated with the submission.
 */
function tpps_ajax_get_species_details_tab(array $data) {
  if (!tpps_access('access tpps details')) {
    return ['errors' => [t('Access denied')]];
  }
  if (empty($data['accession'])) {
    return ['errors' => [t('Empty study accession.')]];
  }
  $submission = new Submission($data['accession']);

  $query = db_select('chado.organism', 'o');
  $query->join('chado.pub_organism', 'po', 'o.organism_id = po.organism_id');
  $query->join('chado.project_pub', 'pp', 'pp.pub_id = po.pub_id');
  $query->fields('o', ['organism_id', 'genus', 'species', 'common_name']);
  $query->condition('o.genus', '', 'NOT LIKE');
  $query->condition('pp.project_id', $submission->sharedState['ids']['project_id']);
  $query = $query->execute();
  $rows = [];

  while (($result = $query->fetchObject())) {
    $id = $result->organism_id;
    $common_name = $result->common_name;
    $family = $order = "";
    $family_exists = tpps_chado_prop_exists('organism', $id, 'family');
    $order_exists = tpps_chado_prop_exists('organism', $id, 'order');
    if (empty($family_exists) || empty($order_exists)) {
      if (variable_get('tpps_ncbi_allow_fix_missing_data', FALSE)) {
        $organism = new Organism($id);
        if ($organism->isLoaded()) {
          $organism->processTaxonomyId($submission);
          $message = t('Updated NCBI Taxonomy data for organism: @name (@id).',
            ['@id' => $organism->id, '@name' => $organism->name]
          );
          drupal_set_message($message);
        }
      }
    }

    if ($family_exists) {
      $family = db_select('chado.organismprop', 'o')
        ->fields('o', ['value'])
        ->condition('organism_id', $id)
        ->condition('type_id', tpps_load_cvterm('family')->cvterm_id)
        ->execute()->fetchObject()->value;
    }
    if ($order_exists) {
      $order = db_select('chado.organismprop', 'o')
        ->fields('o', ['value'])
        ->condition('organism_id', $id)
        ->condition('type_id', tpps_load_cvterm('order')->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    if (empty($common_name) and tpps_chado_prop_exists('organism', $id, 'common name')) {
      $common_name = db_select('chado.organismprop', 'o')
        ->fields('o', ['value'])
        ->condition('organism_id', $id)
        ->condition('type_id', tpps_load_cvterm('common name')->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    $name = "{$result->genus} {$result->species}";
    $link = tpps_entity_link($id, $name, 'Organism');
    $rows[$name] = [
      $order,
      $family,
      $link,
      $common_name,
    ];
  }

  $vars = [
    'header' => [t('Order'), t('Family'), t('Species'), t('Common Name')],
    'rows' => $rows,
    'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => t('No data'),
  ];
  return [
    'success' => TRUE,
    'content' => theme('table', $vars),
  ];
}

/**
 * Gets content of the 'Study' tab of Study Details page.
 *
 * @param array $data
 *   POST data.
 *   Required keys are:
 *   'accession' - string.
 *     Required. Study accession. E.g., 'TGDR1090'.
 *   'page' - int. Optional. TPPS Form page number.
 *
 * @return string
 *   The HTML table of study information of the submission.
 */
function tpps_ajax_get_study_details_tab(array $data) {
  if (!tpps_access('access tpps details')) {
    return ['errors' => [t('Access denied.')]];
  }
  if (empty($data['accession'])) {
    return ['errors' => [t('Empty study accession.')]];
  }
  // @TODO Cache $rows. Processing takes 53 msec >> 2msec.
  $submission = new Submission($data['accession']);
  $state = $submission->sharedState;
  $rows = [];
  $species = [];
  $files = [];

  for ($i = 1; $i <= $submission->getOrganismNumber(); $i++) {
    if (!empty($id = $state['ids']['organism_ids'][$i])) {
      $species[] = tpps_entity_link($id, $submission->getOrganismName($i), 'Organism');
    }
    // Add "Local VCF" (which hasn't file id).
    $files[] = $submission->getLocalVcf($i);
  }
  // Note: Remove empty elements after processing local VCF files.
  $files = array_filter($files);
  $params = ['attributes' => ['target' => '_blank']];
  foreach (array_keys($submission->getFileIdList()) as $fid) {
    if ($file = tpps_file_load($fid)) {
      $file_url = check_plain(file_create_url($file->uri));
      if (
        stripos($file_url, '/isg/') === FALSE
        || stripos($file_url, 'core/labs') === FALSE
      ) {
        $params['attributes']['title'] = 'File id: ' . $fid
          . ', Size: ' . $file->filesize . ' bytes';
        $files[] = l(check_plain($file->filename), $file_url, $params);
      }
    }
  }
  $page1_values = $state['saved_values'][TPPS_PAGE_1] ?? NULL;
  $page2_values = $state['saved_values'][TPPS_PAGE_2] ?? NULL;
  $pub_link = $submission->sharedState['saved_values'][TPPS_PAGE_1]['publication']['title'];
  $pub_id = db_select('chado.project_pub', 'p')
    ->fields('p', ['pub_id'])
    ->condition('project_id', $state['ids']['project_id'])
    ->execute()->fetchObject()->pub_id;
  if (!empty($pub_id)) {
    try {
      $bundle = tripal_load_bundle_entity(['label' => 'Publication']);
      $pub_id = chado_get_record_entity_by_bundle($bundle, $pub_id);
      $pub_link = l($pub_link, 'Publication/' . $pub_id);
    }
    catch (Exception $e) {
      return ['errors' => [$e->getMessage()]];
    }
  }
  // Build list of authors.
  $author_list[] = check_plain($page1_values['primaryAuthor']);
  $secondary_authors_number = $page1_values['publication']['secondaryAuthors']['number'];
  for ($i = 1; $i <= $secondary_authors_number; $i++) {
    $author_list[] = check_plain($page1_values['publication']['secondaryAuthors'][$i]);
  }
  // Build list of species.
  $species_list = [];
  for ($i = 1; $i <= $submission->getOrganismNumber(); $i++) {
    $species_list[] = $i . ': ' . check_plain($page1_values['organism'][$i]['name']);
  }
  $rows[] = [t('Accession'), $state['accession']];
  $rows[] = [t('Title'), $pub_link];
  $rows[] = [t('Authors'), theme('item_list', ['items' => $author_list])];
  $rows[] = [t('Species'), theme('item_list', ['items' => $species_list])];
  $rows[] = [t('Data Type'), $page2_values['data_type']];
  module_load_include('inc', 'tpps', 'includes/form');
  $rows[] = [
    'Study Type',
    tpps_form_get_study_type($page2_values['study_type']),
  ];
  if (!empty($files)) {
    unset($params['attributes']['title']);
    $rows[] = [t('File Downloads'),
      theme('item_list', ['items' => $files])
        // Add ZIP-file download link to the bottom of the list.
        . l(t('Download compressed raw files'),
        'tpps/api/files/' . $submission->accession, $params
      )
    ];
  }
  $rows[] = [t('Population Size'), $state['stats']['tree_count']];
  if (!empty($state['stats']['phenotype_count'])) {
    $rows[] = [t('Phenotype Count'), $state['stats']['phenotype_count']];
  }
  if (!empty($state['stats']['unique_phenotypes'])) {
    $rows[] = [t('Unique Phenotypes'), $state['stats']['unique_phenotypes']];
  }
  // @todo genotype counts.
  // $rows[] = array('Genotype Count', $state['stats']['genotype_count']);
  $vars = [
    'header' => [],
    'rows' => $rows,
    'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  ];
  return [
    'success' => TRUE,
    'content' => theme('table', $vars),
  ];
}

/**
 * Gets content of the 'Environmental' tab of Study Details page.
 *
 * @param array $data
 *   POST data.
 *   Required keys are:
 *   'accession' - string.
 *     Required. Study accession. E.g., 'TGDR1090'.
 *   'page' - int. Optional. TPPS Form page number.
 *
 * @return string
 *   The HTML table of study information of the submission.
 */
function tpps_ajax_get_environmental_details_tab(array $data) {
  if (!tpps_access('access tpps details')) {
    return ['errors' => [t('Access denied.')]];
  }
  if (empty($data['accession'])) {
    return ['errors' => [t('Empty study accession.')]];
  }
  // @TODO Cache $rows. Processing takes 53 msec >> 2msec.
  $submission = new Submission($data['accession']);
  $state = $submission->sharedState;
  $rows = [];
  $species = [];
  $files = [];

  for ($i = 1; $i <= $submission->getOrganismNumber(); $i++) {
    $organism_key = 'organism-' . $i;
    if (!empty($layers = $state['saved_values'][TPPS_PAGE_4][$organism_key]['environment']['env_layers'])) {
      foreach ($layers as $layer_name => $layer) {
        $rows[] = ['Environmental Layer', $layer_name];
      }
    }

  }


  module_load_include('inc', 'tpps', 'includes/form');
  // $rows[] = [
  //   'Study Type',
  //   tpps_form_get_study_type($page2_values['study_type']),
  // ];
  // $rows[] = [t('File Downloads'), implode('<br>', $files)];
  // $rows[] = [t('Population Size'), $state['stats']['tree_count']];

  // @todo genotype counts.
  // $rows[] = array('Genotype Count', $state['stats']['genotype_count']);
  $vars = [
    'header' => [],
    'rows' => $rows,
    'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  ];
  return [
    'success' => TRUE,
    'content' => theme('table', $vars),
  ];
}

/**
 * Gets content of the 'Trees' tab of Study Details page.
 *
 * Creates the table of individual plant details for a submission.
 *
 * @param array $data
 *   POST data.
 *   Required keys are:
 *   'accession' - string.
 *     Required. Study accession. E.g., 'TGDR1090'.
 *   'page' - int. Optional. TPPS Form page number.
 *
 * @return string
 *   The HTML table of plants associated with the submission.
 */
function tpps_ajax_get_trees_details_tab(array $data) {
  if (!tpps_access('access tpps details')) {
    return ['errors' => [t('Access denied')]];
  }
  if (empty($data['accession'])) {
    return ['errors' => [t('Empty study accession.')]];
  }
  $submission = new Submission($data['accession']);
  $path = current_path();
  $args = explode('/', $path);
  $page = $data['page'] ?? 0;
  $rows = [];
  $per_page = 20;

  if (empty($submission->sharedState['tree_info'])) {
    return [
      'errors' => [
        t('The plant information for this study has not been properly updated. '
        . 'Please contact your site administrator.'),
      ],
    ];
  }

  $_GET['page'] = $page;
  pager_default_initialize(count($submission->sharedState['tree_info']), $per_page);
  $chunks = array_chunk($submission->sharedState['tree_info'], $per_page, TRUE);

  foreach ($chunks[$page] as $name => $info) {
    $id = $info['organism_id'];
    $form_number = array_search($id, $submission->sharedState['ids']['organism_ids']);
    $tree_id = tpps_entity_link($info['stock_id'], $name, 'Stock');
    $row = [
      // strtoupper($args[2]) . '-' . strip_tags($tree_id), // RISH: the $args[2] shows the ajax page part of the url 'get_trees_details_tab' which is not what we want
      strtoupper($data['accession'] . '-' . strip_tags($tree_id)),
      tpps_entity_link($info['stock_id'], $name, 'Stock'),
      tpps_entity_link($id, $submission->getOrganismName($form_number), 'Organism'),
    ];
    $row[] = $info['lat'];
    $row[] = $info['lng'];
    $rows[] = $row;
  }
  $header = [
    t('TreeGenes Identifier'),
    t('Identifier'),
    t('Species'),
    t('Latitude'),
    t('Longitude'),
  ];
  $pager = theme('pager',
    ['quantity', count($submission->sharedState['tree_info'])]
  );
  $table = theme('table', [
    'header' => $header,
    'rows' => $rows,
    'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  ]);
  return [
    'success' => TRUE,
    'content' => $pager . $table,
  ];
}

/**
 * Gets content of the 'Phenotype' tab of Study Details page.
 *
 * @param array $data
 *   POST data.
 *   Required keys are:
 *   'accession' - string.
 *     Required. Study accession. E.g., 'TGDR1090'.
 *   'page' - int. Optional. TPPS Form page number.
 *
 * @return string
 *   The HTML table of plants and phenotypes associated with the submission.
 *
 * @todo Send $organism_index from browser to get correct data.
 */
function tpps_ajax_get_phenotype_details_tab(array $data) {
  if (!tpps_access('access tpps details')) {
    return ['errors' => [t('Access denied')]];
  }
  if (empty($data['accession'])) {
    return ['errors' => [t('Empty study accession.')]];
  }
  $submission = new Submission($data['accession']);
  $page = $data['page'] ?? 0;

  $rows = [];
  $per_page = 20;
  $start = $page * $per_page;

  // Get total amount.
  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->addExpression('count(p.phenotype_id)', 'count');
  $query->condition('ps.project_id', $submission->getProjectId());
  $query = $query->execute();
  $total = $query->fetchObject()->count;

  // Init pager.
  $_GET['page'] = $page;
  pager_default_initialize($total, $per_page);

  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');

  // Get optional 'Year' column only when 'year exists in data file.
  //
  // WARNING: 1st phenotype data file could have no 'Year' column but others
  // do have and this will not work in this case.
  //
  if (PhenotypeData::hasColumn(
    $organism_index = 1,
    $submission->state,
    PhenotypeData::DATA_TYPE_YEAR
  )) {
    $query->join('chado.phenotypeprop', 'pp', 'p.phenotype_id = pp.phenotype_id');
    $query->condition('pp.type_id', PhenotypeData::getYearCvTermId());
    $query->addField('pp', 'value', 'year');
  }

  $query->fields('p', [
    'phenotype_id',
    'name',
    'attr_id',
    'observable_id',
    'value',
  ]);
  $query->fields('s', ['uniquename', 'stock_id']);
  $query->condition('ps.project_id', $submission->getProjectId());
  $query->range($start, $per_page);

  // $query = db_select('chado.new_pheno_view_update', 'p');
  // $query->addField('p', 'phenotype_id', 'phenotype_id');
  // $query->addField('p', 'synonym', 'name');
  // $query->addField('p', 'structure_id', 'attr_id');
  // $query->addField('p', 'observable_id', 'observable_id');
  // $query->addField('p', 'value', 'value');
  // $query->addField('p', 'tree_acc', 'uniquename');
  // $query->addField('p', 'stock_id', 'stock_id');
  // $query->condition('ps.project_id', $submission->getProjectId());
  // $query->range($start, $per_page);
  // $query = $query->execute();

  return _tpps_details_tab_build_table($query, $total, $submission);
}

/**
 * Gets content of the 'Genotype' tab of Study Details page.
 *
 * @param array $data
 *   POST data.
 *   Required keys are:
 *   'accession' - string.
 *     Required. Study accession. E.g., 'TGDR1090'.
 *   'page' - int. Optional. TPPS Form page number.
 *
 * @return string
 *   The HTML table of plants and genotypes associated with the submission.
 */
function tpps_ajax_get_genotype_details_tab(array $data) {
  if (!tpps_access('access tpps details')) {
    return ['errors' => [t('Access denied')]];
  }
  if (empty($data['accession'])) {
    return ['errors' => [t('Empty study accession.')]];
  }
  $submission = new Submission($data['accession']);
  if (empty($project_id = $submission->getProjectId())) {
    return ['errors' => [t("Study wasn't yet processed.")]];
  }
  $page = $data['page'] ?? 0;
  $rows = array();
  $per_page = 5;
  $start = $page * $per_page;

  $_GET['page'] = $page;


  // // Set total check = false to begin with
  // $total_check = false;

  // // Check cache to see if a total is available
  // $total_array = variable_get('tpps_tab_genotype_' + $project_id + '_total');
  // if($total_array == NULL) {
  //   $total_check = true;
  // }
  // else {
  //   // total array should have an expire_time
  //   if($total_array['expire_time'] < time()) {
  //     $total_check = true;
  //   }
  //   else {
  //     $total = $total_array['total'];
  //   }

  // }

  //

  // $dev = FALSE;
  // if($dev == TRUE) {
  //   $total_check = true;
  // }

  // if($total_check) {
  //   $query = db_select('chado.genotype', 'g');
  //   $query->join('chado.stock_genotype', 'sg', 'sg.genotype_id = g.genotype_id');
  //   $query->join('chado.project_stock', 'ps', 'ps.stock_id = sg.stock_id');
  //   $query->addExpression('count(g.genotype_id)', 'count');
  //   $query->condition('ps.project_id', $project_id;
  //   $query = $query->execute();
  //   $total = $query->fetchObject()->count;

  //   $total_array = array(
  //     'expire_time' => time() + variable_get('tpps_tab_genotype_total_cache_time', 7*(24*60*60)),
  //     'total' => $total
  //   );
  //   variable_set('tpps_tab_genotype_' + $project_id + '_total', $total_array);
  // }

  // VERSION 2 - RISH - better but far from perfect - can get as slow a 1 minute per query
  // $query_results = chado_query('SELECT COUNT(*) as c1 FROM chado.genotypes_' . $project_id);





  // Removed by Rish in favour of materialized views per project_id / study 3/24/2023
  // $query = db_select('chado.genotype', 'g');
  // $query->join('chado.stock_genotype', 'sg', 'sg.genotype_id = g.genotype_id');
  // $query->join('chado.project_stock', 'ps', 'ps.stock_id = sg.stock_id');
  // $query->join('chado.stock', 's', 's.stock_id = sg.stock_id');
  // $query->fields('g', array(
  //   'genotype_id',
  //   'name',
  //   'uniquename',
  //   'description',
  //   'type_id',
  // ));
  // $query->fields('s', array('uniquename', 'stock_id'));
  // $query->condition('ps.project_id', $project_id);
  // $query->range($start, $per_page);
  // $query = $query->execute();

  // VERSION 2 - RISH - Faster but still linearly slower
  // $query = chado_query("SELECT * FROM chado.genotypes_" . $project_id . " OFFSET $start LIMIT $per_page");

  $view_version = 3;


  // Check if VCF exists (this is a dependency for version 4)
  // Get all VCF file locations (based on per organism from the sharedState)
  require_once(dirname(__FILE__) . '/../forms/submit/submit_all.php');
  $shared_state = $submission->getSharedState();
  $page1_values = $submission->sharedState['saved_values'][TPPS_PAGE_1];
  $organism_number = $page1_values['organism']['number'];
  $vcf_location_details = [];
  for ($i = 1; $i <= $organism_number; $i++) {
    $vcf_details = tpps_vcf_location($shared_state, $i);
    // print_r($vcf_details);
    if ($vcf_details['status'] == 'exists') {
      $view_version = 4;
    }
    $vcf_location_details[] = $vcf_details;
  }

  if ($view_version == 3) {
    // @NEW -RISH using Emily's redesign that uses index_id. Fast response in ms.
    $query_results = chado_query('select max(index_id) as c1 FROM chado.genotypes_' . $project_id, []);

    $total = 0;
    foreach ($query_results as $row) {
      $total = $row->c1;
    }
    pager_default_initialize($total, $per_page);
    // VERSION 3 - RISH using Emily's index_id to paginate the page columns.
    // @TODO Urgent. Update unsecure db query.
    $query = chado_query("select * from chado.genotypes_" . $project_id .
      " where index_id > " . $start .
      " order by index_id asc
      limit " . $per_page . ";", []);

    $cvterm_list = [];
    while (($result = $query->fetchObject())) {
      // preg_match('/^TGDR...-(.*)$/', $result->s_uniquename, $matches);
      preg_match('/^TGDR.{1,4}-(.*)$/', $result->s_uniquename, $matches);
      $tree = $matches[1];
      if (empty($cvterm_list[$result->name])) {
        $cvterm_list[$result->name] = [
          'type' => chado_get_cvterm(['cvterm_id' => $result->type_id])->name ?? '',
        ];
      }
      $rows[] = [
        tpps_entity_link($result->stock_id, $tree, 'Stock'),
        // tpps_entity_link($result->genotype_id, $result->uniquename, 'Genotype'),
        $result->name,
        $result->description,
        $cvterm_list[$result->name]['type'],
      ];
    }
    $table = theme('table', [
      'header' => [
        t('Plant'),
        t('Name'),
        t('Description'),
        t('Marker Type'),
      ],
      'rows' => $rows,
      'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
      'caption' => '',
      'colgroups' => NULL,
      'sticky' => FALSE,
      'empty' => '',
    ]);
    $pager = theme('pager', ['quantity', $total]);
    return [
      'success' => TRUE,
      'content' => $pager . $table,
      'dir' => dirname(__FILE__),
    ];
  }
  else if ($view_version == 4) {
    // VERSION 4 - RISH + MEGHAN'S SH SCRIPT (3/30/2025)

    $rows = [];
    // $vcf_location = "/core/labs/Wegrzyn/meghan_work/query_VCF_for_genotypes/Geraldes_2013.v2.vcf";
    $vcf_location = $vcf_location_details[0]['location'];
    $output_lines = [];

    // Get total reads
    $cmd_plants_count = 'bcftools query -l "' . $vcf_location . '" | wc -l';
    $plants_count = exec($cmd_plants_count);
    // $cmd_markers_count = 'bcftools view -H "' . $vcf_location . '" | wc -l';
    $cmd_markers_count = "bcftools index --stats \"$vcf_location\" | awk '{sum += $3} END {print sum}'";
    $markers_count = exec($cmd_markers_count);
    $total = ceil(($plants_count * $markers_count) / 20);
    pager_default_initialize($total, $per_page);


    // Get the genotype calls from the VCF file for the specified page
    // $cmd_calls = "/bin/bash " . dirname(__FILE__) . '/../tools/vcfscripts/query_VCF_for_genotype_calls.sh -p ' . ($page+1) . ' -f \'' . $vcf_location . '\'';
    $time = microtime(true) * 1000; // Get current time in milliseconds
    $output_location = '/isg/treegenes/study_markers_pages_tmp/markers_' . $data['accession'] . '_' . ($page+1) . '_' . $time . '.txt';
    $cmd_calls = "/bin/bash " . dirname(__FILE__) . '/../tools/vcfscripts/query_vcf_for_markers.sh -p ' . ($page+1) . ' -f \'' . $vcf_location . '\' > ' . $output_location . ' &';
    // $start_line = ($page * 20) + 1;
    // $end_line = ($page + 1) * 20;
    // $cmd_calls = "#!/bin/bash
    //   bcftools query -f '[%SAMPLE\\t%ID\\t%REF\\t%ALT\\t%GT\\n]' \"$vcf_location\" | \\
    //   head -n $end_line | tail -n +$start_line | \\
    //   awk -F'\\t' '{
    //       split(\$4, alts, \",\")
    //       if (\$5 == \"./.\") {
    //           final_gt = \"./.\"
    //       } else {
    //           split(\$5, gt, \"/\")
    //           allele1 = (gt[1] == \".\") ? \".\" : (gt[1] == \"0\") ? \$3 : alts[int(gt[1])]
    //           allele2 = (gt[2] == \".\") ? \".\" : (gt[2] == \"0\") ? \$3 : alts[int(gt[2])]
    //           final_gt = allele1 \"/\" allele2
    //       }
    //       print \$1, \$2, final_gt
    //   }'
    // ";
    // exec($cmd_calls, $output_lines);
    // $output_string = shell_exec($cmd_calls);

    //$file = new SplFileObject('myfile.txt');
    //$file->seek(9999);     // Seek to line no. 10,000
    //echo $file->current(); // Print contents of that line
    // $vcf_content = gzopen($vcf_location, 'r');

    //$cmd_calls = "bcftools query -f '[%SAMPLE\t%ID\t%REF\t%ALT\t%GT\n]' \"$vcf_location\" | head -n 10";

    exec($cmd_calls, $pid_info);
    $pid_data = explode(' ', $pid_info[0]);
    $pid = $pid_data[1]; // Get the PID of the background process
    sleep(1);
    exec('kill -9 ' . $pid); // Kill the background process after 2 seconds
    $output_lines = file($output_location, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    // print_r($output_lines);

    // $output_lines = explode("\n", $output_string);
    foreach ($output_lines as $line) {
      $line_parts = explode(" ", $line);
      $rows[] = [$line_parts[0], $line_parts[1], $line_parts[2]];
    }

    $table = theme('table', [
      'header' => [
        t('Plant'),
        t('Name'),
        t('Description'),
        // t('Marker Type'),
      ],
      'rows' => $rows,
      'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
      'caption' => '',
      'colgroups' => NULL,
      'sticky' => FALSE,
      'empty' => '',
    ]);
    $pager = theme('pager', ['quantity', $total]);
    @unlink($output_location); // Clean up the temporary file
    return [
      'success' => TRUE,
      'content' => $pager . $table,
      'vcf_location_details' => $vcf_location_details,
      'total' => $total,
      //'cmd' => 'bcftools query -l "' . $vcf_location . '" | wc -l',
      'output_lines' => $output_lines,
      'cmd_calls' => $cmd_calls,
      'cmd_plants_count' => $cmd_plants_count,
      'cmd_markers_count' => $cmd_markers_count,
      'start_line' => $start_line,
      'end_line' => $end_line,
      'pid_info' => $pid_info,
    ];
  }
}

/**
 * Gets content of the 'Mass Spectrometry' tab of Study Details page.
 *
 * @param array $data
 *   POST data.
 *   Required keys are:
 *   'accession' - string.
 *     Required. Study accession. E.g., 'TGDR1090'.
 *   'page' - int. Optional. TPPS Form page number.
 *
 * @return string
 *   The HTML table of plants and genotypes associated with the submission.
 */
function tpps_ajax_get_mass_spectrometry_details_tab(array $data) {
  if (!tpps_access('access tpps details')) {
    return ['errors' => [t('Access denied')]];
  }
  if (empty($data['accession'])) {
    return ['errors' => [t('Empty study accession.')]];
  }
  $submission = new Submission($data['accession']);

  $page = $data['page'] ?? 0;
  $rows = [];
  $per_page = 20;
  $start = $page * $per_page;

  // $query = db_select('chado.phenotype', 'p');
  // $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  // $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  // $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  // $query->addExpression('count(p.phenotype_id)', 'count');
  // $query->condition('ps.project_id', $submission->getProjectId());
  // $query = $query->execute();
  // $total = $query->fetchObject()->count;

  // Get total count which is used for pager system (Rish)
  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->addExpression('count(p.phenotype_id)', 'count');
  $query->condition('p.attr_id', tpps_load_cvterm('intensity')->cvterm_id);
  $query->condition('ps.project_id', $submission->getProjectId());
  $query = $query->execute();
  $total = $query->fetchObject()->count;

  $_GET['page'] = $page;
  pager_default_initialize($total, $per_page);

  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->fields('p', [
    'phenotype_id',
    'name',
    'attr_id',
    'observable_id',
    'value',
  ]);
  $query->fields('s', array('uniquename', 'stock_id'));
  $query->condition('p.attr_id', tpps_load_cvterm('intensity')->cvterm_id);
  $query->condition('ps.project_id', $submission->getProjectId());
  $query->range($start, $per_page);

  return _tpps_details_tab_build_table($query, $total, $submission);
}

/**
 * Builds table for 'phenotype' and 'mass spectrometry' tabs.
 *
 * Helper table to avoid code duplication.
 *
 * @param object $query
 *   DB Query object.
 * @param mixed $total
 *   Total amount of records for pager.
 * @param object $submission
 *   TPPS Submission Object.
 *
 * @return array
 *   Returns rendered table with pager.
 */
function _tpps_details_tab_build_table($query, $total, $submission) {
  $query = $query->execute();
  $shared_state = $submission->sharedState;

  // Note: Below code is the same as mass spectrometry has.
  $cvterm_list = [];
  $options = [];

  $organism_number = $submission->getOrganismNumber();
  if (variable_get('tpps_study_details_use_snp_accosiation_year', FALSE)) {
    // Get value of the year from SNP Association file using synonym id.
    // Status: development not completed (works but requires more testing).
    // @todo Minor. Move to the SnpAssociation class.
    for ($i = 1; $i <= $organism_number; $i++) {
      $organism_key = 'organism-' . $i;
      if (
        empty($fid = SnpAssociation::getFileId($i, $shared_state))
        || empty($file = tpps_file_load($fid))
      ) {
        continue;
      }
      // @todo Get index of the year column in file.
      $snp_association_file_columns = TppsArray::getValue(
        $submission->sharedState,
        [
          'saved_values',
          TPPS_PAGE_4,
          $organism_key,
          'genotype',
          'files',
          'snps-association-columns',
        ]
      );
      if (empty($snp_association_file_columns)) {
        // @TODO Report an error. No columns for existing file.
        continue;
      }
      // Get 'Year' column letter in SNP Association file.
      $options['year_column'][$i] = array_search(
        SnpAssociation::DATA_TYPE_YEAR,
        $snp_association_file_columns
      );
      // Get 'Year' column letter in SNP Association file.
      $options['trait_column'][$i] = array_search(
        SnpAssociation::DATA_TYPE_ASSOCIATED_TRAIT,
        $snp_association_file_columns
      );
      $options['organism_index'] = $i;
      // Gets 'year' column of the SNP Association file in:
      // $options['snp_association_year'][$phenotype_attribute_name][$organism_index].
      tpps_file_iterator($fid, 'SnpAssociation::processRowSynonymYear', $options);
    }
  }

  while (($result = $query->fetchObject())) {
    $name = $result->name;
    preg_match('/^TGDR.{1,4}-(.*)$/', $result->uniquename, $matches);
    $tree = $matches[1];

    if (empty($cvterm_list[$name])) {
      $attr = chado_get_cvterm(['cvterm_id' => $result->attr_id])->name ?? '';
      $cvterm_list[$name] = [
        'obs' => chado_get_cvterm(['cvterm_id' => $result->observable_id])->name ?? '',
        'attr' => $attr,
      ];
    }

    // @TODO Minor. Remove duplicate code.
    if (!empty($options['snp_association_year'])) {
      // Show 'Year' column.
      // Add 'Year' from SNP Association file.
      $year = implode(', ', array_values($options['snp_association_year']
        [$cvterm_list[$name]['attr']] ?? []));
      $rows[] = [
        tpps_entity_link($result->stock_id, $tree, 'Stock'),
        tpps_entity_link($result->phenotype_id, $name, 'Phenotype'),
        $cvterm_list[$name]['attr'],
        $cvterm_list[$name]['obs'],
        $year,
        $result->value,
      ];
      $header = [
        t('Plant'),
        t('Name'),
        t('Attribute'),
        t('Entity'),
        t('Year'),
        t('Value'),
      ];
    }
    elseif (!empty($result->year)) {
      // Show 'Year' column.
      $header = [
        t('Plant'),
        t('Name'),
        t('Attribute'),
        t('Entity'),
        t('Year'),
        t('Value'),
      ];
      $rows[] = [
        tpps_entity_link($result->stock_id, $tree, 'Stock'),
        tpps_entity_link($result->phenotype_id, $name, 'Phenotype'),
        $cvterm_list[$name]['attr'],
        $cvterm_list[$name]['obs'],
        $result->year,
        $result->value,
      ];

    }
    else {
      // No 'Year' column.
      $rows[] = [
        tpps_entity_link($result->stock_id, $tree, 'Stock'),
        tpps_entity_link($result->phenotype_id, $name, 'Phenotype'),
        $cvterm_list[$name]['attr'],
        $cvterm_list[$name]['obs'],
        $result->value,
      ];
      $header = [
        t('Plant'),
        t('Name'),
        t('Attribute'),
        t('Entity'),
        t('Value'),
      ];
    }
  }
  if (!empty($header) && !empty($rows)) {
    $table = theme('table', [
      'header' => $header,
      'rows' => $rows,
      'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
      'caption' => '',
      'colgroups' => NULL,
      'sticky' => FALSE,
      'empty' => '',
    ]);
    $pager = theme('pager', ['quantity', $total]);
    return [
      'success' => TRUE,
      'content' => $pager . $table,
    ];
  }
  else {
    return [
      'success' => FALSE,
    ];
  }


}
