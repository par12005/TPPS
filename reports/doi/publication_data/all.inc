<?php

/**
 * @file
 * Shows DOI publication data from all the sources.
 */

module_load_include('inc', 'tpps', 'reports/doi/publication_data/single');

/**
 * Menu callback. Shows DOI publidation data publication_data from PubMed.
 */
function tpps_report_doi_publication_data_all() {
  return drupal_get_form('tpps_report_doi_publication_data_all_form');
}

/**
 * Bulds form 'tpps_report_doi_publication_data_all'.
 *
 * Form shows DOI publication data in tables under fieldset.
 */
function tpps_report_doi_publication_data_all_form(array $form, array &$form_state) {
  $report = [];
  $result = db_select(PublicationDOI::TABLE, 't')
    ->fields('t', [])
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);
  if (!empty($result)) {
    $report = tpps_report_doi_publication_data_show_single($result);
  }
  Report::print2Level($form, $report);
  return $form;
}
