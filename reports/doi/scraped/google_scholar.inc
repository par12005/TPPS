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
  $form = [];
  $source = PublicationDOI::SOURCE_GS;
  $data = db_select(PublicationDOI::TABLE, 't')
    ->fields('t', [])
    ->condition('source', $source)
    ->execute()
    ->fetchAllAssoc('doi', PDO::FETCH_ASSOC);
  if (empty($data)) {
    $form['info'] = [
      '#markup' => t('No data'),
    ];
  }
  else {
    foreach ($data as $item) {
      $form[$item['doi']] = [
        '#type' => 'fieldset',
        '#title' => $item['doi'],
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        // Table.
        'table' => ['#markup' => TppsArray::printTable($item)],
      ];
    }
  }
  return $form;
}
