<?php

/**
 * @file
 * Gets taxonomy id for existing organisms from NCBI using Batch API.
 *
 * Admnin theme was enabled for this page in tpps_admin_paths().
 */

// Name of the update button.
// Only organisms without taxonomy ids will be processed.
define('TPPS_NCBI_BATCH_UPDATE_BUTTON_NAME', 'ncbi_batch_update');

// Name of the button 'Recreate'.
// All existing taxonomy ids will be removed, list of the accepted synonym
// cleared and taxonomy ids requested again.
define('TPPS_NCBI_BATCH_RECREATE_BUTTON_NAME', 'ncbi_batch_recreate');

/**
 * Menu callback for 'batch/ncbi/taxonomy_id'.
 *
 * Form 'tpps_batch_ncbi_taxonomy_id_form'.
 */
function tpps_batch_ncbi_taxonomy_id_form(array $form, array &$form_state) {
  $source = NcbiTaxonomy::KEY;
  return tpps_batch_common_taxonomy_id_form($source, $form, $form_state);
}

/**
 * Common form 'tpps_batch_N_taxonomy_id_form' builder.
 *
 * @param mixed $source
 *   Source of the taxonomy data.
 * @param array $form
 *   Drupal form array.
 * @param array $form_state
 *   Drupal form state array.
 *
 * @return array
 *   Returns drupal form array.
 */
function tpps_batch_common_taxonomy_id_form($source, array &$form, array &$form_state) {
  global $_SESSION;
  $source_title = strtoupper($source);

  $session_name = 'tpps_batch_' . $source . '_taxonomy_id';
  // Show results of previous batch (if any).
  // @todo Show better report as a table.
  if (!empty($_SESSION[$session_name])) {
    $form['results'] = [
      '#markup' => $_SESSION[$session_name] . '<hr/>',
    ];
    // @todo Add button to clear results.
    unset($_SESSION[$session_name]);
  }

  $report_path = 'admin/reports/tpps/organism/' . $source . '/all';
  $form['report'] = [
    '#markup' => '<p>' . l(
      t('List of the @source Taxonomy Ids.', ['@source' => $source_title]),
      $report_path
    ) . '</p>',
  ];

  // Buttons.
  $update_button = constant(
    'TPPS_' . $source_title . '_BATCH_UPDATE_BUTTON_NAME'
  );
  // Button to start batch process.
  // Update only organisms which are missing taxonomy ids.
  $form[$update_button] = [
    '#type' => 'submit',
    '#name' => $update_button,
    '#value' => t('@source Taxonomy Id Batch Update', ['@source' => $source_title]),
  ];
  $recreate_button = constant(
    'TPPS_' . $source_title . '_BATCH_RECREATE_BUTTON_NAME'
  );
  // Reset all organism taxonomy ids.
  $form[$recreate_button] = [
    '#type' => 'submit',
    '#name' => $recreate_button,
    '#value' => t('@source Taxonomy Id Batch Reset', ['@source' => $source_title]),
  ];
  return $form;
}

/**
 * Form submitter for 'tpps_batch_ncbi_taxonomy_id_form'.
 */
function tpps_batch_ncbi_taxonomy_id_form_submit(array $form, array &$form_state) {
  $source = NcbiTaxonomy::KEY;
  tpps_batch_common_taxonomy_id_form_submit($source, $form, $form_state);
}

/**
 * Common 'tpps_batch_N_taxonomy_id_form_submit'.
 *
 * @param mixed $source
 *   Source of the taxonomy data.
 * @param array $form
 *   Drupal form array.
 * @param array $form_state
 *   Drupal form state array.
 */
function tpps_batch_common_taxonomy_id_form_submit($source, array $form, array &$form_state) {
  $source_title = strtoupper($source);
  $button_name = $form_state['triggering_element']['#name'];
  $module_path = drupal_get_path('module', 'tpps');

  $recreate_button = constant(
    'TPPS_' . $source_title . '_BATCH_RECREATE_BUTTON_NAME'
  );
  $update_button = constant(
    'TPPS_' . $source_title . '_BATCH_UPDATE_BUTTON_NAME'
  );

  switch ($button_name) {
    case $recreate_button:
      OrganismTaxonomy::clearAcceptedSynonymList($source);
      OrganismTaxonomy::remove($source);

    case $update_button:
      // Get list of organisms without Taxonomy Id.
      $operations = [];
      $debug_mode = FALSE;
      $list = OrganismTaxonomy::getMissingOrganismIdList($source, $debug_mode);
      if (!empty($total = count($list))) {
        foreach ($list as $organism_id) {
          $operations[] = [
            'tpps_batch_' . $source . '_taxonomy_id_callback',
            [$organism_id, $total],
          ];
        }
        $batch = [
          'title' => $source_title . ' Taxonomy ID Update',
          'operations' => $operations,
          'finished' => 'tpps_batch_' . $source . '_taxonomy_id_finished_callback',
          'file' => $module_path . '/batch/' . $source . '/taxonomy_id.inc',
        ];
        batch_set($batch);
      }
      else {
        $message = 'No organisms without ' . $source_title . ' Taxonomy Id found.';
        drupal_set_message($message);
      }
      // There is no need to explicitly call batch_process() because this is
      // *_submit() function.
      break;
  }
}

