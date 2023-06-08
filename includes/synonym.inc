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
 * @param array $options
 *   List of options which controls how list will look like.
 *   Options and it's default value:
 *     'not_in_list_option' => TRUE; - Item 'Can't find ...' will be added.
 *     'debug' => FALSE; - options value will be shown in title.
 *
 * @return array
 *   Returns associative array with synonym_id as key
 *   and synonym's name as label.
 */
function tpps_synonym_get_list(array $options = []) {
  // Get defaults.
  $defaults = ['not_in_list_option' => TRUE, 'debug' => FALSE];
  $options = array_merge($defaults, $options);

  // Using chado_select_record() don't allow to use fetchAllKeyed().
  $query = db_select('chado.phenotype_synonyms', 'cps')
    ->fields('cps', ['phenotype_synonyms_id', 'synonym'])
    ->condition('cps.synonym', 0, '>');
  $list = $query->execute()->fetchAllKeyed();
  // This option must be last.
  if ($options['not_in_list_option']) {
    $list[0] = t("I can't find synonym");
  }
  if ($options['debug']) {
    foreach ($list as $key => $value) {
      $list[$key] = $key . ': ' . $value ;
    }
  }
  return $list;
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
function tpps_synonym_get_unit_list($synonym_id = '', array $options = []) {
  $defaults = ['not_in_list_option' => TRUE, 'debug' => FALSE];
  $options = array_merge($defaults, $options);

  $query = db_select('chado.phenotype_units', 'cpu')
    ->fields('cpu', ['unit_cvterm_id', 'unit_name']);
  if (!empty($synonym_id)) {
    $query->leftJoin('chado.phenotype_synonyms', 'cps',
      'cps.unit_group = cpu.unit_group'
    );
    $query->condition('cps.phenotype_synonyms_id', $synonym_id);
  }
  $list = $query->execute()->fetchAllKeyed();
  // This option must be last.
  if (!empty($list) && $options['not_in_list_option']) {
    $list['other'] = t('My unit is not in this list');
  }
  if ($options['debug']) {
    foreach ($list as $key => $value) {
      $list[$key] = $key . ': ' . $value ;
    }
  }

  return $list;
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
    $unit_field['#options'] = tpps_synonym_get_unit_list(
      $synonym_id,
      ['debug' => FALSE]
    );
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
  if (empty($unit_id) || !is_numeric($unit_id)) {
    return;
  }
  $unit_name = db_select('chado.phenotype_units', 'cpu')
    ->fields('cpu', ['unit_name'])
    ->condition('cpu.unit_cvterm_id', $unit_id)
    ->range(0, 1)
    ->execute()
    ->fetchField();
  if (empty($unit_name)) {
    $unit_name = db_select('chado.cvterm', 'cc')
      ->fields('cc', ['name'])
      ->condition('cc.cvterm_id', $unit_id)
      ->range(0, 1)
      ->execute()
      ->fetchField();
  }
  return $unit_name;
}


/**
 * Stores relation between Phenotype, Synonym and Unit [vs].
 *
 * @param array $synonym_list
 *   Associative array where key is a number of phenotype
 *   (number of items is equal to number of phenotypes). Value is an
 *   associative array with keys:
 *   - 'unit' int Unit Id.
 *   - 'synonym_id' int Synonym Id.
 * @param array $id_list
 *   List of Phenotype Ids received after saving into database.
 */
function tpps_synonym_save(array $synonym_list, array $id_list) {
  tpps_log('[INFO] Phenotype Synonyms processing started...');
  $j = 1;
  $count_synonym_list = count($synonym_list);
  foreach (array_values($id_list) as $phenotype_id) {
    // Store Phenotype and Synonym relation.
    if ($synonym_list[$j]['synonym_id']) {
      db_merge('chado.phenotype_to_synonym')
        ->key([
          'phenotype_id' => $phenotype_id,
          'phenotype_synonyms_id' => $synonym_list[$j]['synonym_id'],
        ])
        ->fields([
          'phenotype_id' => $phenotype_id,
          'phenotype_synonyms_id' => $synonym_list[$j]['synonym_id'],
        ])
        ->execute();
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Create a record in 'Unit Warning' table
    // when Synonym's Unit differs from Submitted Unit.
    //$unit_group = tpps_synonym_get_unit_list(
    //  $phenotypes_meta[$j]['synonym_id']
    //);
    if ($synonym_list[$j]['unit'] == 0) {
      // Store phenotype Id which has custom unit.
      // Note: The same data could be found in column
      // 'public.tpps_submissions.submission_state' but it's serialized
      // and so hard to search and use reports.
      db_merge('tpps_phenotype_unit_warning')
        ->key(['phenotype_id' => $phenotype_id])
        ->fields(['phenotype_id' => $phenotype_id])
        ->execute();
    }
    // Loop phenotypes to get correct Synonym Id.
    $j = ($j < $count_synonym_list) ? ($j + 1) : 1;
  }
  tpps_log('[INFO] Phenotype Synonyms processing completed.');
}
