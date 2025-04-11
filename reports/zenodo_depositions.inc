<?php

/**
 * @file
 * Shows list of Zenodo Depositions.
 *
 * ZenodoDeposit::list().
 *
 * Note:
 * 1. There could be ZenodoDeposit without TPPS submission and vice versa.
 * 2. The same account used for live- and dev-sites.
 *
 * See
 * - /src/zenodo/ZenodoDeposit.class.inc.
 * - https://developers.zenodo.org/?shell#depositions.
 */

/**
 * Menu callback. Shows list of submissions which has Zenodo Depositions.
 *
 * Returns rendered page content.
 *
 * @TODO Cache Zenodo Responses. Should we?
 */
function tpps_admin_zenodo_depositions_report() {
  $host = arg(4);
  $entity = arg(5);
  $output = '';
  $path = 'admin/reports/tpps/zenodo-depositions/';
  if (empty($entity)) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // All depostions.
    ZenodoHost::printStatus();
    $host_list = ['live', 'sandbox'];
    if (!empty($host) && in_array($host, $host_list)) {
      $host_list = [$host];
    }
    foreach ($host_list as $host) {
      ZenodoHost::set($host);
      $output .= '<h2>Zenodo Host: ' . l($host, $path . $host) . '</h2>';
      $response = ZenodoDeposit::list([]);
      if (empty($response)) {
        drupal_set_message(t('Empty Zenodo API response.'), 'error');
        continue;
      }
      elseif (!empty($response['status'])) {
        drupal_set_message(t('Zenodo API Response: @code - @message', [
          '@code' => $response['status'],
          '@message' => $response['message'],
        ]), 'error');
        continue;
      }
      else {
        $output .= ZenodoDeposit::printDepositionList($response, 'table');
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  else {
    // Single deposition.
    ZenodoHost::set(($host ?? 'sandbox'));
    ZenodoHost::printStatus();
    if (strpos('TGDR', $entity) !== FALSE) {
      // Accession number.
      $submission = new Submission($entity);
      $deposit_id = $submission->getZenodoDepositId();
    }
    elseif (strpos('zenodo', arg(5)) !== FALSE) {
      // This is Deposit Id.
      // Example: 10.5281/zenodo.15011388.
      $doi = arg(4) . '/' . arg(5);
      // @TODO Minor. Get deposit_id by DOI.
    }
    elseif (is_numeric($entity) && ((int) $entity) == $entity) {
      // Deposit Id.
      $deposit_id = $entity;
    }

    if (empty($deposit_id)) {
      drupal_set_message(t('Empty Zenodo Deposit Id'), 'error');
    }
    else {
      $response = ZenodoDeposit::retrieve(['deposit_id' => $deposit_id]);
      if (empty($response)) {
        drupal_set_message(t('Empty Zenodo API response.'), 'error');
      }
      elseif (!empty($response['status'])) {
        drupal_set_message(t('Zenodo API Response: @code - @message', [
          '@code' => $response['status'],
          '@message' => $response['message'],
        ]), 'error');
      }
      else {
        $output = ZenodoDeposit::printDeposition($response, 'table');
        // Show also 'files', 'metadata' and 'links' items.
        $output .= tpps_array_print_list($response);
      }
    }
  }
  return $output;
}
