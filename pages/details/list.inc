<?php

/**
 * @file
 * TPPS Study details page.
 *
 * URL: /tpps/details.
 */

/**
 * Menu callback. Shows list of stydies.
 *
 * @return string
 *   The appropriate output.
 */
function tpps_details_list() {
  global $base_url;
  global $user;
  $output = "";

  // @TODO Cache those numbers.
  $query = db_select('chado.plusgeno_view', 'pg');
  $query->addExpression('count(distinct(accession))', 'count');
  $query = $query->execute();
  $num_studies = $query->fetchObject()->count;

  $query = db_select('chado.plusgeno_view', 'pg');
  $query->addExpression('count(distinct(species))', 'count');
  $query = $query->execute();
  $num_species = $query->fetchObject()->count;

  // @TODO Minor. Replace with l().
  $tpps_link = "<b><a target=\"blank\" href=\"/tpps\">submit your study</a></b>";
  $output .= "This page reflects community submissions associated with published studies. Each submission is associated with a longterm identifier that connects the genotype, phenotype, and/or environmental metrics assessed. These studies represent diversity assessements, association mapping, and landscape genomics. We accept studies with a variety of marker types. We encourage you to $tpps_link to TreeGenes and utilize the provided accession in your publication. We currently have <b>$num_studies</b> studies representing <b>$num_species</b> species.";
  if (module_exists('cartogratree')) {
    $output .= " Studies that have georeferenced accessions associated with them can be queried on <b><a href=\"cartogratree\">CartograPlant.</a></b>";
  }
  // @todo Use 'drupal_get_form' as a 'page_callback' in hook_menu instead of
  // rendering form manually.
  $form = drupal_get_form('tpps_details_search');
  tpps_add_css_js('main', $form);
  $output .= render($form);
  $output .= tpps_details_tags_sort();
  module_load_include('inc', 'tpps', 'pages/details/top');
  $output .= "<div id='tpps-details-table'>" . tpps_details_top() . "</div>";
  return $output;
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
      'title' => t('Title'),
      'species' => t('Species'),
      'accession' => t('Accession'),
      'author' => t('Primary Author'),
      'year' => t('Publication Year'),
      'phenotype_name' => t('Phenotype Name'),
      'phenotype_ontology' => t('Phenotype Ontology Name'),
      'genotype_name' => t('Genotype Name'),
      'genotype_marker' => t('Genotype Marker Type'),
      'tags' => t('Tags'),
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

  $form['reset_button'] = array(
    '#markup' => '<button class="btn btn-primary form-button" type="reset"/>Reset</button>',
  );

  $form['#attributes'] = array(
    'style' => 'text-align:center',
  );

  return $form;
}

/**
 * Gets list af all tags.
 *
 * @return string
 *   Returns rendered list of tags. Tag options as rendered HTML.
 */
function tpps_details_tags_sort() {
  $query = db_select('tpps_tag', 't');
  $query->addField('t', 'tpps_tag_id', 'id');
  $query->addField('t', 'name', 'name');
  $query->addField('t', 'color', 'color');
  $query->addField('t', 'static', 'static');
  $query->orderBy('static', 'DESC')->orderBy('id');
  $tags = $query->execute()->fetchAllAssoc('id', PDO::FETCH_ASSOC);
  return '<label for="tpps-tags-filter">' . t('Sort by tags:')
    . '</label><div id="tpps-tags-filter">' . (tpps_show_tags($tags) ?? '')
    . '</div>';
}
