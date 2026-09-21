<?php

/**
 * @file
 * Contains abstract class 'OrganismTaxonomy'.
 */

/**
 * Holds common methods for 'NcbiTaxonomy' and 'GbifSpeciesApi' classes.
 *
 * @todo Remove 'organism_name' column. Used in reports to build list of
 * organism_ids withour taxonomy id.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
abstract class OrganismTaxonomy {
  /**
   * Name of the table which stores Taxonomy Ids per organism.
   */
  const TABLE = '';

  /**
   * Gets DB table schema.
   *
   * Table to store relations between organism name and Taxonomy Id.
   *
   * Used in hook_install().
   * Table name: self::TABLE.
   *
   * Usage example:
   *   $field_list = array_keys(self::getSchema()['fields']);
   *   unset($field_list['source']);
   *
   * @return array
   *   Returns db table schema.
   */
  public static function getSchema() {
    return [
      'description' => 'Taxonomy Id for organisms.',
      'fields' => [
        'organism_id' => [
          'description' => 'Organism Id.',
          'type' => 'int',
          'size' => 'big',
          'not null' => FALSE,
        ],
        'taxonomy_id' => [
          'description' => 'Taxomony Id.',
          'type' => 'int',
          'size' => 'big',
          'not null' => FALSE,
        ],
      ],
      'foreign keys' => [
        'organism_id' => [
          'table' => 'chado.organism',
          'columns' => [
            'organism_id' => 'organism_id',
          ],
        ],
      ],
      'unique keys' => [
        'organism_id' => ['organism_id'],
      ],
      // @todo Add indexes.
    ];
  }

  /**
   * Checks if organism exists in DB table 'chado.organism'.
   *
   * Function db_select() is slower than db_query().
   * https://stackoverflow.com/a/21481960/1041470
   *
   * @param int $organism_id
   *   Organism Id.
   *
   * @return bool
   *   Returns TRUE if record was found in DB table and FALSE otherwise.
   */
  public static function isExists($organism_id) {
    $sql = 'select organism_id from {' . static::TABLE . '} where organism_id = :id limit 1';
    return (bool) (db_query($sql, [':id' => $organism_id])->fetchField());
  }

  /**
   * Saves Taxonomy Id into db.
   *
   * Existing record won't be updated. See ::update().
   *
   * @param int $organism_id
   *   Organism Id.
   * @param int $taxonomy_id
   *   Optional. Taxonomy Id.
   *   Insert NULL when organism wasn't found to avoid the same API-requests.
   *
   * @return bool
   *   Returns TRUE if new record was inserted and FALSE otherwise.
   */
  public static function save($organism_id, $taxonomy_id = NULL) {
    if (empty($organism_id) || static::isExists($organism_id)) {
      return FALSE;
    }
    $fields = [
      'organism_id' => $organism_id,
      'taxonomy_id' => $taxonomy_id,
    ];
    db_insert(static::TABLE)->fields($fields)->execute();
    return TRUE;
  }

  /**
   * Updates existing Taxonomy Id into db.
   *
   * @param int $organism_id
   *   Organism Id.
   * @param int $taxonomy_id
   *   Optional. Taxonomy Id.
   *   Insert NULL when organism wasn't found to avoid the same requests.
   *
   * @return bool
   *   Returns TRUE if record was inserted/updated and FALSE otherwise.
   */
  public static function update($organism_id, $taxonomy_id = NULL) {
    if (empty($organism_id) | !static::isExists($organism_id)) {
      return FALSE;
    }
    db_update(static::TABLE)
      ->condition('organism_id', $organism_id)
      ->fields(['taxonomy_id' => $taxonomy_id])
      ->execute();
    return TRUE;
  }

  /**
   * Loads Taxonomy Id from DB.
   *
   * @param int $organism_id
   *   Organism id.
   *
   * @return int
   *   Returns Taxonomy Id.
   */
  public static function load($organism_id) {
    return db_select(static::TABLE, 't')
      ->fields('t', ['taxonomy_id'])
      ->range(0, 1)
      ->condition('organism_id', $organism_key)
      ->execute()
      ->fetchField();
  }

}
