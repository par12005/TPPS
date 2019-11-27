<?php

/**
 * 
 */

/**
 * 
 */
function tpps_details($accession = NULL) {
  global $base_url;
  $output = "";

  if (empty($accession)) {
    return tpps_details_top();
  }

  $output = "<a href=\"$base_url/tpps/details\">Return to TPPS Details</a>$output";
  $state = tpps_load_submission($accession);
  if (!empty($state)) {
    $map_api_key = variable_get('tpps_maps_api_key', NULL);
    $map_api_tools = "<script src=\"https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/markerclusterer.js\"></script><script src=\"https://maps.googleapis.com/maps/api/js?key=$map_api_key&callback=initMap\"
      async defer></script>"
      . "<div id=\"map_wrapper\"></div>";
    $tree_info = array();
    if (!empty($state['tree_info'])) {
      foreach ($state['tree_info'] as $name => $info) {
        $tree_info[] = array(
          $name,
          $info['lat'],
          $info['lng'],
        );
      }
      drupal_add_js(array('tpps' => array('tree_info' => $tree_info)), 'setting');
    }
    $output .= $map_api_tools;
    drupal_add_js(drupal_get_path('module', 'tpps') . TPPS_JS_PATH);
    drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);

    $output .= "<h2>{$state['title']}</h2>";
    $authors = implode(', ', $state['authors']);
    $output .= "<h4>$authors</h4>";
    $output .= "<h4>Published {$state['pyear']}, in {$state['journal']}</h4>";
    $output .= "<div id=\"abstract\"><a href=\"#abstract\">Click to show abstract</a></div>"
      . "<script>jQuery(document).ready(jQuery('#abstract').on('click', function() {
          jQuery('#abstract')[0].innerHTML = \"{$state['abstract']}\";
        }));</script>";
    if (module_exists('cartogratree')) {
      $output .= "<a href=\"$base_url/cartogratree?accession=$accession\" target=\"blank\">View in CartograTree</a><br>";
    }
    $output .= "<a href=\"$base_url/tpps/api/files/$accession\">Download compressed raw files</a><br>";

    $output .= tpps_details_tabs($state);
    return $output;
  }

  return $output;
}

/**
 * 
 */
function tpps_details_tabs(&$state) {
  $output = '<ul class="nav nav-tabs">
    <li class="active"><a data-toggle="tab" class="active" href="#species">Species</a></li>
    <li><a data-toggle="tab" href="#study_details">Study Details</a></li>
    <li><a data-toggle="tab" href="#trees">Trees</a></li>';
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
  $output .= $p_exist ? '<li><a data-toggle="tab" href="#phenotype">Phenotypes</a></li>' : "";
  $output .= $g_exist ? '<li><a data-toggle="tab" href="#genotype">Genotypes</a></li>' : "";
  $output .= $e_exist ? '<li><a data-toggle="tab" href="#environment">Environments</a></li>' : "";
  $output .= '</ul><div class="tab-content">';

  $output .= tpps_details_species($state);
  $output .= tpps_details_study($state);
  $output .= tpps_details_trees($state);

  $output .= $p_exist ? tpps_details_phenotype($state) : "";
  $output .= $g_exist ? tpps_details_genotype($state) : "";
  $output .= $e_exist ? tpps_details_environment($state) : "";
  $output .= '</div>';
  return $output;
}

/**
 * Builds the TPPS Details page when no accession is provided.
 *
 * @return string
 *   The HTML string for the TPPS Details page.
 */
function tpps_details_top() {
  global $base_url;

  $submissions = db_select('chado.plusgeno_view', 'pg');
  $submissions->innerJoin('chado.project', 'p', 'p.project_id = pg.project_id');
  $submissions->fields('pg', array(
    'title',
    'project_id',
    'accession',
    'phenotypes_assessed',
    'phen_count',
    'gen_count',
  ));
  $submissions = $submissions->execute();

  $rows = array();
  while (($sub = $submissions->fetchObject())) {
    $proj_id = $sub->project_id;

    $query = db_select('chado.stock', 's');
    $query->join('chado.organism', 'o', 'o.organism_id = s.organism_id');
    $query->fields('o', array('genus', 'species', 'organism_id'));
    $query->addExpression('count(s.stock_id)', 'count');
    $query->condition('s.uniquename', db_like($sub->accession) . '%', 'LIKE');
    $query->groupBy('o.organism_id');
    $query = $query->execute();

    $tree_count = 0;
    $species = array();
    while (($result = $query->fetchObject())) {
      $tree_count += $result->count;
      $species[] = tpps_entity_link($result->organism_id, "{$result->genus} {$result->species}", 'Organism');
    }

    $row = array(
      "<a href=\"$base_url/tpps/details/{$sub->accession}\">{$sub->accession}</a>",
      "<a href=\"$base_url/tpps/details/{$sub->accession}\">{$sub->title}</a>",
      implode('<br>', $species),
      $tree_count,
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
      'Species',
      'Tree Count',
      'Phenotypes Assessed',
      'Phenotypic Measures',
      'Genotype Count',
    ),
    'rows' => $rows,
    'attributes' => array('class' => array('view'), 'id' => 'tpps_table_display'),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );

  $output = theme_table($vars);
  return $output;
}

/**
 * 
 */
