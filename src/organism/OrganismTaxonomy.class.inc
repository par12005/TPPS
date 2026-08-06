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
        'organism_name' => [
          'description' => 'Organism Name.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
        ],
        'taxonomy_id' => [
          'description' => 'Taxomony Id.',
          'type' => 'int',
          'size' => 'big',
          'not null' => FALSE,
        ],
        'name' => [
          'description' => 'Short organism name.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
        ],
        'full_name' => [
          'description' => 'Full organism name.',
          'type' => 'varchar',
          'size' => 'normal',
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
   * @param int $organism_id
   *   Organism Id.
   * @param string $organism_name
   *   Orgnaism name.
   * @param int $taxonomy_id
   *   Optional. Taxonomy Id.
   *   Insert NULL when organism wasn't found to avoid the same requests.
   * @param string $name
   *   Short organism name. E.g., 'Acacia angusta'.
   * @param string $full_name
   *   Full organism name.
   *   E.g., 'Acacia angusta Maiden & Blakely, 1927'.
   *
   * @return bool
   *   Returns TRUE if record was inserted/updated and FALSE otherwise.
   */
  public static function save($organism_id, $organism_name, $taxonomy_id = NULL, $name = NULL, $full_name = NULL) {
    $fields = [
      'organism_id' => $organism_id,
      'organism_name' => $organism_name,
      'taxonomy_id' => $taxonomy_id,
      'name' => $name,
      'full_name' => $full_name,
    ];
    if (
      !count($fields)
      || (empty($fields['organism_id']) && empty($fields['organism_name']))
    ) {
      return FALSE;
    }

    if (static::isExists($organism_id)) {
      // Update.
      $query = db_update(static::TABLE);
      if (!empty($organism_id)) {
        unset($fields['organism_id']);
        $query->condition('organism_id', $organism_id);
      }
      else {
        unset($fields['organism_name']);
        $query->condition('organism_name', $organism_name);
      }
      $query->fields($fields)->execute();
    }
    else {
      // Insert.
      db_insert(static::TABLE)->fields($fields)->execute();
    }
    return TRUE;
  }

  /**
   * Loads Taxonomy Id from DB.
   *
   * @param mixed $organism_key
   *   Organism id (int) or Organism name (string).
   *
   * @return int
   *   Returns Taxonomy Id.
   */
  public static function load($organism_key) {
    $query = db_select(static::TABLE, 't')
      ->fields('t', ['taxonomy_id'])
      ->range(0, 1);
    if (is_numeric($organism_key)) {
      // Organism id.
      $query->condition('organism_id', $organism_key);
    }
    else {
      $query->condition(
        db_or()
          ->condition('organism_name', $organism_key)
          ->condition('name', $organism_key)
          ->condition('full_name', $organism_key)
      );
    }
    return $query->execute()->fetchField();
  }

}
