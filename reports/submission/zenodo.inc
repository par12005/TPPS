<?php

/**
 * @file
 * Shows list of studies which has Zenodo Depositions.
 *
 * See
 * - /src/zenodo/ZenodoDeposit.class.inc.
 * - https://developers.zenodo.org/?shell#depositions.
 */

/**
 * Menu callback. Shows list of submissions which has Zenodo Depositions.
 */
function tpps_report_submission_zenodo($reset = FALSE) {
  $submission_cache = SubmissionCache::getAll($reset);
  $rows = [];
  $i = 0;
  foreach ($submission_cache as $accession => $item) {
    if (empty($item['zenodo'])) {
      continue;
    }
    // Get new data.
    $deposit_id = $item['zenodo']['deposit_id'] ?? NULL;
    $conceptrecid = $item['zenodo']['conceptrecid'] ?? NULL;
    if (!$deposit_id && !$conceptrecid) {
      continue;
    }
    $host = $item['zenodo']['host'] ?? NULL;
    $rows[] = [
      'number' => ++$i,
      // @TODO Convert to theme-function.
      'accession' => theme('submission_accession_link', [
        'accession' => $accession,
        'path' => 'api/submission/' . $accession . '/view/state',
        'show_version' => TRUE,
      ]),
      'project_id' => $item['project_id'],
      'status' => $item['status'] ?? NULL,
      'host' => ucfirst($host),
      'deposit_id' => theme('zenodo_deposit_link',
        ['deposit_id' => $deposit_id, 'host' => $host]
      ),
      'conceptrecid' => theme('zenodo_concept_record_link',
        ['conceptrecid' => $conceptrecid, 'host' => $host]
      ),
      'publication_doi' => PublicationDoi::print(
        $item['publication']['publication_doi'] ?? ''
      ),
      'dataset_doi' => $item['publication']['dataset_doi'] ?? '',
      'deposition_files' => l(
        t('View Files'), 'tpps/submission/' . $accession . '/zenodo/files'
      ),
    ];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $header = [
    // No sorting.
    ['data' => '#'],
    [
      'data' => t('Accession'),
      'field' => 'accession',
    ],
    [
      'data' => t('Project Id'),
      'field' => 'project_id',
    ],
    [
      'data' => t('Status'),
      'field' => 'status',
    ],
    [
      'data' => t('Zenodo Host'),
      'field' => 'host',
    ],
    [
      'data' => t('Zenodo Deposit Id'),
      'field' => 'deposit_id',
    ],
    [
      'data' => t('Zenodo Concept Record Id'),
      'field' => 'conceptrecid',
    ],
    [
      'data' => t('Publication DOI'),
      'field' => 'publication_doi',
    ],
    [
      'data' => t('Dataset DOI'),
      'field' => 'dataset_doi',
    ],
    // No sorting.
    ['data' => t('Deposition Files')],
  ];
  return Report::print([
    'name' => 'submission_zenodo',
    'header' => $header,
    'rows' => $rows,
    'submission_cache' => TRUE,
  ]);
}
