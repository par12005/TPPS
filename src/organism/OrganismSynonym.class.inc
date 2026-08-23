<?php

/**
 * @file
 * Contains 'OrganismSynonym' class.
 */

/**
 * Manages TPPS OrganismSynonym.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class OrganismSynonym {

  /**
   * Name of the table where data is stored.
   */
  const TABLE = 'chado.organismprop';

  /**
   * CvTerm Id for 'Synonym'.
   *
   * See: chado.cvterm.name.
   * 'Synonym. An alternative for a word. [http://edamontology.org]'.
   * Emily: cvterm_id 50375 is preferable as it uses the EDAM ontology
   * rather than local.
   */
  const CVTERM_ID = 50375;

  /**
   * Gets organism id by key (which could be name or id).
   *
   * @param mixed $organism_key
   *   Organism Key (name or id).
   *
   * @return int
   *   Returns Organism Id.
   *
   * @throws Exception
   *   When Organism Id couldn't be found by name.
   *   When given $organism_key is nor number neither string.
   */
  public static function getId($organism_key) {
    if (is_int($organism_key)) {
      return (int) $organism_key;
    }
    elseif (is_string($organism_key)) {
      $id = tpps_organism($organism_key)->id;
      if (empty($id)) {
        $message = t("Organism Id can't be found for name: '@key'",
          ['@key' => $organism_key]);
        throw new Exception($message);
      }
      return (int) $id;
    }
    else {
      $message = "Unsupported type of the organism key: @key_dump";
      $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 10);
      $params = [
        '@key_dump' => var_export($organism_key, 1),
        '!stack' => print_r($stack, 1),
      ];
      watchdog('tpps', $message . '<pre>!stack</pre>', $params);
      throw new Exception(t($message, $params));
    }
  }

  /**
   * Gets data for 'Organism Synonym' report.
   *
   * No need to cache result because it takes about 10 msec to build.
   *
   * @param int $organism_key
   *   Organism Key (id or name).
   *
   * @return array
   *   Returns data for 'Organism Synonym' report.
   *
   * @throws Exception
   *   When Organism Id couldn't be found by name.
   *   When given $organism_key is nor number neither string.
   */
  public static function get($organism_key = NULL) {
    $query = db_select(static::TABLE, 't')
      ->fields('t', ['organismprop_id'])
      ->condition('t.type_id', static::CVTERM_ID);
    $query->leftJoin('chado.organism', 'o', 't.organism_id = o.organism_id');
    $query->addField('o', 'organism_id');
    $query->addField('o', 'genus');
    $query->addField('o', 'species');
    $query->addField('o', 'infraspecific_name');
    $query->addField('t', 'value', 'synonym_name');
    $query->orderBy('t.value');

    // Specific organism.
    if (!empty($organism_key)) {
      $organism_id = static::getId($organism_key);
      $query->orderBy('o.organism_id');
      $query->condition('t.organism_id', $organism_id);
    }

    return $query->execute()
      ->fetchAllAssoc('organismprop_id', PDO::FETCH_ASSOC);
  }

  /**
   * Loads list of the synonyms for organism.
   *
   * @param mixed $organism_key
   *   Organism Key (id or name).
   *
   * @return array
   *   Returns list of the synonyms or empty array.
   *   Format: <oranismprop_id> => <synonym name>.
   *   Example: [18454 => 'Pinus halepensis subsp. brutia'].
   *
   * @throws Exception
   *   When Organism Id couldn't be found by name.
   *   When given $organism_key is nor number neither string.
   */
  public static function load($organism_key) {
    $organism_id = static::getId($organism_key);
    $list = db_select(static::TABLE, 't')
      ->fields('t', ['organismprop_id', 'value'])
      ->condition('t.type_id', static::CVTERM_ID)
      ->condition('t.organism_id', $organism_id)
      ->orderBy('t.organismprop_id')
      ->execute()
      ->fetchAllKeyed();
    return $list ?? [];
  }

  /**
   * Checks if there is a record for synonym name.
   *
   * @param mixed $organism_key
   *   Organism Key (id or name).
   * @param string $synonym_name
   *   Organism Synonym name.
   *
   * @return bool
   *   Returns TRUE if synonym name found and FALSE otherwise.
   */
  public static function isExists($organism_key, $synonym_name) {
    return (
      // WARNING: Do NOT change the order of checks!
      !empty($synonym_list = static::load($organism_key))
      && $id = TppsArray::searchInsensitive($synonym_name, $synonym_list)
    );
  }

  /**
   * Creates new record for organism synonym.
   *
   * @param mixed $organism_key
   *   Organism Key (id or name).
   * @param string $synonym_name
   *   Organism Synonym.
   *
   * @return int
   *   Returns id of the created or existing record.
   */
  public static function insert($organism_key, $synonym_name) {
    $synonym_list = load($organism_key);
    if (
      // WARNING: Do NOT change the order of checks!
      !empty($synonym_list)
      && $id = TppsArray::searchInsensitive($synonym_name, $synonym_list)
    ) {
      return $id;
    }
    $id = db_insert(static::TABLE)
      ->fields([
        'organism_id' => static::getId($organism_key),
        'type_id' => static::CVTERM_ID,
        'value' => $synonym_name,
        'rank' => count($synonym_list),
      ])
      ->execute();
    return $id;
  }

}
