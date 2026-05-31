<?php

/**
 * @file
 * Shows single DOI publication data from all the sources.
 */

/**
 * Menu callback. Shows DOI publidation data publication_data from PubMed.
 */
function tpps_report_doi_publication_data_single($doi1, $doi2) {
  return drupal_get_form('tpps_report_doi_publication_data_single_form', $doi1, $doi2);
}

/**
 * Bulds form 'tpps_report_doi_publication_data_all'.
 *
 * Form shows DOI publication data in tables under fieldset.
 */
function tpps_report_doi_publication_data_single_form(array $form, array &$form_state, $doi1, $doi2) {
  $doi = $doi1 . '/' . $doi2;

  $report = [];
  $result = PublicationDoi::load($doi, 'all');
  if (empty($result)) {
    drupal_set_message(t('No publication data found for DOI: @doi', ['@doi' => $doi]));
    drupal_goto('admin/reports/tpps/doi/publication_data/all');
  }
  if (!empty($result)) {
    $report = tpps_report_doi_publication_data_show_single($result);
  }
  Report::print2Level($form, $report);
  return $form;
}

/**
 * Build data for single DOI report-table.
 *
 * @param array $data
 *   Data for report. Structure:
 *   'source' => publication data.
 *
 * @return array
 *   Returns report-data ready for Report::print2Level().
 */
function tpps_report_doi_publication_data_show_single(array $data) {
  $report = [];
  // Build 2 level structure for report.
  foreach ($data as $item) {
    $doi = $item['doi'];
    $source = $item['source'];
    unset($item['doi']);
    unset($item['source']);
    $report[$doi][$source] = $item;
  }
  // Add 'combined' column.
  foreach ($data as $item) {
    $data = PublicationDOI::load($item['doi']);
    $column_title = 'combined [ '
      . (TppsArray::isComplete($data) ? '' : 'not') . ' complete ]';

    unset($data['doi']);
    $report[$item['doi']][$column_title] = $data;
  }
  return $report;
}
