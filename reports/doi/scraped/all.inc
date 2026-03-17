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
  $form = [];
  $data = db_select(PublicationDOI::TABLE, 't')
    ->fields('t', [])
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);

  if (empty($data)) {
    $form['info'] = [
      '#markup' => t('No data'),
    ];
  }
  else {
    $report = [];
    foreach ($data as $item) {
      $doi = $item['doi'];
      $source = $item['source'];
      unset($item['doi']);
      unset($item['source']);
      $report[$doi][$source] = $item;
    }
//dpm($report);

    foreach ($report as $doi => $data) {
      $header = [' '];
      $rows = [];
      foreach ($data as $source => $item) {
        $header[] = $source;
        foreach ($item as $key => $value) {
          if (empty($rows[$key])) {
            $rows[$key][] = '<strong>' . $key . '</strong>';
          }
          $rows[$key][] = $value;
        }
      }
      $form[$doi] = [
        '#type' => 'fieldset',
        '#title' => $doi,
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ];
      $form[$doi][$source] = [

        '#markup' => theme('table', [
          'header' => $header,
          'rows' => $rows,
          'empty' => t('No data'),
        ]),
      ];
    }
  }
  return $form;
}
