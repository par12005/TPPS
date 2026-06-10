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
    'http_code' => [
      'variables' => [
        'code' => NULL,
      ],
      'file' => 'theme/misc.inc',
    ],
    'http_method' => [
      'variables' => [
        'method' => NULL,
      ],
      'file' => 'theme/misc.inc',
    ],
    'response_body' => [
      'variables' => [
        'title' => NULL,
        'body' => NULL,
        'content_type' => NULL,
      ],
      'file' => 'theme/misc.inc',
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    'fcbold' => [
      'variables' => [
        'string' => NULL,
      ],
    ],
    // DOI.
    'doi_link' => [
      'variables' => [
        'doi' => NULL,
      ],
      'file' => 'theme/doi.inc',
    ],
    // Submission.
    'submission_accession_link' => [
      'variables' => [
        'accession' => NULL,
        'path' => NULL,
        'use_abr' => NULL,
      ],
      'file' => 'theme/submission.inc',
    ],
    'submission_tag_list' => [
      'variables' => [
        'tags' => NULL,
      ],
      'file' => 'theme/submission.inc',
    ],
    'submission_date' => [
      'variables' => [
        'date' => NULL,
      ],
      'file' => 'theme/submission.inc',
    ],
    'submission_submitter' => [
      'variables' => [
        'submitter' => NULL,
      ],
      'file' => 'theme/submission.inc',
    ],
    'submission_actions' => [
      'variables' => [
        'accession' => NULL,
        'deposit_id' => NULL,
        'edit_publication_information' => NULL,
      ],
      'file' => 'theme/submission.inc',
    ],
    'submission_pages' => [
      'variables' => [
        'accession' => NULL,
      ],
      'file' => 'theme/submission.inc',
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Zenodo.
    'zenodo_deposit_link' => [
      'variables' => [
        'deposit_id' => NULL,
        'host' => NULL,
      ],
      'file' => 'theme/zenodo.inc',
    ],
    'zenodo_concept_record_link' => [
      'variables' => [
        'conceptrecid' => NULL,
      ],
      'file' => 'theme/zenodo.inc',
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Files.
    'file_id' => [
      'variables' => [
        'fid' => NULL,
        'attributes' => NULL,
      ],
      'file' => 'theme/file.inc',
    ],
    'file_size' => [
      'variables' => [
        'size' => NULL,
      ],
      'file' => 'theme/file.inc',
    ],
    'file_local_vcf' => [
      'variables' => [
        'path' => NULL,
        'accession' => NULL,
      ],
      'file' => 'theme/file.inc',
    ],
    'file_action_list' => [
      'variables' => [
        'fid' => NULL,
        'attributes' => NULL,
      ],
      'file' => 'theme/file.inc',
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Submission Flatten.
    'submission_page' => [
      'variables' => [
        'form_state' => NULL,
        'options' => NULL,
      ],
      'file' => 'theme/submission_flatten.inc',
    ],
    'submission_block' => [
      'variables' => [
        'data' => NULL,
      ],
      'file' => 'theme/submission_flatten.inc',
    ],
  ];
}

/**
 * Makes first char of the string bold.
 *
 * Note: Used in function tpps_report_submission_imported().
 * Usage:
 * theme('fcbold', ['string' => $string]);
 *
 * @param array $variables
 *   Data for theming.
 *   'string' - String to be processed.
 *
 * @return string
 *   Returns string where 1st char is wrapped with '<strong>' HTML-tag.
 */
function theme_fcbold(array $variables) {
  $string = $variables['string'] ?? NULL;
  if (!empty($string)) {
    return str_replace($string[0], '<strong>' . $string[0] . '</strong>', $string);
  }
}
