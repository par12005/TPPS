<?php

/**
 * @file
 * Shows DOI publication data from all the sources.
 */

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
    // Build 2 level structure for report.
    foreach ($result as $item) {
      $doi = $item['doi'];
      $source = $item['source'];
      unset($item['doi']);
      unset($item['source']);
      $report[$doi][$source] = $item;
    }
    // Add 'combined' column.
    foreach ($result as $item) {
      $data = PublicationDOI::load($item['doi']);
      $column_title = 'combined';
      if (empty($data) || count($data) !== count(array_filter($data))) {
        $column_title .= ' [ not complete ]';
      }
      else {
        $column_title .= ' [ complete ]';
      }
      unset($data['doi']);
      $report[$item['doi']][$column_title] = $data;
    }
  }
  Report::print2Level($form, $report);
  return $form;
}