/**
 * Batch callback. Updates NCBI Taxonomy Id for a single organism.
 *
 * @param int $organism_id
 *   Orgaanism Id. See chado.orgnism.organism_id.
 * @param int $total
 *   Total amount of organisms to be processed.
 * @param array $context
 *   Batch context.
 */
function tpps_batch_ncbi_taxonomy_id_callback($organism_id, $total, array &$context) {
  tpps_batch_common_taxonomy_id_callback(
    NcbiTaxonomy::KEY,
    $organism_id,
    $total,
    $context
  );
}

/**
 * Common 'batch_taxonomy_id_callback'.
 *
 * @param mixed $source
 *   Taxonomy id source. Possible values: 'ncbi' and 'gbif'.
 * @param int $organism_id
 *   Orgaanism Id. See chado.orgnism.organism_id.
 * @param int $total
 *   Total amount of organisms to be processed.
 * @param array $context
 *   Batch context.
 */
function tpps_batch_common_taxonomy_id_callback($source, $organism_id, $total, array &$context) {
  if (empty($class = OrganismTaxonomy::SOURCE_LIST[$source])) {
    $context['finished'] = 1;
    $context['success'] = FALSE;
    // @TODO  Report an error.
    return;
  }
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_item'] = '';
    $context['sandbox']['total'] = $total;
  }
  // Get Taxonomy Id.
  $taxonomy_id = NULL;
  $primary_id = NULL;
  if (!empty($taxonomy = $class::getId($organism_id))) {
    $taxonomy_id = $taxonomy['taxonomy_id'];
    $primary_id = $taxonomy['primary_id'];
  }
  // Build report.
  try {
    $organism_name = tpps_organism($organism_id)->name;
  }
  catch (Exception $e) {
    $organism_name = '';
  }
  $primary_name = $taxonomy['primary_name'] ?? '';

  if (empty($taxonomy_id)) {
    $taxonomy_id = t('<span style="color:red;">Not found</span');
    $organism_name = '<span style="color: red;">' . $organism_name . '</span>';
  }
  elseif (
    $primary_name == $organism_name
    || $taxonomy['primary_id'] == $organism_id
  ) {
    $organism_name = '<span style="color: green;">' . $organism_name . '</span>';
  }
  elseif (empty($taxonomy['primary_id'])) {
    $primary_name = '<span style="color: red;">' . $primary_name . '</span>';
    $taxonomy_id = l($taxonomy_id, $class::getUrl($taxonomy_id));
  }
  elseif (is_numeric($taxonomy['primary_id'])) {
    $organism_name = '<span style="color: blue;">' . $organism_name . '</span>';
  }
  else {
    $organism_name = '<span style="color: red;">' . $organism_name . '</span>';
  }

  $row = [
    'organism_id'  => $organism_id,
    'organism_name'  => $organism_name,
    'taxonomy_id' => $taxonomy_id,
    'primary_name'  => $primary_name,
    'primary_id'  => $primary_id,
  ];
  $context['results'][] = $row;
  $context['message'] = implode(', ', $row);
  $context['sandbox']['progress']++;
  $context['sandbox']['count'] = $context['sandbox']['total'] - $context['sandbox']['progress'];
  $context['sandbox']['current_item'] = $organism_id;
}

/**
 * Batch callback. Shows summary of batch process or error message.
 *
 * @param bool $success
 *   TRUE when batch completed with success.
 * @param array $results
 *   List of output of each callback call.
 */
function tpps_batch_ncbi_taxonomy_id_finished_callback($success, array $results) {
  $source = NcbiTaxonomy::KEY;
  tpps_batch_common_taxonomy_id_finished_callback($source, $success, $results);
}

/**
 * Common 'batch_N_taxonomy_id_finished_callback'.
 *
 * @param string $source
 *   Taxonomy id source. Possible values: 'ncbi' and 'gbif'.
 * @param bool $success
 *   When TRUE batch complete with success.
 * @param array $results
 *   Batch processing results.
 */
function tpps_batch_common_taxonomy_id_finished_callback($source, $success, array $results) {
  global $_SESSION;
  $session_name = 'tpps_batch_' . $source . '_taxonomy_id';
  // The 'success' parameter means no fatal PHP errors were detected. All
  // other error management should be handled using 'results'.
  if ($success) {
    $rows = [];
    // Add index to each row.
    foreach ($results as $key => $row) {
      if (is_numeric($row['taxonomy_id'])) {
        $row['taxonomy_id'] = '<strong>' . $row['taxonomy_id'] . '</strong>';
      }
      $rows[] = array_merge(['#' => $key + 1], $row);
    }
    $message = format_plural(count($results), 'One item processed.', '@count items processed.');
    $_SESSION[$session_name] = theme('table', [
      'header' => [
        '#',
        t('Organism Id'),
        t('Organism Name'),
        t('Taxonomy Id'),
        t('Primary Name'),
        t('Primary Id'),
      ],
      'rows' => $rows,
      'attributes' => ['class' => ['view', 'tpps_table']],
      'sticky' => TRUE,
    ]);
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
}
