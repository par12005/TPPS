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
 */
function tpps_admin_zenodo_depositions_report() {
  $host = arg(4);
  $entity = arg(5);
  $output = '';
  if (empty($entity)) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Show all items.
    ZenodoDeposit::printHostStatus();
    $host_list = ['live', 'sandbox'];
    if (!empty($host) && in_array($host, $host_list)) {
      $host_list = [$host];
    }
    $output = '';
    foreach ($host_list as $host) {
      ZenodoDeposit::setHost($host);
      $output .= '<h2>Zenodo Host: ' . $host . '</h2>';
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
    return $output;
  }
  else {
    ZenodoDeposit::setHost(($host ?? 'sandbox'));
    ZenodoDeposit::printHostStatus();
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Show single entity report.
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
        return '';
      }
      elseif (!empty($response['status'])) {
        drupal_set_message(t('Zenodo API Response: @code - @message', [
          '@code' => $response['status'],
          '@message' => $response['message'],
        ]), 'error');
        return '';
      }
      else {
        return ZenodoDeposit::printDeposition($response, 'table');
      }
    }
  }
}
