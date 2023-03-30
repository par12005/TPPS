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
  $results = chado_select_record(
    'phenotype_synonyms',
    ['phenotype_synonyms_id', 'synonym'],
    ['synonym' => ['data' => 0, 'op' => '>']]
  );
  foreach ($results as $row) {
    $options[$row->phenotype_synonyms_id] = check_plain($row->synonym);
  }
  // This option must be last.
  $options[0] = "I can't find synonym";
  return $options;
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
 *
 * @TODO Return object.
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
  $condition = (
    // Synonym was selected, and it's not 'other' and it was found by id.
    (
      !empty($current_phenotype['synonym_id'])
      && !empty($synonym = tpps_synonym_get($synonym_id))
    )
    // For some Synonyms 'unit_id' is NULL and we skip check in this case.
    && !empty($synonym->unit_id)
    && (
      // Submitted Unit differs from Synonym's Unit.
      $current_phenotype['units'] != $synonym->unit_id
      || (
        // Custom Unit differs from Synonym's Unit.
        $current_phenotype['units'] == 'other'
        && $current_phenotype['unit-other'] != $synonym->unit_id
      )
    )
  );


  // @TODO Remove debug code.
  // Used for demo only.
    //db_merge('tpps_phenotype_unit_warning')
    //  ->key(['phenotype_id' => $phenotype_id])
    //  ->fields(['phenotype_id' => $phenotype_id])
    //  ->execute();
  // @End Remove debug code



  if ($condition) {
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





/**
* Custom callback to build form fields.
*/
function tpps_synonym_update_unit_list($form, &$form_state) {
  $values = $form_state['values'];

  $organism_name = $form_state['triggering_element']['#parents'][0];
  $organism_id = $form_state['triggering_element']['#parents'][3];
  $synonym_id = $form_state['triggering_element']['#value'];



  // @TODO There could be a lot of checkboxes and we need to update right one.
  watchdog('tpps', '<pre>' . print_r($form_state, 1));

  $unit_list =  tpps_synonym_get_unit_list(8);
  // $unit_list = ['cm', 'mm'];

  // Overwrite the student list fields with new options value.
  $form['units'] = [
    '#type' => 'select',
    '#title' => 'Phenotype !num Units: *',
    '#options' => $unit_list,
    '#prefix' => '<div id="unit-list-wrapper">',
    '#suffix' => '</div>',
  ];
  $form['units'] = form_process_select($form['units']);
  return $form['units'];
}

/**
 * Get list of units which belongs to given group.
 *
 * @param string $group
 *   Unit group. See chado.phenotype_synonyms.unit_group.
 *
 * @return void
 */
function tpps_synonym_get_unit_list($synonym_id) {

  $sql = "SELECT unit_cvterm_id as id, unit_name as name
    FROM {chado.phenotype_units} pu
      LEFT JOIN {chado.phenotype_synonyms} ps
      ON ps.unit_group = pu.unit_group
    WHERE ps.phenotype_synonyms_id = :synonym_id";

  $result = db_query($sql, [':synonym_id' => $synonym_id])
    ->fetchAll();
  $unit_list = array();
  foreach ($result as $item) {
    $unit_list[$item->id] = $item->name;
  }
  // This option must be last.
  // @TODO Update title.
  $unit_list[0] = "I can't find unit";
  return $unit_list;
}
