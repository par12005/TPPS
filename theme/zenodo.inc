<?php

/**
 * @file
 * Zenodo related theme-functions.
 */

/**
 * Creates hyperlink to local Zenodo Deposit page.
 *
 * Usage: theme('zenodo_deposit_link', $vars);
 *
 * Usage: theme('submission_zenodo_deposit_link', $vars);
 * Note: Deposits withour host will be highlighted with red color.
 */
function theme_zenodo_deposit_link(array $variables) {
  if (empty($deposit_id = $variables['deposit_id'])) {
    return '';
  }

  // Use ZenodoHost::search(['deposit_id' => $deposit_id]); to get host.
  $host = $variables['host'] ?? NULL;
  // Build link.
  if ($host) {
    $url = 'admin/reports/tpps/zenodo/deposition/' . $host . '/' . $deposit_id;
    return l($deposit_id, $url);
  }
  else {
    return '<div style="color:red;">' . $deposit_id . '</div>';
  }
}

/**
 * Gets Hyperlink to Zenodo API Endpoint which shows all versions.
 *
 * Usage: theme('zenodo_concept_record_link', $vars);
 *
 * @return string
 *   Returns HTML A-tag which points to Zenodo Deposit List page where listed
 *   all versions of the Deposition.
 */
function theme_zenodo_concept_record_link($variables) {
  if (
    empty($conceptrecid = $variables['conceptrecid'])
    || empty($host = $variables['host'])
  ) {
    return '';
  }
  // Docs: https://developers.zenodo.org/?javascript#list
  $query = [
    'q' => 'conceptrecid:' . $conceptrecid,
    'all_versions' => 1,
    // optional. Sort order (bestmatch or mostrecent).
    // Prefix with minus to change form ascending to descending (e.g. -mostrecent).
    'sort' => 'mostrecent',
  ];
  // The same API Endpoint uses ZenodoDeposit::list().
  $url = 'https://' . ($host == 'live' ? '' : 'sandbox.')
    . 'zenodo.org/api/deposit/depositions';
  return l($conceptrecid, Zenodo::addAccessToken($url), ['query' => $query]);
}
