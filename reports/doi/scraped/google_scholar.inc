<?php

/**
 * @file
 * Shows publication data scraped from Gooble Scholar web-site.
 */

/**
 * Menu callback. Shows DOI publidation data scraped from Google Scholar.
 */
function tpps_report_doi_scraped_google_scholar() {
  return drupal_get_form('tpps_report_doi_scraped_google_scholar_form');
}

/**
 * Bulds form 'tpps_report_doi_scraped_google_scholar'.
 *
 * Form shows DOI publication data in tables under fieldset.
 */
function tpps_report_doi_scraped_google_scholar_form(array $form, array &$form_state) {
  $report = [];

  $source = PublicationDOI::SOURCE_GS;
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
