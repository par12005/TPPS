<?php

/**
 * @file
 * Shows publication data scraped from all sources.
 */

/**
 * Menu callback. Shows DOI publidation data scraped from PubMed.
 */
function tpps_report_doi_scraped_all() {
  return drupal_get_form('tpps_report_doi_all_form');
}

/**
 * Bulds form 'tpps_report_doi_scraped_all'.
 *
 * Form shows DOI publication data in tables under fieldset.
 */
function tpps_report_doi_all_form(array $form, array &$form_state) {
  $report = [];

  $result = db_select(PublicationDOI::TABLE, 't')
    ->fields('t', [])
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);
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
