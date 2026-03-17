<?php

/**
 * @file
 * Shows publication data scraped from all sources.
 */

/**
 * Menu callback. Shows DOI publidation data scraped from PubMed.
 */
function tpps_report_doi_scraped_all() {
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $rows = db_select(PublicationDOI::TABLE, 't')
    ->fields('t', [])
    ->execute()
    ->fetchAllAssoc('doi', PDO::FETCH_ASSOC);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $name = 'doi_scraped_all';
  //$page_title = variable_get('tpps_report_' . $name . '_title', 'DOI Duplicates');
  //drupal_set_title($page_title);
  $header = [
    t('DOI'),
    t('Authors'),
    t('Date'),
    t('Title'),
    t('Jornal'),
    t('Abstract'),
    t('Source'),
  ];
  return Report::printTable([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No data'),
  ]);
}
