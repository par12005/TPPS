<?php

/**
 * @file
 * Contans theme-functions related to DOI.
 */

/**
 * Builds hyperlink for DOI.
 *
 * @param array $variables
 *   Data for theming.
 *   'doi' string
 *     Required. DOI (in short format).
 *
 * @return string
 *   Returns rendered HTML for DOI hyperlink.
 */
function theme_doi_link(array $variables) {
  $doi = $variables['doi'] ?? NULL;
  $output = '';
  if (!empty($doi)) {
    $output = l(
      $doi,
      'https://doi.org/' . $doi,
      [
        'attributes' => [
          'class' => ['tpps_doi_link'],
          'target' => '_blank',
        ],
      ]
    );
  }
  return $output;
}
