<?php

/**
 * @file
 * TPPS Study details page.
 *
 * URL: /tpps/details/top.
 */

/**
 * Serves as a direct callback to print the contents of tpps_details_top().
 */
function tpps_details_top_callback() {
  print(tpps_details_top());
}

/**
 * Builds the TPPS Details page when no accession is provided.
 *
 * @return string
 *   The HTML string for the TPPS Details page.
 *
 * @TODO [VS] Minor. This page was broken. Seems not used and outdated. Remove it.
 */
function tpps_details_top() {
  global $base_url;
  $params = drupal_get_query_parameters($_POST);
  $page = 0;
  if (empty($params)) {
    $params = drupal_get_query_parameters();
  }
  if (!empty($params['page'])) {
    $page = $params['page'];
  }
  $per_page = 20;

  $query = db_select('chado.plusgeno_view', 'pg');
  $query->addExpression('count(distinct(pg.accession))', 'count');
  // $query->addExpression('substring(pg.accession,5)', 'accession_number');
  if (!empty($params['type']) and !empty($params['value']) and !empty($params['op'])) {
    switch ($params['type']) {
      case 'title':
      case 'species':
      case 'project_id':
      case 'accession':
      case 'author':
      case 'year':
        $query->condition($params['type'], $params['value'], $params['op']);
        break;

      case 'phenotype_name':
        $query->innerJoin('chado.project_stock', 'ps', 'ps.project_id = pg.project_id');
        $query->innerJoin('chado.stock_phenotype', 'sp', 'sp.stock_id = ps.stock_id');
        $query->innerJoin('chado.phenotype', 'ph', 'ph.phenotype_id = sp.phenotype_id');
        $query->condition('ph.name', $params['value'], $params['op']);
        break;

      case 'phenotype_ontology':
        $query->innerJoin('chado.project_stock', 'ps', 'ps.project_id = pg.project_id');
        $query->innerJoin('chado.stock_phenotype', 'sp', 'sp.stock_id = ps.stock_id');
        $query->innerJoin('chado.phenotype', 'ph', 'ph.phenotype_id = sp.phenotype_id');
        $query->innerJoin('chado.cvterm', 'cvt', 'cvt.cvterm_id = ph.attr_id');
        $query->innerJoin('chado.cv', 'cv', 'cv.cv_id = cvt.cv_id');
        $query->condition('cv.name', $params['value'], $params['op']);
        break;

      case 'genotype_name':
        $query->innerJoin('chado.tpps_search_genotype_name', 'g', 'g.project_id = pg.project_id');
        $query->condition('g.name', $params['value'], $params['op']);
        break;

      case 'genotype_marker':
        $query->innerJoin('chado.tpps_search_genotype_marker', 'g', 'g.project_id = pg.project_id');
        $query->condition('g.name', $params['value'], $params['op']);
        break;

      case 'tags':
        $query->innerJoin('public.tpps_submission', 'ts', 'ts.accession = pg.accession');
        $query->innerJoin('public.tpps_submission_tag', 'st', 'st.tpps_submission_id = ts.tpps_submission_id');
        $query->innerJoin('public.tpps_tag', 'tt', 'tt.tpps_tag_id = st.tpps_tag_id');
        $query->condition('tt.name', $params['value'], $params['op']);
        break;

      default:
        break;
    }
  }

  $query = $query->execute();
  $total = $query->fetchObject()->count;

  $_GET['page'] = $page;
  $page = pager_default_initialize($total, $per_page);
  $start = $page * $per_page;

  $submissions = db_select('chado.plusgeno_view', 'pg');
  $submissions->distinct();
  $submissions->innerJoin('chado.project', 'p', 'p.project_id = pg.project_id');
  if (!empty($params['type']) and !empty($params['value']) and !empty($params['op'])) {
    switch ($params['type']) {
      case 'title':
      case 'species':
      case 'project_id':
      case 'accession':
      case 'author':
      case 'year':
        $submissions->condition($params['type'], $params['value'], $params['op']);
        break;

      case 'phenotype_name':
        $submissions->innerJoin('chado.project_stock', 'ps', 'ps.project_id = pg.project_id');
        $submissions->innerJoin('chado.stock_phenotype', 'sp', 'sp.stock_id = ps.stock_id');
        $submissions->innerJoin('chado.phenotype', 'ph', 'ph.phenotype_id = sp.phenotype_id');
        $submissions->condition('ph.name', $params['value'], $params['op']);
        break;

      case 'phenotype_ontology':
        $submissions->innerJoin('chado.project_stock', 'ps', 'ps.project_id = pg.project_id');
        $submissions->innerJoin('chado.stock_phenotype', 'sp', 'sp.stock_id = ps.stock_id');
        $submissions->innerJoin('chado.phenotype', 'ph', 'ph.phenotype_id = sp.phenotype_id');
        $submissions->innerJoin('chado.cvterm', 'cvt', 'cvt.cvterm_id = ph.attr_id');
        $submissions->innerJoin('chado.cv', 'cv', 'cv.cv_id = cvt.cv_id');
        $submissions->condition('cv.name', $params['value'], $params['op']);
        break;

      case 'genotype_name':
        $submissions->innerJoin('chado.tpps_search_genotype_name', 'g', 'g.project_id = pg.project_id');
        $submissions->condition('g.name', $params['value'], $params['op']);
        break;

      case 'genotype_marker':
        $submissions->innerJoin('chado.tpps_search_genotype_marker', 'g', 'g.project_id = pg.project_id');
        $submissions->condition('g.name', $params['value'], $params['op']);
        break;

      case 'tags':
        $submissions->innerJoin('public.tpps_submission', 'ts', 'ts.accession = pg.accession');
        $submissions->innerJoin('public.tpps_submission_tag', 'st', 'st.tpps_submission_id = ts.tpps_submission_id');
        $submissions->innerJoin('public.tpps_tag', 'tt', 'tt.tpps_tag_id = st.tpps_tag_id');
        $submissions->condition('tt.name', $params['value'], $params['op']);
        break;

      default:
        break;
    }
  }
  $submissions->fields('pg', array(
    'title',
    'project_id',
    'accession',
    'tree_count',
    'phenotypes_assessed',
    'phen_count',
    'gen_count',
  ));
  $submissions->addExpression('SUBSTRING(pg.accession, 5)::integer', 'accession_number_only');
  $submissions->range($start, $per_page);
  // $submissions->orderBy('pg.accession');
  $submissions->orderBy('accession_number_only', 'ASC');
  $submissions = $submissions->execute();

  $rows = array();
  while (($sub = $submissions->fetchObject())) {
    $proj_id = $sub->project_id;

    $query = db_select('chado.organism', 'o');
    $query->join('chado.pub_organism', 'po', 'o.organism_id = po.organism_id');
    $query->join('chado.project_pub', 'pp', 'pp.pub_id = po.pub_id');
    $query->fields('o', array('organism_id', 'genus', 'species'));
    $query->condition('pp.project_id', $proj_id);
    $query->distinct();
    $query = $query->execute();

    $species = array();
    while (($result = $query->fetchObject())) {
      $species[] = tpps_entity_link($result->organism_id, "{$result->genus} {$result->species}", 'Organism');
    }

    $warning = "";
    $submission = new Submission($sub->accession);
    if ($submission->doesNotExist()) {
      $title = t('This study has not yet been resubmitted through the new TPPS pipeline');
      $warning = "<img src='$base_url/misc/message-16-warning.png' title='"
        . $title . "'> ";
    }

    $row = array(
      "<a href=\"$base_url/tpps/details/{$sub->accession}\">{$sub->accession}</a>",
      "$warning<a href=\"$base_url/tpps/details/{$sub->accession}\">{$sub->title}</a>",
      theme('submission_tag_list', ['tags' => $submission->getTagList()]),
      implode('<br>', $species),
      $sub->tree_count,
      $sub->phenotypes_assessed,
      $sub->phen_count,
      $sub->gen_count,
    );
    $rows[$sub->accession] = $row;
  }

  ksort($rows);

  $vars = array(
    'header' => array(
      'Accession',
      'Title',
      'Tags',
      'Species',
      'Plant Count',
      'Phenotypes Assessed',
      'Phenotypic Measures',
      'Genotype Count',
    ),
    'rows' => $rows,
    'attributes' => array(
      'class' => array('view'),
      'id' => 'tpps_table_display',
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );

  $output = theme('table', $vars);
  $pager = theme('pager', array('quantity', $total));
  return $pager . $output;
}
