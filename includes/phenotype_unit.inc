<?php

/**
 * @file
 * Helper functions related to 'Phenotype Unit'.
 */

/**
 * Gets list of units.
 *
 * @param int $synonym_id
 *   Synonym Id. See db table 'chado.phenotype_synonyms'.
 *   Default is 'all' (show all units).
 * @param array $options
 *   Associative array with keys and default values:
 *   'not_in_list_option' => TRUE,
 *   'debug' => FALSE.
 * @param bool $reset
 *   When TRUE static caching will not be used. Default is FALSE.
 *
 * @return array
 *   Returns list of units related to given synonym.
 *   Returns all units if synonym not specified.
 *   Returns empty array if synonym not found.
 */
function tpps_unit_get_list($synonym_id = 'all', array $options = [], $reset = FALSE) {
  $defaults = ['not_in_list_option' => TRUE, 'debug' => FALSE];
  $options = array_merge($defaults, $options);

  $synonym_cid = 'tpps_synonym_list';
  $unit_cid = 'tpps_unit_list';
  $cache = cache_get($unit_cid);

  if (empty($cache) || $reset) {
    // Get synonyms and related unit groups.
    $synonym_table = db_select('chado.phenotype_synonyms', 'cps')
      ->fields('cps', [
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
      $groups[$unit['unit_group']][$unit['unit_cvterm_id']]
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
      $list[$key] = $key . ': ' . $value;
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
function tpps_unit_update_list($form, &$form_state) {
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
    $unit_field['#options'] = tpps_unit_get_list(
      $synonym_id ?? 'all',
      ['debug' => FALSE]
    );
    // @see https://www.drupal.org/forum/support/post-installation/2018-04-25/an-illegal-choice-has-been-detected-please-contact-the#comment-13254706
    return form_process_select($unit_field);
  }
  watchdog('tpps', 'Field "unit" not found in AJAX callback.', [], WATCHDOG_ERROR);
}

/**
 * Gets unit name.
 *
 * @param int $unit_id
 *   Unit Id. See db table 'chado.phenotype_units'.
 *
 * @return string
 *   Returns name of unit.
 */
function tpps_unit_get_name($unit_id = '') {
  if (empty($unit_id) || !is_numeric($unit_id)) {
    return '';
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