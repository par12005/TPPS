<?php

/**
 * @file
 * Defines functions to be used in the TPPS Details pages.
 */

/**
 * Starting point for most TPPS Details pages.
 *
 * Either returns the top details page, which allows users to select from
 * completed studies, or returns the per-submission details page, which allows
 * users to browse the details of a study.
 *
 * @param string $accession
 *   The accession to get details from. If NULL, show all submissions.
 *
 * @return string
 *   The appropriate output.
 */
function tpps_details($accession = NULL) {
  global $base_url;
  global $user;
  $output = "";

  if (empty($accession)) {
    $query = db_select('chado.plusgeno_view', 'pg');
    $query->addExpression('count(distinct(accession))', 'count');
    $query = $query->execute();
    $num_studies = $query->fetchObject()->count;

    $query = db_select('chado.plusgeno_view', 'pg');
    $query->addExpression('count(distinct(species))', 'count');
    $query = $query->execute();
    $num_species = $query->fetchObject()->count;

    $tpps_link = "<b><a target=\"blank\" href=\"/tpps\">submit your study</a></b>";
    $output .= "This page reflects community submissions associated with published studies. Each submission is associated with a longterm identifier that connects the genotype, phenotype, and/or environmental metrics assessed. These studies represent diversity assessements, association mapping, and landscape genomics. We accept studies with a variety of marker types. We encourage you to $tpps_link to TreeGenes and utilize the provided accession in your publication. We currently have <b>$num_studies</b> studies representing <b>$num_species</b> species.";
    if (module_exists('cartogratree')) {
      $output .= " Studies that have georeferenced accessions associated with them can be queried on <b><a href=\"cartogratree\">CartograPlant.</a></b>";
    }
    $form = drupal_get_form('tpps_details_search');
    $output .= render($form);
    $output .= tpps_details_tags_sort();
    $output .= "<div id='tpps-details-table'>" . tpps_details_top() . "</div>";
    drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
    drupal_add_js(drupal_get_path('module', 'tpps') . TPPS_JS_PATH);
    return $output;
  }

  $output = "<a href=\"$base_url/tpps/details\">Return to TPPS Details</a>$output";
  $state = tpps_load_submission($accession);
  if (!empty($state)) {
    if ($state['status'] != 'Approved') {
      drupal_set_message(t('This submission is still under construction. Some of the information may be available here, but please keep in mind that it may be incomplete or subject to change in the near future.'), 'warning');
      if (function_exists('tpps_details_byaccession_page')) {
        $output .= @tpps_details_byaccession_page()['raw_markup']['#markup'];
        drupal_add_js(drupal_get_path('module', 'tpps') . TPPS_JS_PATH);
        return $output;
      }
    }

    if (empty($state['saved_values'][TPPS_PAGE_3]['skip_validation'])) {
      $map_api_key = variable_get('tpps_maps_api_key', NULL);
      $map_api_tools = "<script src=\"https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/markerclusterer.js\"></script><script src=\"https://maps.googleapis.com/maps/api/js?key=$map_api_key&callback=initMap\"
        async defer></script>"
        . "<div id=\"_map_wrapper\"></div>";
      $tree_info = array();
      if (!empty($state['tree_info'])) {
        foreach ($state['tree_info'] as $name => $info) {
          if (!empty($info['lat']) and !empty($info['lng'])) {
            $tree_info[] = array(
              $name,
              $info['lat'],
              $info['lng'],
            );
          }
        }
        drupal_add_js(array('tpps' => array('tree_info' => $tree_info)), 'setting');
      }
      $output .= $map_api_tools;
    }
    drupal_add_js(drupal_get_path('module', 'tpps') . TPPS_JS_PATH);
    drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);

    $pub_link = $state['title'];
    $pub_id = db_select('chado.project_pub', 'p')
      ->fields('p', array('pub_id'))
      ->condition('project_id', $state['ids']['project_id'])
      ->execute()->fetchObject()->pub_id;
    if (!empty($pub_id)) {
      $bundle = tripal_load_bundle_entity(array('label' => 'Publication'));
      $pub_id = chado_get_record_entity_by_bundle($bundle, $pub_id);
      $pub_link = "<a href='$base_url/Publication/$pub_id'>$pub_link</a>";
    }

    if (!empty($state['saved_values'][TPPS_PAGE_3]['skip_validation'])) {
      $image_path = drupal_get_path('module', 'tpps') . '/images/';
      $pub_link .= "<img title=\"This submission contains no location information\" src=\"/{$image_path}no_location.png\">";
    }

    $output .= "<h2>$pub_link</h2>";
    $output .= tpps_show_tags(tpps_submission_get_tags($state['accession']));
    $primary_author = $state['authors'][0];
    if (preg_match('/,/', $primary_author)) {
      $parts = explode(',', $primary_author);
      $lastname = trim($parts[0]);
    }
    else {
      $parts = explode(' ', $primary_author);
      $lastname = $parts[count($parts) - 1];
    }
    $output .= "<h4>$lastname et al {$state['pyear']}";
    if (!empty($state['journal'])) {
      $output .= " published in {$state['journal']}";
    }
    $output .= "</h4>";
    $output .= "<div id=\"abstract\"><a href=\"#abstract\">Click to show abstract</a></div>"
      . "<script>jQuery(document).ready(jQuery('#abstract').on('click', function() {
          jQuery('#abstract')[0].innerHTML = `{$state['abstract']}`;
        }));</script>";
    if (empty($state['saved_values'][TPPS_PAGE_3]['skip_validation']) and module_exists('cartogratree')) {
      $output .= "<a href=\"$base_url/cartogratree?accession=$accession\" target=\"blank\">View in CartograPlant</a><br>";
    }
    $output .= "<a href=\"$base_url/tpps/api/files/$accession\">Download compressed raw files</a><br>";
    if ($state['saved_values'][TPPS_PAGE_1]['publication']['status'] != 'Published' and $user->uid == tpps_load_submission($accession, FALSE)->uid) {
      $output .= "<a href=\"$base_url/tpps/$accession/edit-publication\">Edit publication information</a>";
    }

    $output .= tpps_details_tabs($state);
    return $output;
  }

  drupal_set_message(t('This submission is pending resubmission through the new TPPS pipeline. Some of the information may be available here, but please keep in mind that it may be incomplete or subject to change in the near future.'), 'warning');

  if (function_exists('tpps_details_byaccession_page')) {
    $output .= @tpps_details_byaccession_page()['raw_markup']['#markup'];
    drupal_add_js(drupal_get_path('module', 'tpps') . TPPS_JS_PATH);
  }
  return $output;
}