function tpps_details_species(&$state) {
  $rows = array();

  for ($i = 1; $i <= $state['stats']['species_count']; $i++) {
    $id = $state['ids']['organism_ids'][$i];
    $family = $order = $common_name = "";

    if (tpps_chado_prop_exists('organism', $id, 'family')) {
      $family = db_select('chado.organismprop', 'o')
        ->fields('o', array('value'))
        ->condition('organism_id', $id)
        ->condition('type_id', chado_get_cvterm(array(
          'name' => 'family',
        ))->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    if (tpps_chado_prop_exists('organism', $id, 'order')) {
      $order = db_select('chado.organismprop', 'o')
        ->fields('o', array('value'))
        ->condition('organism_id', $id)
        ->condition('type_id', chado_get_cvterm(array(
          'name' => 'order',
        ))->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    if (tpps_chado_prop_exists('organism', $id, 'common name')) {
      $common_name = db_select('chado.organismprop', 'o')
        ->fields('o', array('value'))
        ->condition('organism_id', $id)
        ->condition('type_id', chado_get_cvterm(array(
          'name' => 'common name',
        ))->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    $species_name = $state['saved_values'][TPPS_PAGE_1]['organism'][$i];
    $species_link = tpps_entity_link($id, $species_name, 'Organism');

    $rows[] = array(
      $order,
      $family,
      $species_link,
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
    'attributes' => array('class' => array('view'), 'id' => 'tpps_table_display'),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );
  $table = theme_table($vars);
  $output = "<div id=\"species\" class=\"tab-pane fade in active show\">$table</div>";
  return $output;
}

/**
 * 
 */
function tpps_details_study(&$state) {
  $rows = array();

  $species = array();
  for ($i = 1; $i <= $state['stats']['species_count']; $i++) {
    $id = $state['ids']['organism_ids'][$i];
    $species_name = $state['saved_values'][TPPS_PAGE_1]['organism'][$i];
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

  $rows[] = array('Accession', $state['accession']);
  $rows[] = array('Title', $state['title']);
  $rows[] = array('Authors', implode(', ', $state['authors']));
  $rows[] = array('Species', implode('<br>', $species));
  $rows[] = array('Data Type', $state['saved_values'][TPPS_PAGE_2]['data_type']);
  $rows[] = array('Study Type', $study_types[$state['saved_values'][TPPS_PAGE_2]['study_type']]);
  $rows[] = array('File Downloads', implode('<br>', $files));
  $rows[] = array('Population Size', $state['stats']['tree_count']);
  if (!empty($state['stats']['phenotype_count'])) {
    $rows[] = array('Phenotype Count', $state['stats']['phenotype_count']);
  }
  if (!empty($state['stats']['unique_phenotypes'])) {
    $rows[] = array('Unique Phenotypes', $state['stats']['unique_phenotypes']);
  }
  // TODO.
  //$rows[] = array('Genotype Count', $state['stats']['genotype_count']);
  
  $vars = array(
    'header' => array(),
    'rows' => $rows,
    'attributes' => array('class' => array('view'), 'id' => 'tpps_table_display'),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );
  $table = theme_table($vars);
  $output = "<div id=\"study_details\" class=\"tab-pane fade\">$table</div>";
  return $output;
}

/**
 * 
 */
function tpps_details_trees(&$state) {
  $rows = array();

  if (empty($state['tree_info'])) {
    $output = t('The tree information for this study has not been properly updated. Please contact your site administrator.');
    return $output;
  }

  foreach ($state['tree_info'] as $name => $info) {
    $id = $info['organism_id'];
    $form_number = array_search($id, $state['ids']['organism_ids']);
    $species_name = $state['saved_values'][TPPS_PAGE_1]['organism'][$form_number];
    
    $rows[] = array(
      tpps_entity_link($info['stock_id'], $name, 'Stock'),
      tpps_entity_link($id, $species_name, 'Organism'),
      $info['lat'],
      $info['lng'],
    );
  }

  $vars = array(
    'header' => array(
      'Identifier',
      'Species',
      'Latitude',
      'Longitude',
    ),
    'rows' => $rows,
    'attributes' => array('class' => array('view'), 'id' => 'tpps_table_display'),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );
  $table = theme_table($vars);
  $output = "<div id=\"trees\" class=\"tab-pane fade\">$table</div>";
  return $output;
}

/**
 * 
 */
function tpps_details_phenotype(&$state) {
  $rows = array();
  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->fields('p', array('phenotype_id', 'name', 'attr_id', 'observable_id', 'value'));
  $query->fields('s', array('uniquename', 'stock_id'));
  $query->condition('ps.project_id', $state['ids']['project_id']);
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
      $obs,
      $attr,
      $result->value,
    );
  }

  $vars = array(
    'header' => array(
      'Tree',
      'Name',
      'Entity',
      'Attribute',
      'Value'
    ),
    'rows' => $rows,
    'attributes' => array('class' => array('view'), 'id' => 'tpps_table_display'),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );
  $table = theme_table($vars);
  $output = "<div id=\"phenotype\" class=\"tab-pane fade\">$table</div>";
  return $output;
}

/**
 * 
 */
function tpps_details_genotype(&$state) {
  // TODO.
  $table = "Coming Soon!";
  $output = "<div id=\"genotype\" class=\"tab-pane fade\">$table</div>";
  return $output;
}

/**
 * 
 */
function tpps_details_environment(&$state) {
  // TODO.
  $table = "Coming Soon!";
  $output = "<div id=\"environment\" class=\"tab-pane fade\">$table</div>";
  return $output;
}
