<?php

/**
 * @file
 * Theme hooks of the TPPS module.
 */

/**
 * Implements hook_theme().
 */
function tpps_theme() {
  return [
    'fcbold' => [
      'variables' => [
        'string' => NULL,
      ],
    ],
    'submission_accession_link' => [
      'variables' => [
        'accession' => NULL,
        'path' => NULL,
        'use_abr' => NULL,
      ],
    ],
    'submission_tag_list' => [
      'variables' => [
        'tags' => NULL,
      ],
    ],
    'submission_date' => [
      'variables' => [
        'date' => NULL,
      ],
    ],
    'submission_submitter' => [
      'variables' => [
        'submitter' => NULL,
      ],
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Zenodo.
    'zenodo_deposit_link' => [
      'variables' => [
        'deposit_id' => NULL,
        'host' => NULL,
      ],
    ],
    'zenodo_concept_record_link' => [
      'variables' => [
        'conceptrecid' => NULL,
      ],
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Files.
    'file_id' => [
      'variables' => [
        'fid' => NULL,
        'attributes' => NULL,
      ],
    ],
    'file_action_list' => [
      'variables' => [
        'fid' => NULL,
        'attributes' => NULL,
      ],
    ],
  ];
}
