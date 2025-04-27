<?php

/**
 * @file
 * Shows list of Zenodo Depositions.
 *
 * Path: /admin/reports/tpps/zenodo/deposition
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
function tpps_report_zenodo_deposition() {
  $host = arg(5);
  $entity = arg(6);
  $output = '';

  $path = 'admin/reports/tpps/zenodo/deposition/';
  if (empty($entity)) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // All depositions.
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
    elseif (strpos('zenodo', $host) !== FALSE) {
      // This is Deposit Id.
      // Example: 10.5281/zenodo.15011388.
      $doi = $host . '/' . $entity;
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
      try {
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
      catch (Exception $e) {
        if ($e->getCode() == 404) {
          drupal_set_message($e->getMessage()
            // Suggest to look at other host.
            . '<br />' . l(t('Check this Deposit Id on the other host?'), $path
            . (ZenodoHost::isLive() ? 'sandbox' : 'live') . '/' . $deposit_id),
            'error'
          );
        }
        elseif ($e->getCode() == 403) {
          drupal_set_message($e->getMessage(), 'error');
        }
        else {
          throw new Exception($e->getMessage(), $e->getCode());
        }
      }
    }
  }
  return $output;
}
