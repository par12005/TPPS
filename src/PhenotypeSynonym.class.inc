<?php

/**
 * @file
 * Contains 'Phenotype Synonym' class.
 */

/**
 * Manages phenotype synonyms.
 *
 * Phenotype Synonym is a predefined set of Phenotypes attributes which could
 * be used to make it easier to add new Phenotypes and speed-up form submission.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PhenotypeSynonym {

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
  public static function getOptionList(array $options = [], $reset = FALSE) {

    $list = PhenotypeSynonym::getAll($reset);
    // Get defaults.
    $defaults = ['not_in_list_option' => TRUE, 'debug' => FALSE];
    $options = array_merge($defaults, $options);

    asort($list, SORT_STRING | SORT_FLAG_CASE | SORT_NATURAL);
    if (variable_get('tpps_page_4_use_alphabetic_list', TRUE)) {
      $list = tpps_build_alphabetic_list($list);
    }
    if ($options['debug']) {
      foreach ($list as $letter => $row) {
        foreach ($row as $key => $value) {
          $list[$letter][$key] = $key . ': ' . $value;
        }
      }
    }
    // This option must be last.
    if ($options['not_in_list_option']) {
      $list[0] = t("I can't find phenotype");
    }
    return $list;
  }

  /**
   * Gets Synonym's data.
   *
   * @param int $synonym_id
   *   Phenotype Synonym Id.
   *
   * @return object
   *   Returns all the Synonym's data.
   *   Returns FALSE if synonym wasn't found.
   *
   * @todo Merge with ::getAll() method.
   */
  public static function get($synonym_id) {
    // Table 'phenotype_synonyms' was created manually and wasn't added
    // to Chado so Chado API can't be used for now.
    $use_chado = FALSE;
    if ($use_chado) {
      $results = chado_select_record(
        'phenotype_synonyms', ['*'], ['phenotype_synonyms_id' => $synonym_id]
      );
    }
    else {
      $result = db_select('chado.phenotype_synonyms', 'cps')
        ->fields('cps', [])
        ->condition('cps.phenotype_synonyms_id', $synonym_id)
        ->execute()
        ->fetchAll();
    }
    return reset($result);
  }

  /**
   * Sets correct Phenotype values by Synonym.
   *
   * @param array $current_phenotype
   *   Currently processing phenotype.
   */
  public static function restoreValues(array &$current_phenotype) {
    if (empty($synonym_id = $current_phenotype['synonym_id'])) {
      return;
    }
    $synonym = PhenotypeSynonym::get($synonym_id);
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
   *   If there will be no 'synonym_id' key or it has empty value we will
   *   try to get Synonym Id using Unit Id but it's not exact match because
   *   Unit could belong to many Synonyms. We will use 1st Synonym from the list.
   * @param array $id_list
   *   List of Phenotype Ids received after saving into database.
   *   Item format: $phenotype_name => $phenotype_id.
   */
  public static function save(array $phenotypes_meta, array $id_list) {
    tpps_log('Phenotype Synonyms processing started...');
    // Create a small array to make search of phenotype name easier.
    foreach ($phenotypes_meta as $short_name => $item) {
      // Note: It's possible that synonym not exists when it was automatically
      // detected by unit from Metadata File.
      if (!empty($item['synonym_id']) && !empty($item['name'])) {
        // $item['name'] is Phenotype name in lowser case. For example: 'leaf a'.
        // Phenotype uniquename is using the same name but not lowercased so
        // it couldn't be used.
        $synonym_list[$item['synonym_id']] = [
          'short_name' => $short_name,
          'full_name' => $item['name'],
        ];
      }
    }
    if (empty($synonym_list) || empty($id_list)) {
      return;
    }

    if ($debug_mode ?? FALSE) {
      tpps_log($synonym_list, 'synonym_list');
    }

    foreach ($id_list as $uniquename => $phenotype_id) {
      // $uniquename exampe: 'TGDR824-ID_18-11-leaf A-154-Piln-unit'
      // Ordinal number:      0       1     2  3      4   5    6.
      // $phenotype_name exampe: 'leaf A'
      // But phenotype name could be a one letter!..
      // The other way is to get phenotype original name in $id_list.
      $phenotype_name = explode('-', strtolower($uniquename))[3];
      foreach ($synonym_list as $synonym_id => $synonym_data) {
        if (strtolower($synonym_data['short_name']) == $phenotype_name) {
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
    tpps_log('Phenotype Synonyms processing completed.');
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
  public static function selector($id) {
    return ':input[name="' . $id . '[phenotype][phenotypes-meta][!num][synonym_id]"]';
  }

  /**
   * Creates new phenotype synonym in database.
   *
   * @param array $data
   *   Synonym's data to be inserted into database.
   *
   * @return int
   *   Returns Id of recently added Phenotype Synonym.
   */
  public static function add(array $data) {
    $id = db_insert('chado.phenotype_synonyms')->fields($data)->execute();
    // Update cached list of units.
    // @todo Use hook to allow others to react on change.
    PhenotypeUnit::getList('all', [], TRUE);
    return $id;
  }

  /**
   * Gets list of phenotype's attributes.
   *
   * @param string $field_name
   *   Default is 'attribute_id'.
   *   Name of the column in 'chado.phenotype_synonyms' table linked with
   *   'chado.cvterm' (foreign key).
   *   Possible values are: 'attribute_id', 'structure_id'.
   *
   * @return array
   *   Returns list of Phenotype attrubutes. Key is an attrubute Id and value is
   *   an attribute name.
   *   Returns empty array of nothing was found.
   *
   * @todo Cache results.
   */
  public static function getTermList($field_name = 'attribute_id') {
    if (!in_array($field_name, ['attribute_id', 'structure_id'])) {
      watchdog('tpps', 'Not supported term @term in @function',
        ['@term' => $field_name, '@function' => __FUNCTION__],
        WATCHDOG_ERROR
      );
      return [];
    }
    // Table 'phenotype_synonyms' was created manually and wasn't added
    // to Chado so Chado API can't be used for now.
    // Note: Table 'phenotype_synonyms' doesn't have attribute names but only IDs.
    $field_name = 'attribute_id';
    $query = db_select('chado.phenotype_synonyms', 'cps')->fields('cps', [$field_name]);
    $query->leftJoin('chado.cvterm', 'ccvt', 'cps.' . $field_name . ' = ccvt.cvterm_id');
    $query->addField('ccvt', 'name');
    //$query->groupBy('ccvt.name');
    //$query->groupBy('cps.' . $field_name);
    $query->orderBy('cps.' . $field_name);
    //$query->addTag('debug');
    return $query->execute()->fetchAllKeyed() ?? [];
  }

  /**
   * Gets list of the all phenotype synonyms.
   *
   * @param bool $reset
   *   When TRUE static caching will not be used. Default is FALSE.
   *
   * @return array
   *   Returns associative array with synonym_id as key
   *   and synonym's name as label.
   */
  public static function getAll($reset = FALSE) {
    $key = $synonym_cid = 'tpps_synonym_list';
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    $cache = cache_get($synonym_cid, $cache_bin);
    $is_cache_allowed = variable_get('tpps_page_4_cache_synonym_list', FALSE);
    if (
      $reset || empty($cache) || empty($cache->data[$key])
      || !$is_cache_allowed
    ) {
      // Using chado_select_record() don't allow to use fetchAllKeyed().
      $query = db_select('chado.phenotype_synonyms', 'cps')
        ->fields('cps', ['phenotype_synonyms_id', 'synonym'])
        ->condition('cps.synonym', 0, '>')
        ->orderBy('cps.phenotype_synonyms_id');
      $cache->data[$key] = $query->execute()->fetchAllKeyed();
      cache_set($synonym_cid, $cache->data, $cache_bin);
    }
    return $cache->data[$key];
  }

}
