<?php

/**
 * @file
 * Contains class 'OrganismTaxonomy'.
 */

/**
 * Manages organism's taxonomy ids obtained from external taxonomy repositories.
 *
 * Repositories: NCBI, GBIF.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class OrganismTaxonomy {
  /**
   * Name of the table which stores Taxonomy Ids.
   */
  const TABLE = 'tpps_organism_taxonomy';

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
      'description' => "Organism's Taxonomy Id.",
      'fields' => [
        'organism_id' => [
          'description' => 'Organism Id.',
          'type' => 'int',
          'size' => 'big',
          'not null' => FALSE,
        ],
        'source' => [
          'description' => 'Source of the Taxonomy Id.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
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
        'organism_id_source' => ['organism_id', 'source'],
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
   * @param string $source
   *   Source of the Taxonomy Id. E.g., 'ncbi', 'gbif'.
   *
   * @return bool
   *   Returns TRUE if record was found in DB table and FALSE otherwise.
   */
  public static function isExists($organism_id, $source) {
    $sql = 'SELECT organism_id
      FROM {' . static::TABLE . '}
      WHERE organism_id = :organism_id AND source = :source
      LIMIT 1';
    $params = [
      ':organism_id' => $organism_id,
      ':source' => $source,
    ];
    return (bool) (db_query($sql, $params)->fetchField());
  }

  /**
   * Saves Taxonomy Id into db.
   *
   * Existing record won't be updated. See ::update().
   *
   * @param int $organism_id
   *   Organism Id.
   * @param string $source
   *   Source of the Taxonomy Id. E.g., 'ncbi', 'gbif'.
   * @param int $taxonomy_id
   *   Optional. Taxonomy Id.
   *   Insert NULL when organism wasn't found to avoid the same API-requests.
   *
   * @return bool
   *   Returns TRUE if new record was inserted and FALSE otherwise.
   */
  public static function save($organism_id, $source, $taxonomy_id = NULL) {
    if (
      empty($organism_id) || empty($source)
      || static::isExists($organism_id, $source)
    ) {
      return FALSE;
    }
    $fields = [
      'organism_id' => $organism_id,
      'source' => $source,
      'taxonomy_id' => $taxonomy_id,
    ];
    db_insert(static::TABLE)->fields($fields)->execute();
    return TRUE;
  }

  /**
   * Updates existing Taxonomy Id.
   *
   * @param int $organism_id
   *   Organism Id.
   * @param string $source
   *   Source of the Taxonomy Id. E.g., 'ncbi', 'gbif'.
   * @param int $taxonomy_id
   *   Optional. Taxonomy Id.
   *   Insert NULL when organism wasn't found to avoid the same requests.
   *
   * @return bool
   *   Returns TRUE if record was inserted/updated and FALSE otherwise.
   */
  public static function update($organism_id, $source, $taxonomy_id = NULL) {
    if (
      empty($source) || empty($organism_id)
      || !static::isExists($source, $organism_id)
    ) {
      return FALSE;
    }
    db_update(static::TABLE)
      ->condition('organism_id', $organism_id)
      ->condition('source', $source)
      ->fields(['taxonomy_id' => $taxonomy_id])
      ->execute();
    return TRUE;
  }

  /**
   * Loads Taxonomy Id from DB.
   *
   * @param int $organism_id
   *   Organism id.
   * @param string $source
   *   Source of the Taxonomy Id. E.g., 'ncbi', 'gbif'.
   *
   * @return array
   *   Returns associative array where:
   *   - key is $source and
   *   - value is the taxonomy id from this source.
   */
  public static function load($organism_id, $source = NULL) {
    $query = db_select(static::TABLE, 't')
      ->fields('t', ['source', 'taxonomy_id'])
      ->condition('organism_id', $organism_id);
    if (!empty($source)) {
      $query->condition('source', $source);
    }
    return $query->execute()->fetchAllKeyed();
  }

  /**
   * Gets list of organisms without NCBI/GBIF Taxonomy Id.
   *
   * @return array
   *   Returns list of organism ids.
   */
  public static function getMissingOrganismIdList() {
    $query = db_select('chado.organism', 'o')
      ->fields('o', ['organism_id'])
      ->orderBy('o.organism_id');
    $query->leftJoin(
      OrganismTaxonomy::TABLE, 'ot',
      'o.organism_id = ot.organism_id'
    );
    $query->isNull('ot.source');
    if ($debug_mode ?? FALSE) {
      $query->range(0, 2);
    }
    return $query->execute()->fetchCol();
  }

}
