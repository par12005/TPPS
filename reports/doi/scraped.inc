<?php

/**
 * @file
 * Shows list of the all studies which has Publication DOI.
 */

/**
 * Menu callback. Shows list of organisms per study.
 */
function tpps_report_doi_scraped() {
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $rows = db_select(PublicationDOI::TABLE, 't')
    ->fields('t', [])
    ->execute()
    ->fetchAllAssoc('doi', PDO::FETCH_ASSOC);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $name = 'doi_all';
  //$page_title = variable_get('tpps_report_' . $name . '_title', 'DOI Duplicates');
  //drupal_set_title($page_title);
  $header = [
    t('DOI'),
    t('Authors'),
    t('Date'),
    t('Title'),
    t('Jornal'),
    t('Abstract'),
  ];
  return Report::printTable([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No data'),
  ]);
}