/**
 * Returns a template for details tabs.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   HTML for the details tabs.
 */
function tpps_details_tabs(array &$state) {
  $output = '<ul class="nav nav-tabs" role="tablist">
    <li class="nav-item"><a class="nav-link active" role="tab" data-toggle="tab" href="#species">Species</a></li>
    <li class="nav-item"><a class="nav-link" role="tab" data-toggle="tab" href="#study">Study Details</a></li>
    <li class="nav-item"><a class="nav-link" role="tab" data-toggle="tab" href="#trees">Plants</a></li>';
  $p_exist = FALSE;
  $g_exist = FALSE;
  $e_exist = FALSE;
  for ($i = 1; $i <= $state['stats']['species_count']; $i++) {
    if (!empty($state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype'])) {
      $p_exist = TRUE;
    }
    if (!empty($state['saved_values'][TPPS_PAGE_4]["organism-$i"]['genotype'])) {
      $g_exist = TRUE;
    }
    if (!empty($state['saved_values'][TPPS_PAGE_4]["organism-$i"]['environment'])) {
      $e_exist = TRUE;
    }
  }
  $output .= $p_exist ? '<li class="nav-item"><a class="nav-link" role="tab" data-toggle="tab" href="#phenotype">Phenotypes / Environments</a></li>' : "";
  $output .= $g_exist ? '<li class="nav-item"><a class="nav-link" role="tab" data-toggle="tab" href="#genotype">Genotypes</a></li>' : "";
  $output .= '</ul><div id="tab-content" class="tab-content">';

  $output .= "<div id=\"species\" role=\"tabpanel\" class=\"tab-pane fade in active\"></div>";
  $output .= "<div id=\"study\" role=\"tabpanel\" class=\"tab-pane fade\"></div>";
  $output .= "<div id=\"trees\" role=\"tabpanel\" class=\"tab-pane fade\"></div>";

  $output .= $p_exist ? "<div id=\"phenotype\" role=\"tabpanel\" class=\"tab-pane fade\"></div>" : "";
  $output .= $g_exist ? "<div id=\"genotype\" role=\"tabpanel\" class=\"tab-pane fade\"></div>" : "";

  $output .= '</div>';
  return $output;
}

/**
 * A callback to print a single detail tab.
 *
 * @param string $accession
 *   The accession number of the submission.
 * @param string $type
 *   The type of details we want to print.
 */
function tpps_details_tab($accession, $type) {
  $function = "tpps_details_$type";
  $state = tpps_load_submission($accession);
  print($function($state));
}

/**
 * Builds the TPPS Details page when no accession is provided.
 *
 * @return string
 *   The HTML string for the TPPS Details page.
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
  $submissions->range($start, $per_page);
  $submissions->orderBy('pg.accession');
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
    if (empty(tpps_load_submission($sub->accession))) {
      $warning = "<img src='$base_url/misc/message-16-warning.png' title='This study has not yet been resubmitted through the new TPPS pipeline'> ";
    }

    $row = array(
      "<a href=\"$base_url/tpps/details/{$sub->accession}\">{$sub->accession}</a>",
      "$warning<a href=\"$base_url/tpps/details/{$sub->accession}\">{$sub->title}</a>",
      tpps_show_tags(tpps_submission_get_tags($sub->accession)),
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

  $output = theme_table($vars);
  $pager = theme('pager', array('quantity', $total));
  return $pager . $output;
}

/**
 * Serves as a direct callback to print the contents of tpps_details_top().
 */
function tpps_details_top_callback() {
  print(tpps_details_top());
}

/**
 * Define a search form to search and filter through the TPPS Details page.
 *
 * @param array $form
 *   The form array object.
 * @param array $form_state
 *   The form state array object.
 *
 * @return array
 *   The populated search form.
 */
function tpps_details_search(array $form, array $form_state) {
  $params = drupal_get_query_parameters();
  $form['details_type'] = array(
    '#type' => 'select',
    '#options' => array(
      'title' => 'Title',
      'species' => 'Species',
      'accession' => 'Accession',
      'author' => 'Primary Author',
      'year' => 'Publication Year',
      'phenotype_name' => 'Phenotype Name',
      'phenotype_ontology' => 'Phenotype Ontology Name',
      'genotype_name' => 'Genotype Name',
      'genotype_marker' => 'Genotype Marker Type',
      'tags' => 'Tags',
    ),
    '#ajax' => array(
      'wrapper' => 'tpps-details-form',
      'callback' => 'tpps_details_form_callback',
    ),
    '#prefix' => '<div id="tpps-details-form">',
    '#default_value' => $params['type'] ?? NULL,
  );

  $ops = array(
    '~*' => '~*',
    'LIKE' => 'LIKE',
    '=' => '=',
  );

  $form['details_op'] = array(
    '#type' => 'select',
    '#options' => $ops,
    '#default_value' => $params['op'] ?? NULL,
  );

  $form['details_value'] = array(
    '#type' => 'textfield',
    '#suffix' => '</div>',
    '#autocomplete_path' => 'tpps/autocomplete/project_title',
    '#default_value' => $params['value'] ?? NULL,
  );

  if (empty($form_state['values']['details_type'])) {
    $form_state['values']['details_type'] = $params['type'] ?? NULL;
  }

  if (!empty($form_state['values']['details_type'])) {
    switch ($form_state['values']['details_type']) {
      case 'title':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/project_title';
        break;

      case 'species':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/species';
        break;

      case 'accession':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/project_accession';
        break;

      case 'author':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/author';
        break;

      case 'phenotype_name':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/phenotype';
        break;

      case 'phenotype_ontology':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/phenotype_ontology';
        break;

      case 'genotype_name':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/genotype';
        break;

      case 'genotype_marker':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/genotype_marker';
        break;

      default:
        $form['details_value']['#autocomplete_path'] = NULL;
        break;
    }
  }

  $form['details_search'] = array(
    '#type' => 'button',
    '#button_type' => 'button',
    '#value' => t('Search'),
  );

  $form['#attributes'] = array(
    'style' => 'text-align:center',
  );

  return $form;
}

/**
 * Display TPPS tags to sort Details page.
 *
 * @return string
 *   Tag options as rendered HTML.
 */
function tpps_details_tags_sort() {
  $output = "";

  $query = db_select('tpps_tag', 't')
    ->fields('t')
    ->orderBy('static', 'DESC')
    ->orderBy('tpps_tag_id')
    ->execute();
  $tags = array();
  while (($result = $query->fetchObject())) {
    $tags[$result->tpps_tag_id] = array(
      'id' => $result->tpps_tag_id,
      'name' => $result->name,
      'color' => $result->color,
      'static' => $result->static,
    );
  }

  $output .= tpps_show_tags($tags);

  return "<label for=\"tpps-tags-filter\">Sort by tags:</label><div id=\"tpps-tags-filter\">" . $output . "</div>";
}

/**
 * AJAX callback for the search form details_op element.
 *
 * @param array $form
 *   The form being updated.
 * @param array $form_state
 *   The state of the form being updated.
 *
 * @return array
 *   The part of the form to be updated.
 */
function tpps_details_form_callback(array $form, array $form_state) {
  return array(
    'details_type' => $form['details_type'],
    'details_op' => $form['details_op'],
    'details_value' => $form['details_value'],
  );
}

/**
 * Creates the table for species details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of species associated with the submission.
 */
function tpps_details_species(array &$state) {
  $query = db_select('chado.organism', 'o');
  $query->join('chado.pub_organism', 'po', 'o.organism_id = po.organism_id');
  $query->join('chado.project_pub', 'pp', 'pp.pub_id = po.pub_id');
  $query->fields('o', array('organism_id', 'genus', 'species', 'common_name'));
  $query->condition('pp.project_id', $state['ids']['project_id']);
  $query = $query->execute();
  $rows = array();

  while (($result = $query->fetchObject())) {
    $id = $result->organism_id;
    $common_name = $result->common_name;
    $family = $order = "";

    if (tpps_chado_prop_exists('organism', $id, 'family')) {
      $family = db_select('chado.organismprop', 'o')
        ->fields('o', array('value'))
        ->condition('organism_id', $id)
        ->condition('type_id', tpps_load_cvterm('family')->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    if (tpps_chado_prop_exists('organism', $id, 'order')) {
      $order = db_select('chado.organismprop', 'o')
        ->fields('o', array('value'))
        ->condition('organism_id', $id)
        ->condition('type_id', tpps_load_cvterm('order')->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    if (empty($common_name) and tpps_chado_prop_exists('organism', $id, 'common name')) {
      $common_name = db_select('chado.organismprop', 'o')
        ->fields('o', array('value'))
        ->condition('organism_id', $id)
        ->condition('type_id', tpps_load_cvterm('common name')->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    $name = "{$result->genus} {$result->species}";
    $link = tpps_entity_link($id, $name, 'Organism');
    $rows[$name] = array(
      $order,
      $family,
      $link,
      $common_name,
    );
  }

  $vars = array(
    'header' => array(
      'Order',
      'Family',
      'Species',
      'Common Name',
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
  return theme_table($vars);
}

/**
 * Creates the table for study information details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of study information of the submission.
 */
function tpps_details_study(array &$state) {
  $rows = array();

  $species = array();
  for ($i = 1; $i <= $state['stats']['species_count']; $i++) {
    $id = $state['ids']['organism_ids'][$i];
    $species_name = $state['saved_values'][TPPS_PAGE_1]['organism'][$i]['name'];
    $species[] = tpps_entity_link($id, $species_name, 'Organism');
  }

  $study_types = array(
    1 => 'Natural Population (Landscape)',
    2 => 'Growth Chamber',
    3 => 'Greenhouse',
    4 => 'Experimental/Common Garden',
    5 => 'Plantation',
  );

  $files = array();
  foreach ($state['files'] as $fid) {
    $file = file_load($fid);
    $file_url = check_plain(file_create_url($file->uri));
    $files[] = "<a href='$file_url' target='blank'>$file->filename</a>";
  }

  $pub_link = $state['title'];
  $pub_id = db_select('chado.project_pub', 'p')
    ->fields('p', array('pub_id'))
    ->condition('project_id', $state['ids']['project_id'])
    ->execute()->fetchObject()->pub_id;
  if (!empty($pub_id)) {
    $bundle = tripal_load_bundle_entity(array('label' => 'Publication'));
    $pub_id = chado_get_record_entity_by_bundle($bundle, $pub_id);
    $pub_link = "<a href='$base_url/Publication/$pub_id'>$pub_link</a>";
  }
  $rows[] = array('Accession', $state['accession']);
  $rows[] = array('Title', $pub_link);
  $rows[] = array('Authors', implode(', ', $state['authors']));
  $rows[] = array('Species', implode('<br>', $species));
  $rows[] = array('Data Type', $state['saved_values'][TPPS_PAGE_2]['data_type']);
  $rows[] = array(
    'Study Type',
    $study_types[$state['saved_values'][TPPS_PAGE_2]['study_type']],
  );
  $rows[] = array('File Downloads', implode('<br>', $files));
  $rows[] = array('Population Size', $state['stats']['tree_count']);
  if (!empty($state['stats']['phenotype_count'])) {
    $rows[] = array('Phenotype Count', $state['stats']['phenotype_count']);
  }
  if (!empty($state['stats']['unique_phenotypes'])) {
    $rows[] = array('Unique Phenotypes', $state['stats']['unique_phenotypes']);
  }
  // @todo genotype counts.
  // $rows[] = array('Genotype Count', $state['stats']['genotype_count']);
  $vars = array(
    'header' => array(),
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
  return theme_table($vars);
}

/**
 * Creates the table of individual plant details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of plants associated with the submission.
 */
function tpps_details_trees(array &$state) {
  $params = drupal_get_query_parameters($_POST);
  $page = 0;
  if (!empty($params['page'])) {
    $page = $params['page'];
  }
  $rows = array();
  $per_page = 20;

  if (empty($state['tree_info'])) {
    $output = t('The plant information for this study has not been properly updated. Please contact your site administrator.');
    return $output;
  }

  $skip = !empty($state['saved_values'][TPPS_PAGE_3]['skip_validation']);

  $_GET['page'] = $page;
  pager_default_initialize(count($state['tree_info']), $per_page);
  $chunks = array_chunk($state['tree_info'], $per_page, TRUE);

  foreach ($chunks[$page] as $name => $info) {
    $id = $info['organism_id'];
    $form_number = array_search($id, $state['ids']['organism_ids']);
    $species_name = $state['saved_values'][TPPS_PAGE_1]['organism'][$form_number]['name'];

    $row = array(
      tpps_entity_link($info['stock_id'], $name, 'Stock'),
      tpps_entity_link($id, $species_name, 'Organism'),
    );
    if (!$skip) {
      $row[] = $info['lat'];
      $row[] = $info['lng'];
    }
    $rows[] = $row;
  }

  $header = array(
    'Identifier',
    'Species',
  );
  if (!$skip) {
    $header[] = 'Latitude';
    $header[] = 'Longitude';
  }

  $vars = array(
    'header' => $header,
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
  $table = theme_table($vars);
  $pager = theme('pager', array('quantity', count($state['tree_info'])));
  return $pager . $table;
}

/**
 * Creates the table of individual plant phenotype details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of plants and phenotypes associated with the submission.
 */
function tpps_details_phenotype(array &$state) {
  $params = drupal_get_query_parameters($_POST);
  $page = 0;
  if (!empty($params['page'])) {
    $page = $params['page'];
  }
  $rows = array();
  $per_page = 20;
  $start = $page * $per_page;

  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->addExpression('count(p.phenotype_id)', 'count');
  $query->condition('ps.project_id', $state['ids']['project_id']);
  $query = $query->execute();
  $total = $query->fetchObject()->count;

  $_GET['page'] = $page;
  pager_default_initialize($total, $per_page);

  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->fields('p', array(
    'phenotype_id',
    'name',
    'attr_id',
    'observable_id',
    'value',
  ));
  $query->fields('s', array('uniquename', 'stock_id'));
  $query->condition('ps.project_id', $state['ids']['project_id']);
  $query->range($start, $per_page);
  $query = $query->execute();

  while (($result = $query->fetchObject())) {
    preg_match('/^TGDR...-(.*)$/', $result->uniquename, $matches);
    $tree = $matches[1];
    $obs = chado_get_cvterm(array(
      'cvterm_id' => $result->observable_id,
    ))->name ?? "";
    $attr = chado_get_cvterm(array(
      'cvterm_id' => $result->attr_id,
    ))->name ?? "";

    $rows[] = array(
      tpps_entity_link($result->stock_id, $tree, 'Stock'),
      tpps_entity_link($result->phenotype_id, $result->name, 'Phenotype'),
      $attr,
      $obs,
      $result->value,
    );
  }

  $vars = array(
    'header' => array(
      'Plant',
      'Name',
      'Attribute',
      'Entity',
      'Value',
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
  $table = theme_table($vars);
  $pager = theme('pager', array('quantity', $total));
  return $pager . $table;
}

/**
 * Creates the table of individual plant genotype details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of plants and genotypes associated with the submission.
 */
function tpps_details_genotype(array &$state) {
  $params = drupal_get_query_parameters($_POST);
  $page = 0;
  if (!empty($params['page'])) {
    $page = $params['page'];
  }
  $rows = array();
  $per_page = 20;
  $start = $page * $per_page;

  $query = db_select('chado.genotype', 'g');
  $query->join('chado.stock_genotype', 'sg', 'sg.genotype_id = g.genotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sg.stock_id');
  $query->addExpression('count(g.genotype_id)', 'count');
  $query->condition('ps.project_id', $state['ids']['project_id']);
  $query = $query->execute();
  $total = $query->fetchObject()->count;

  $_GET['page'] = $page;
  pager_default_initialize($total, $per_page);

  $query = db_select('chado.genotype', 'g');
  $query->join('chado.stock_genotype', 'sg', 'sg.genotype_id = g.genotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sg.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sg.stock_id');
  $query->fields('g', array(
    'genotype_id',
    'name',
    'uniquename',
    'description',
    'type_id',
  ));
  $query->fields('s', array('uniquename', 'stock_id'));
  $query->condition('ps.project_id', $state['ids']['project_id']);
  $query->range($start, $per_page);
  $query = $query->execute();

  while (($result = $query->fetchObject())) {
    preg_match('/^TGDR...-(.*)$/', $result->s_uniquename, $matches);
    $tree = $matches[1];
    $type = chado_get_cvterm(array(
      'cvterm_id' => $result->type_id,
    ))->name ?? "";
    $rows[] = array(
      tpps_entity_link($result->stock_id, $tree, 'Stock'),
      tpps_entity_link($result->genotype_id, $result->uniquename, 'Genotype'),
      $result->description,
      $type,
    );
  }

  $vars = array(
    'header' => array(
      'Plant',
      'Name',
      'Description',
      'Marker Type',
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
  $table = theme_table($vars);
  $pager = theme('pager', array('quantity', $total));
  return $pager . $table;
}
