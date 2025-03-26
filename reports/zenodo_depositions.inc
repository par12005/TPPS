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
 */
function tpps_admin_zenodo_depositions_report() {
  $entity = arg(4);
  $output = '';
  if (empty($entity)) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Show all items.
    drupal_set_message(t('Zenodo Host: @host_type',
      ['@host_type' => (variable_get('tpps_zenodo_debug_mode') ? 'Sandbox' : 'Live')]
    ), 'warning');
    $response = ZenodoDeposit::list([]);
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
      return ZenodoDeposit::printDepositionList($response, 'table');
    }
  }
  else {
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
