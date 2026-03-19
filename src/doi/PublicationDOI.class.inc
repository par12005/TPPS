<?php

/**
 * @file
 * Contains static 'PublicationDOI' class.
 */

/**
 * Manages publication DOI information.
 *
 * To get lists of Publication/Dataset DOI see reports:
 * /admin/reports/tpps/doi/submission/publication.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PublicationDOI {

  // Name of the DB table where stored DOI publication information.
  const TABLE = 'tpps_publication_doi';

  /**
   * Checks if the record for given DOI exists in DB table.
   *
   * @param string $doi
   *   DOI.
   * @param string $source
   *   Required. Source of the publication data.
   *   Possible values: see DoiSerp::KEY.
   *
   * @return bool
   *   Returns TRUE if record exists and FALSE otherwise.
   *
   * @throws Exception
   *   When $source is invalid.
   */
  public static function isExists($doi, $source) {
    $query = db_select(self::TABLE, 't')
      ->fields('t', ['doi'])
      ->condition('doi', $doi)
      ->range(0, 1)
      ->condition('source', $source);
    return (bool) $query->execute()->fetchField();
  }

  /**
   * Saves publication data into DB table.
   *
   * @param array $data
   *   See self::TABLE columns.
   *
   * @return bool
   *   Returns TRUE in case of success and FALSE otherwise.
   *
   * @throws Exception
   *   When record for given $data['doi'] already exists.
   */
  public static function save(array $data = []) {
    if (empty($data) || empty($data['doi']) || empty($data['source'])) {
      return FALSE;
    }
    $is_record_exists = PublicationDOI::isExists($data['doi'], $data['source']);
    if ($is_record_exists) {
      $message = 'Record already exists. DOI: @doi, Source: @source';
      $params = [
        '@doi' => $data['doi'],
        '@source' => $data['source'],
      ];
      throw new Exception(t($message, $params));
    }
    else {
      db_insert(self::TABLE)->fields($data)->execute();
      return TRUE;
    }
  }

  /**
   * Loads publication data from DB table.
   *
   * @param string $doi
   *   Required. DOI (Digital Object Identifier).
   * @param string $source
   *   Required. Source of the publication data.
   *   Possible values: see DoiSerp::KEY.
   *
   * @return array
   *   Returns record from DB table for given DOI if exists.
   *   Returns empty array if record wasn't found.
   *
   * @throws Exception
   *   When $source is invalid.
   */
  public static function load($doi, $source) {
    // Build query.
    $data = db_select(static::TABLE, 't')
      ->fields('t', [])
      ->condition('doi', $doi)
      ->condition('source', $source)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
    // Output.
    if (empty($data)) {
      return [];
    }
    return $data;
  }

  /**
   * Gets DB table schema.
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
      'description' => 'Stores DOI publication information.',
      'fields' => [
        'doi' => [
          'description' => 'DOI (Digital Object Identifier).',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'source' => [
          'description' => 'Source of the data. See $serpProvider::KEY.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'authors' => [
          'description' => 'Authors (primary and secondary).',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'date' => [
          'description' => 'Publication Date.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'title' => [
          'description' => 'Publication Title.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'journal' => [
          'description' => 'Publication Journal.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'abstract' => [
          'description' => 'Publication Abstract.',
          // varchar/normal: mysql: 64KB, postgresql: 1GB.
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
      ],
      'unique keys' => [
        'doi_source' => ['doi', 'source'],
      ],
    ];
  }

}
