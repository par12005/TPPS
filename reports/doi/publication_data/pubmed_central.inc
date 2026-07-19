<?php

/**
 * @file
 * Shows publication data publication_data from PubMedCentral API.
 */

/**
 * Menu callback. Shows DOI publidation data from PubMedCentral.
 */
function tpps_report_doi_publication_data_pubmed_central() {
  return drupal_get_form('tpps_report_doi_publication_data_pubmed_central_form');
}

/**
 * Bulds form 'tpps_report_doi_publication_data_google_scholar'.
 *
 * Form shows DOI publication data in tables under fieldset.
 */
function tpps_report_doi_publication_data_pubmed_central_form(array $form, array &$form_state) {
  $report = [];
  $source = 'PubMedCentral';
  $result = db_select(PublicationDoi::TABLE, 't')
    ->fields('t', [])
    ->condition('source', $source)
    ->execute()
    ->fetchAllAssoc('doi', PDO::FETCH_ASSOC);
  if (!empty($result)) {
    // Build 2 level structure for report.
    foreach ($result as $item) {
      $doi = $item['doi'];
      $source = $item['source'];
      unset($item['doi']);
      unset($item['source']);
      $report[$doi][$source] = $item;
    }
  }
  Report::print2Level($form, $report);
  return $form;
}
