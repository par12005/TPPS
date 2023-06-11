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
 * @param bool $reset
 *   When TRUE static caching will not be used. Default is FALSE.
 *
 * @return array
 *   Returns associative array with synonym_id as key
 *   and synonym's name as label.
 */
function tpps_synonym_get_list(array $options = [], $reset = FALSE) {
  // Get defaults.
  $defaults = ['not_in_list_option' => TRUE, 'debug' => FALSE];
  $options = array_merge($defaults, $options);

  // Use Drupal's cache in database instead of static caching.
  $synonym_cid = 'tpps_synonym_list';
  $cache = cache_get($synonym_cid);
  if (empty($cache->data) || $reset) {
    // Using chado_select_record() don't allow to use fetchAllKeyed().
    $query = db_select('chado.phenotype_synonyms', 'cps')
      ->fields('cps', ['phenotype_synonyms_id', 'synonym'])
      ->condition('cps.synonym', 0, '>')
      ->orderBy('cps.phenotype_synonyms_id');
    $cache = $query->execute()->fetchAllKeyed();
    $list = $cache;
  }
  else {
    $list = $cache->data;
  }
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
 *   Default is 'all' (show all units).
 * @param bool $not_in_list_option
 *   When TRUE extra item will be added to the unit list. Could be used in
 *   dropdown lists. When FALSE then only list of units will be returned.
 * @param bool $reset
 *   When TRUE static caching will not be used. Default is FALSE.
 *
 * @return array
 *  Returns list of units related to given synonym.
 *  Returns all units if synonym not specified.
 *  Returns empty array if synonym not found.
 */
function tpps_synonym_get_unit_list($synonym_id = 'all', array $options = [], $reset = FALSE) {
  $defaults = ['not_in_list_option' => TRUE, 'debug' => FALSE];
  $options = array_merge($defaults, $options);

  $synonym_cid = 'tpps_synonym_list';
  $unit_cid = 'tpps_unit_list';
  $cache = cache_get($unit_cid);

  if (empty($cache) || $reset) {
    // Get synonyms and related unit groups.
    $synonym_table = db_select('chado.phenotype_synonyms', 'cps')
      ->fields('cps',[
        'phenotype_synonyms_id',
        'synonym',
        'unit_group',
      ])
      ->condition('cps.synonym', 0, '>')
      ->execute()
      ->fetchAllAssoc('phenotype_synonyms_id', PDO::FETCH_ASSOC);
    foreach ($synonym_table as $synonym) {
      $synonym_list[$synonym['phenotype_synonyms_id']] = $synonym['synonym'];
      $synonym_unit_group[$synonym['phenotype_synonyms_id']] = $synonym['unit_group'];
    }
    ksort($synonym_list);
    ksort($synonym_unit_group);
    // Update cached list of synonyms.
    cache_set($synonym_cid, $synonym_list);

    // Get list of all units.
    $unit_table = db_select('chado.phenotype_units', 'cpu')
      ->fields('cpu', [
        'unit_cvterm_id',
        'unit_name',
        'unit_group',
      ])
      ->execute()
      ->fetchAllAssoc('unit_cvterm_id', PDO::FETCH_ASSOC);
    // Create list of all units and list of unit groups.
    foreach ($unit_table as $unit_id => $unit) {
      $cache['all'][$unit_id] = $unit['unit_name'];
      $groups[ $unit['unit_group'] ][ $unit['unit_cvterm_id'] ]
        = $unit['unit_name'];
    }
    // Create list of units per synonym.
    foreach ($synonym_unit_group as $synonym_id => $unit_group) {
      $cache[$synonym_id] = ($groups[$unit_group] ?? NULL);
    }
    cache_set($unit_cid, $cache);
    $list = $cache[$synonym_id];
  }
  else {
    $list = $cache->data[$synonym_id];
  }
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
 * Creates a form element for 'Unit' dropdown which will replace existing
 * one at page when Synonym selectbox changed.
*/
function tpps_synonym_update_unit_list($form, &$form_state) {
  // @TODO Cache list of units by phenotype id and $synonym_id because
  // adding 2+ phenotypes increases ajax request processing time.
  if (isset($form_state['triggering_element']['#value'])) {
    $synonym_id = $form_state['triggering_element']['#value'];
  }
  $parents = $form_state['triggering_element']['#parents'];
  // This is true until 'synonym_id' and 'unit' fields are siblings.
  array_pop($parents);
  $parents[] = 'unit';

  // Update list of units.
  $unit_field = drupal_array_get_nested_value($form, $parents, $key_exists);
  if ($key_exists) {
    $unit_field['#options'] = tpps_synonym_get_unit_list(
      $synonym_id ?? 'all',
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
 * @param array $phenotypes_meta
 *   Associative array where key is a phenotype name in lowercase and value
 *   is an associative array with phenotype metadata.
 * @param array $id_list
 *   List of Phenotype Ids received after saving into database.
 *   Item format: $phenotype_name => $phenotype_id.
 */
function tpps_synonym_save(array $phenotypes_meta, array $id_list) {
  tpps_log('[INFO] Phenotype Synonyms processing started...');

  // Create a small array to make search of phenotype name easier.
  foreach ($phenotypes_meta as $item) {
    // $name is Phenotype name in lowser case. For example: 'leaf a'.
    // Phenotype uniquename is using the same name but not lowercased.
    $synonym_list[ $item['synonym_id'] ] = $item['name'];
  }
  foreach ($id_list as $uniquename => $phenotype_id) {
    foreach ($synonym_list as $synonym_id => $phenotype_name) {
      // $uniquename exampe: 'TGDR824-ID_18-11-leaf A-154-Piln-unit'
      // $phenotype_name exampe: 'leaf A'
      if (strpos($uniquename, $phenotype_name) !== FALSE) {
        // Store relation between Phenotype Id and Synonym Id.
        db_merge('chado.phenotype_to_synonym')
          ->key([
            'phenotype_id' => $phenotype_id,
            'phenotype_synonyms_id' => $synonym_id,
          ])
          ->fields([
            'phenotype_id' => $phenotype_id,
            'phenotype_synonyms_id' => $synonym_id,
          ])
          ->execute();
      }
    }
    tpps_log('[INFO] Phenotype Synonyms processing completed.');
  }
}
