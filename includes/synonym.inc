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
 * @param bool $not_in_list_option
 *   When TRUE extra item will be added to the unit list. Could be used in
 *   dropdown lists. When FALSE then only list of units will be returned.
 *
 * @return array
 *   Returns associative array where key is synonym_id and value is name.
 */
function tpps_synonym_get_list($not_in_list_option = TRUE) {
  // Using chado_select_record() don't allow to use fetchAllKeyed().
  $query = db_select('chado.phenotype_synonyms', 'cps')
    ->fields('cps', ['phenotype_synonyms_id', 'synonym'])
    ->condition('cps.synonym', 0, '>');
  $synonym_list = $query->execute()->fetchAllKeyed();
  // This option must be last.
  if ($not_in_list_option) {
    $synonym_list[0] = t("I can't find synonym");
  }
  return $synonym_list;
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
 *   Currently processing phenotype.
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
}

/**
 * Stores relation between Phenotype, Synonym and Unit [vs].
 *
 * @param array $current_phenotype
 *   Part of $form_state with currently processing Phenotype.
 * @param int $phenotype_id
 *   Phenotype Id.
 */
function tpps_synonym_save(array $current_phenotype, $phenotype_id ) {
  $debug_mode = FALSE;
  if ($debug_mode) {
    tpps_log('[DEBUG] '
      . 'Phenotype Id: ' . $phenotype_id
      . "\nCurrent Phenotype:\n " . print_r($current_phenotype, 1)
    );
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Store phenotype and synonym relation.
  if ($current_phenotype['synonym_id']) {
    // We store zero value when Synonym wasn't used to allow to filter out
    // only new phenotypes without synonym from 'phenotype_to_synonym' table
    // which not allow NULL but only integer values.
    // We use db_merge() instead of db_insert() to avoid errors if phenotype
    // will be updated.
    // Chado API couldn't be used because table 'chado.phenotype_to_synonym'
    // do not belongs to Chado yet.
    // Note:
    //   To insert multiple records in one query (db_insert()) we need to
    //   check if records exist and use db_update() so it's better to use
    //   db_merge() even if it's slower.
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
      . 'and Synonym #@synonym_id.',
      [
        '@phenotype_id' => $phenotype_id,
        '@synonym_id' => $current_phenotype['synonym_id'],
      ]
    );
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Create a record in 'Unit Warning' table
  // when Synonym's Unit differs from Submitted Unit.
  $synonym = tpps_synonym_get($synonym_id);
  $unit_group = tpps_synonym_get_unit_list($synonym_id, FALSE);

  // @TODO Remove debug code.
  // Used for demo only.
    //db_merge('tpps_phenotype_unit_warning')
    //  ->key(['phenotype_id' => $phenotype_id])
    //  ->fields(['phenotype_id' => $phenotype_id])
    //  ->execute();
  // @End Remove debug code

  if ($current_phenotype['units'] == 0) {
    // Store phenotype Id which has custom unit.
    // Note: The same data could be found in column
    // 'public.tpps_submissions.submission_state' but it's serialized
    // and so hard to search and use reports.
    db_merge('tpps_phenotype_unit_warning')
      ->key(['phenotype_id' => $phenotype_id])
      ->fields(['phenotype_id' => $phenotype_id])
      ->execute();
  }
  if ($debug_mode) {
    tpps_log('[DEBUG] ' . 'Phenotype Id: ' . $phenotype_id);
    tpps_log('[DEBUG] ' . 'Phenotype Unit: ' . $current_phenotype['unit']);
    tpps_log('[DEBUG] ' . 'Synonym Unit: ' . $synonym->unit_id);
  }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Synonym related Units.

/**
 * Gets list of units.
 *
 * @param int $synonym_id
 *   Synonym Id. See db table 'chado.phenotype_synonyms'.
 * @param bool $not_in_list_option
 *   When TRUE extra item will be added to the unit list. Could be used in
 *   dropdown lists. When FALSE then only list of units will be returned.
 *
 * @return array
 *  Returns list of units related to given synonym.
 *  Returns all units if synonym not specified.
 *  Returns empty array if synonym not found.
 */
function tpps_synonym_get_unit_list($synonym_id = '', $not_in_list_option = TRUE) {
  $query = db_select('chado.phenotype_units', 'cpu')
    ->fields('cpu', ['unit_cvterm_id', 'unit_name']);
  if (!empty($synonym_id)) {
    $query->leftJoin('chado.phenotype_synonyms', 'cps',
      'cps.unit_group = cpu.unit_group'
    );
    $query->condition('cps.phenotype_synonyms_id', $synonym_id);
  }
  $unit_list = $query->execute()->fetchAllKeyed();
  // This option must be last.
  if (!empty($unit_list) && $not_in_list_option) {
    $unit_list['other'] = t('My unit is not in this list');
  }
  return $unit_list;
}

/**
 * Custom callback to build form fields.
 *
 * Creates a form element for 'Unit' dropdown which will replce existing one
 * at page when Synonym selectbox changed.
*/
function tpps_synonym_update_unit_list($form, &$form_state) {

  // @TODO Cache list of units by phenotype id and $synonym_id because
  // adding 2+ phenotypes increases ajax request processing time.
  $synonym_id = $form_state['triggering_element']['#value'];
  $parents = $form_state['triggering_element']['#parents'];
  // This is true until 'synonym_id' and 'unit' fields are siblings.
  array_pop($parents);
  $parents[] = 'unit';

  // Update list of units.
  $unit_field = drupal_array_get_nested_value($form, $parents, $key_exists);
  if ($key_exists) {
    $unit_field['#options'] = tpps_synonym_get_unit_list($synonym_id, TRUE);
    // https://www.drupal.org/forum/support/post-installation/2018-04-25/an-illegal-choice-has-been-detected-please-contact-the#comment-13254706
    //$unit_field['#validated'] = TRUE;
    return form_process_select($unit_field);
  }
  watchdog('tpps', 'Field "unit" not found in AJAX callback.', [], WATCHDOG_ERROR);
}

/**
 * Gets unit name.
 *
 * @param int $synonym_id
 *   Unit Id. See db table 'chado.phenotype_units'.
 *
 * @return string
 *  Returns name of unit.
 */
function tpps_synonym_get_unit_name($unit_id = '') {
  if (empty($unit_id)) {
    return;
  }
  return db_select('chado.phenotype_units', 'cpu')
    ->fields('cpu', ['unit_name'])
    ->condition('cpu.unit_cvterm_id', $unit_id)
    ->range(0, 1)
    ->execute()
    ->fetchField();
}
