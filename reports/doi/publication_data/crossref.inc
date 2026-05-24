<?php

/**
 * @file
 * Shows publication data obtained from CrossRef API.
 */

/**
 * Menu callback. Shows DOI publidation data from CrossRef.
 */
function tpps_report_doi_publication_data_crossref() {
  return drupal_get_form('tpps_report_doi_publication_data_crossref_form');
}

/**
 * Bulds form 'tpps_report_doi_publication_data_crossref'.
 *
 * Form shows DOI publication data in tables under fieldset.
 */
function tpps_report_doi_publication_data_crossref_form(array $form, array &$form_state) {
  $report = [];
  $source = CrossRef::KEY;
  $result = db_select(PublicationDOI::TABLE, 't')
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
