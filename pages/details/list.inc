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
  $form['details_type'] = [
    '#type' => 'select',
    '#options' => [
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
    ],
    '#ajax' => [
      'wrapper' => 'tpps-details-form',
      'callback' => 'tpps_details_form_callback',
    ],
    '#prefix' => '<div id="tpps-details-form">',
    '#default_value' => $params['type'] ?? NULL,
  ];
  $ops = [
    '~*' => '~*',
    'LIKE' => 'LIKE',
    '=' => '=',
  ];
  $form['details_op'] = [
    '#type' => 'select',
    '#options' => $ops,
    '#default_value' => $params['op'] ?? NULL,
  ];
  $form['details_value'] = [
    '#type' => 'textfield',
    '#suffix' => '</div>',
    '#autocomplete_path' => 'tpps/autocomplete/project_title',
    '#default_value' => $params['value'] ?? NULL,
  ];
  if (empty($form_state['values']['details_type'])) {
    $form_state['values']['details_type'] = $params['type'] ?? NULL;
  }
  if (!empty($form_state['values']['details_type'])) {
    $form['details_value']['#autocomplete_path'] =
      tpps_details_get_autocomplete_path($form_state['values']['details_type']);
  }
  $form['details_search'] = [
    '#type' => 'button',
    '#button_type' => 'button',
    '#value' => t('Search'),
  ];
  $form['reset_button'] = [
    '#markup' => '<button class="btn btn-primary form-button" type="reset"/>Reset</button>',
  ];
  $form['#attributes'] = [
    'style' => 'text-align:center',
  ];
  return $form;
}

/**
 * Gets list of all tags.
 *
 * @return string
 *   Returns rendered list of tags. Tag options as rendered HTML.
 */
function tpps_details_tags_sort() {
  return '<label for="tpps-tags-filter">' . t('Sort by tags:')
    . '</label><div id="tpps-tags-filter">'
    . theme('submission_tag_list', ['tags' => SubmissionTag::load()])
    . '</div>';
}

/**
 * Gets autocomplete path for given details type.
 *
 * @param string $type
 *   Details type.
 *
 * @return string
 *   Returns relative autocomplete path.
 */
function tpps_details_get_autocomplete_path($type) {
  $list = [
    'title' => 'tpps/autocomplete/project_title',
    'species' => 'tpps/autocomplete/species',
    'accession' => 'tpps/autocomplete/project_accession',
    'author' => 'tpps/autocomplete/author',
    'phenotype_name' => 'tpps/autocomplete/phenotype',
    'phenotype_ontology' => 'tpps/autocomplete/phenotype_ontology',
    'genotype_name' => 'tpps/autocomplete/genotype',
    'genotype_marker' => 'tpps/autocomplete/genotype_marker'
  ];
  return $list[$type] ?? NULL;
}
