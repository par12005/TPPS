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
   * List of the supported sources.
   *
   * Key is a short name of source and value is a class name.
   */
  const SOURCE_LIST = [
    'ncbi' => 'NcbiTaxonomy',
    'gbif' => 'GbifSpeciesApi',
  ];

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
   * @param string|null $source
   *   Source of the Taxonomy Id. See static::SOURCE_LIST.
   *   If NULL then all records will be removed.
   */
  public static function remove($source = NULL) {
    $query = db_delete(static::TABLE);
    if (static::isSourceExist($source)) {
      $query->condition('source', $source);
    }
    $query->execute();
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
   * @param string $source
   *   Source of the Taxonomy Id.
   *   Possible values are:
   *   - 'ncbi' (NcbiTaxonomy::KEY),
   *   - 'gbif' (GbifSpeciesApi::KEY).
   * @param bool $debug_mode
   *   Optional. Default is FALSE.
   *   When TRUE only first 10 items will be in resulting set of data.
   *
   * @return array
   *   Returns list of organism ids.
   */
  public static function getMissingOrganismIdList($source, $debug_mode = FALSE) {
    $query = db_select('chado.organism', 'o')
      ->fields('o', ['organism_id'])
      ->orderBy('o.organism_id');
    $query->leftJoin(
      static::TABLE, 'tot',
      'o.organism_id = tot.organism_id AND tot.source = :source',
      [':source' => $source]
    );
    $query->isNull('tot.organism_id');

    // Check if organism name is not a synonym.
    $query->leftJoin('chado.organismprop', 'op',
      'o.organism_id = op.organism_id
        AND op.type_id = :type_id
        AND op.rank = 0',
      [':type_id' => OrganismSynonym::CVTERM_ID]
    );
    if (!empty($source_class = static::getSourceClassName($source))) {
      // Get list of the synonym organism ids (not primary id).
      $accepted_synonym_id_list = array_keys(
        $source_class::getAcceptedSynonymList()
      );
      if (!empty($accepted_synonym_id_list)) {
        $query->condition('o.organism_id', $accepted_synonym_id_list, 'NOT IN');
      }
    }
    // 'chado.organismprop.value' is not numeric.
    $query->where("op.value IS NULL");

    if ($debug_mode ?? FALSE) {
      $query->range(0, 10);
    }
    return $query->execute()->fetchCol();
  }

  /**
   * Gets name of the class by short source name.
   *
   * @param string $source
   *   Source of the data. Possible values: 'ncbi', 'gbif'.
   *
   * @return string|null
   *   Returns name of the class or NULL if match wasn't found.
   */
  public static function getSourceClassName($source) {
    return static::SOURCE_LIST[$source] ?? NULL;
  }

  /**
   * Checks if source is exists and supported.
   *
   * @param string $source
   *   Short name of the source. See static::SOURCE_LIST.
   *
   * @return bool
   *   Returns TRUE if exists and FALSE otherwise.
   */
  public static function isSourceExist($source) {
    return (bool) (!empty($source) && isset(static::SOURCE_LIST[$source]));
  }

  /**
   * Clears accepted synonym list for given source.
   *
   * @param string $source
   *   Short name of the source.
   *
   * @return bool
   *   Returns TRUE if list was really cleared and FALSE otherwise.
   */
  public static function clearAcceptedSynonymList($source) {
    if (!empty($class_name = static::getSourceClassName($source))) {
      return $class_name::clearAcceptedSynonymList();
    }
    return FALSE;
  }

  /**
   * Gets list of organism id and related taxonomy id for single $source.
   *
   * @param string $source
   *   Source of the Taxonomy Id.
   *   Possible values are:
   *   - 'ncbi' (NcbiTaxonomy::KEY),
   *   - 'gbif' (GbifSpeciesApi::KEY).
   * @param bool $debug_mode
   *   Optional. Default is FALSE.
   *   When TRUE only first 10 items will be in resulting set of data.
   *
   * @return array
   *   Returns list of the organisms and their Taxonomy Id.
   *   Format (associative array):
   *   [
   *     <organism_id> => <taxonomy_id>,
   *   ]
   */
  public static function getIdList($source, $debug_mode = FALSE) {
    // Get all organisms.
    $query = db_select('chado.organism', 'o')
      ->fields('o', ['organism_id'])
      ->orderBy('o.organism_id');
    $query->leftJoin(static::TABLE, 'tot', 'o.organism_id = tot.organism_id');
    $query->condition('tot.source', $source);
    $query->isNotNull('tot.taxonomy_id');
    $query->addField('tot', 'taxonomy_id');

    if ($debug_mode) {
      $query->range(0, 11);
    }
    return $query->execute()->fetchAllKeyed();
  }

}
