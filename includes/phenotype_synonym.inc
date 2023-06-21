<?php

/**
 * @file
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
      $list[$key] = $key . ': ' . $value;
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
    $synonym_list[$item['synonym_id']] = $item['name'];
  }
  if (empty($synonym_list) || empty($id_list)) {
    return;
  }
  foreach ($id_list as $uniquename => $phenotype_id) {
    // $uniquename exampe: 'TGDR824-ID_18-11-leaf A-154-Piln-unit'
    // $phenotype_name exampe: 'leaf A'
    // But phenotype name could be a one letter!..
    foreach ($synonym_list as $synonym_id => $phenotype_name) {
      if ($phenotype_name == explode('-', $uniquename)[2]) {
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
  }
  tpps_log('[INFO] Phenotype Synonyms processing completed.');
}

/**
 * Builds a selector for 'Synonym Missing' checkbox for form states.
 *
 * @param string $id
 *   Organism Id.
 *
 * @return string
 *   Ready for form state selector.
 */
function tpps_synonym_selector($id) {
  return ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][synonym_id]"]';
}
