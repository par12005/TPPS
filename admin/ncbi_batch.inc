<?php

/**
 * @file
 * Gets taxonomy id for existing organisms from NCBI using Batch API.
 */

// Name of the submit button.
define('TPPS_NCBI_BATCH_UPDATE_BUTTON_NAME', 'ncbi_batch_update');

// Delay between requests to NCBI server.
// @TODO Move to the module's settings page.
// WARNING: When there is no delay a lof of requests will fail.
define('TPPS_NCBI_REQUEST_DELAY', 25000);

/**
 * NCBI Taxonomy Id batch update form.
 */
function tpps_ncbi_batch_settings_form(array $form, array &$form_state) {

  // Show results of previous batch (if any).
  // @todo Show better report as a table.
  if (!empty($_SESSION['tpps_ncbi_batch'])) {
    $form['results'] = [
      '#markup' => $_SESSION['tpps_ncbi_batch'] . '<hr/>',
    ];
    // @todo Add button to clear results.
    unset($_SESSION['tpps_ncbi_batch']);
  }

  // Button to start batch process.
  $form[TPPS_NCBI_BATCH_UPDATE_BUTTON_NAME] = [
    '#type' => 'submit',
    '#name' => TPPS_NCBI_BATCH_UPDATE_BUTTON_NAME,
    '#value' => t('NCBI Taxonomy Id Batch Update'),
  ];
  return $form;
}

/**
 * TPPS Cache Settings Form.
 */
function tpps_ncbi_batch_settings_form_submit(array $form, array &$form_state) {
  // Process 'Clear TPPS Cache' button.
  $button_name = $form_state['triggering_element']['#name'];
  if ($button_name == TPPS_NCBI_BATCH_UPDATE_BUTTON_NAME) {
    // Get list of organisms without Taxonomy Id.
    $operations = [];
    $list = tpps_get_organisms_without_taxonomy_id();
    $total = count($list);
    foreach ($list as $organism_id => $organism_name) {
      if (empty($organism_name)) {
        watchdog('tpps', "Organism #id has empty name and NCBI Taxonomy Id "
          . "can't be found for this organism", ['@id' => $organism_id],
          WATCHDOG_ERROR
        );
        continue;
      }
      $operations[] = [
        'tpps_ncbi_batch_callback',
        [$organism_id, $organism_name, $total],
      ];
    }
    $batch = [
      'title' => t('NCBI Taxonomy ID Update'),
      'operations' => $operations,
      'finished' => 'tpps_ncbi_batch_finished_callback',
      'file' => drupal_get_path('module', 'tpps') . '/admin/ncbi_batch.inc',
    ];
    batch_set($batch);

    // There is no need to explicitly call batch_process() because this is
    // *_submit() function.
  }
}

/**
 * Batch callback. Updates NCBI Taxonomy Id for a single organism.
 *
 * @param int $organism_id
 *   Orgaanism Id. See chado.orgnism.organism_id.
 * @param string $organism_name
 *   Full organism name. Space separated 'genus' and 'species'.
 * @param int $total
 *   Total amount of organisms to be processed.
 * @param array $context
 *   Batch context.
 */
function tpps_ncbi_batch_callback($organism_id, $organism_name, $total, array &$context) {
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_item'] = '';
    $context['sandbox']['total'] = $total;
  }
  // Delay to avoid failed requests.
  if (!empty(TPPS_NCBI_REQUEST_DELAY)) {
    usleep(TPPS_NCBI_REQUEST_DELAY ?? 200000);
  }
  // Request NCBI.
  $organism = tpps_organism($organism_id)->requestNcbiTaxonomyId();
  if (!empty($organism->ncbiTaxonomyId)) {
    $organism->saveTaxonomy();
  }

  $label = t('NCBI Taxonomy Id for "@organism_name" (#@organism_id): @taxonomy_id', [
    '@organism_id'  => $organism_id,
    '@organism_name' => $organism_name,
    '@taxonomy_id' => $organism->ncbiTaxonomyId ?? t('Not found'),
  ]);
  $context['results'][] = $label;
  $context['sandbox']['progress']++;
  $context['sandbox']['count'] = $context['sandbox']['total'] - $context['sandbox']['progress'];
  $context['sandbox']['current_item'] = $organism_name;
  $context['message'] = $label;
}

/**
 * Batch callback. Shows summary of batch process or error message.
 *
 * @param mixed $success
 * @param mixed $results
 * @param mixed $operations
 */
function tpps_ncbi_batch_finished_callback($success, $results, $operations) {
  // The 'success' parameter means no fatal PHP errors were detected. All
  // other error management should be handled using 'results'.
  if ($success) {
    $message = format_plural(count($results), 'One item processed.', '@count items processed.');
    $_SESSION['tpps_ncbi_batch'] = theme('item_list', ['items' => $results]);
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
}
