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
  $form = drupal_get_form('tpps_details_search');
  tpps_add_css_js('main', $form);
  $output .= render($form);
  $output .= tpps_details_tags_sort();
  module_load_include('inc', 'tpps', 'pages/details/top');
  $output .= "<div id='tpps-details-table'>" . tpps_details_top() . "</div>";
  return $output;
}
