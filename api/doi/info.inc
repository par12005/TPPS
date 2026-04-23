<?php

/**
 * @file
 * API Callback. Return's file info in JSON format.
 *
 * URL: 'api/doi/%/%/%'.
 * URL: 'api/doi/$doi1/$doi2/$source'.
 */

/**
 * API Callback. Return's DOI Publication info in JSON format.
 *
 * Note: there is an extra elements:
 * 'path': an absolute path in filesystem.
 * 'url': URL with dynamic token to download file. Managed file and uploaded
 * directly to server (outside of the web-root) could be downloaded.
 *
 * @param string $doi1
 *   First part of the DOI (before '/'). DOI Registrant.
 */
function tpps_api_doi_info($doi1) {

  $doi2 = arg(3);
  $source = arg(4);

  if (empty($doi2)) {
    $list = db_select(PublicationDOI::TABLE, 't')
      ->fields('t', ['doi'])
      ->condition('doi', $doi1 . '/%', 'ILIKE')
      ->groupBy('source')
      ->groupBy('doi')
      ->execute()
      ->fetchCol('doi');
    return $list;
  }
  else {
    $doi = $doi1 . '/' . $doi2;
    if (!empty($source) && in_array($source, ['pubmed', 'google_scholar'])) {
      return PublicationDOI::load($doi, $source);
    }
    return [
      'google_scholar' => PublicationDOI::load($doi, 'google_scholar'),
      'pubmed' => PublicationDOI::load($doi, 'pubmed'),
    ];
  }

}
