<?php

/**
 * @file
 *
 * Helper functions related to Phenotype Synonym.
 *
 * Phenotype Synonym is a predefined set of Phenotypes attributes which could
 * be used to make it easier to add new Phenotypes and speed-up form submission.
 */

/**
 * Gets list of Phenotype Synonyms.
 *
 * @return array
 *   Returns associative array where key is synonym_id and value is name.
 */
function tpps_synonym_get_list() {
  $matches = array();
  $results = chado_select_record(
    'phenotype_synonyms',
    array('phenotype_synonyms_id', 'synonym'),
    array('synonym' => array('data' => 0, 'op' => '>'))
  );
  foreach ($results as $row) {
    $matches[$row->phenotype_synonyms_id] = check_plain($row->synonym);
  }
  return $matches;
}

/**
 * Gets Synonym's data.
 *
 * To get full Phenotype data use tpps_synonym_get_phenotype();
 *
 * @param int $synonym_id
 *   Phenotype Synonym Id.
 *
 * @return object
 *   Returns all the Synonym's data.
 *   Returns FALSE if synonym wasn't found.
 */
function tpps_synonym_get($synonym_id) {
  $results = chado_select_record(
    'phenotype_synonyms',
    array('*'),
    array('phenotype_synonyms_id' => $synonym_id)
  );
  if (!empty($results)) {
    // We need only one row.
    return reset($results);
  }
  return FALSE;
}

/**
 * Sets correct Phenotype values by Synonym.
 *
 * @param array $current_phenotype
 * @access public
 *
 * @return void
 */
function tpps_synonym_restore_values(array &$current_phenotype) {
  if (empty($synonym_id = $current_phenotype['synonym_id'])) {
    return;
  }
  $synonym = tpps_synonym_get($synonym_id);
  $current_phenotype['name'] = $current_phenotype['synonym_name'];
  $current_phenotype['description'] = $current_phenotype['synonym_description'];
  // Restore phenotype attributes by $synonym_id.
  $current_phenotype['attribute'] = $synonym->attribute_id ?? 'other';
  if (empty($synonym->attribute_id)) {
    watchdog('tpps', 'Synonym #@synonym_id has no "attribute_id"',
      array('@synonym_id' => $synonym_id), WATCHDOG_CRITICAL
    );
    $current_phenotype['attribute'] = 'other';
    // Set fake attribute name to avoid errors.
    // @TODO Get attribute name from the file?
    $current_phenotype['attr-other'] = 'new attribute';
  }
  // Structure.
  $current_phenotype['structure'] = $synonym->structure_id;
  if (empty($synonym->structure_id)) {
    watchdog('tpps', 'Synonym #@synonym_id has no "structure_id"',
      array('@synonym_id' => $synonym_id), WATCHDOG_CRITICAL
    );
  }
  // Unit.
  $current_phenotype['units'] = $synonym->unit_id ?? 'other';

  // @TODO Map unused synonym fields:
  // cvterm_1_id;
  // cvterm_2_id;
  // is_common_phenotype.
  // @TODO Unmapped form fields:
  // env-check
  // bin-check
}


/**
 * Stores relation between Phenotype, Synonym and Unit [vs].
 *
 * @param array $current_phenotype
 *   Part of $form_state with currently processing Phenotype.
 * @param array $id_list
 *   Accosiative array where keys are table names and values are foreign keys
 *   and their values.
 *   For example for $id_list['phenotype'] keys will be Phenotype names and v
 *   alues are  Phenotype Ids.
 *
 * @return bool
 *   Returns TRUE in case of success and FALSE otherwise.
 */
function tpps_synonym_save(array $current_phenotype, array $id_list ) {
  $condition = (
    empty($current_phenotype['synonym_id'])
    || $synonym = tpps_synonym_get($current_phenotype['synonym_id'])
  );
  if ($condition) {
    return FALSE;
  }

  //tpps_log(print_r($form_state, 1));
  // Checkbox 'I can't find Synonym' is unchecked.
  // Synonym was used.
  foreach ($id_list['phenotype'] as $phenotype_id) {
    // Chado API couldn't be used because table 'chado.phenotype_to_synonym'
    // do not belongs to Chado yet.
    // @TODO Minor. Use db_query() to insert multiple records in one query.
    //       Need to check if records exists.
    db_merge('chado.phenotype_to_synonym')
      ->key(array(
        'phenotype_id' => $phenotype_id,
        'phenotype_synonyms_id' => $current_phenotype['synonym_id'],
      ))
      ->fields(array(
        'phenotype_id' => $phenotype_id,
        'phenotype_synonyms_id' => $current_phenotype['synonym_id'],
      ))
      ->execute();
    tpps_log('[INFO] Stored relation between Phenotype #@phenotype_id '
      . 'and Synonym #@synonym_id.', array(
        '@phenotype_id' => $phenotype_id,
        '@synonym_id' => $current_phenotype['synonym_id'],
      )
    );
  }

  // Case when custom Unit was usd.
  $condition = (
    // For some Synonyms 'unit_id' is NULL.
    !empty($synonym['unit_id'])
    && $current_phenotype['units'] != $synonym['unit_id']
  );
  //if ($condition) {
  //  db_insert('tpps_phenotype_unit_warning')
  //    ->fields(array(
  //      'phenotype_id' => $phenotype_id,
  //      'unit_id' => $current_phenotype['units'],
  //    ))->execute();
  //}


  return TRUE;

}
