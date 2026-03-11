<?php

/**
 * @file
 * Contains 'PublicationDOI' class.
 */

/**
 * Manages publication DOI information.
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
   *   Optional. The source of publication data.
   *   Possible values: 'gemini', 'custom'.
   *
   * @return bool
   *   Returns TRUE if record exists and FALSE otherwise.
   */
  public static function isExists($doi, $source = '') {
    $query = db_select(self::TABLE, 't')
      ->fields('t', ['doi'])
      ->condition('doi', $doi)
      ->range(0, 1);
    if ($source) {
      $query->condition('source', $source);
    }
    return (bool) $query->execute()->fetchField();
  }

  /**
   * Saves publication data into DB table.
   *
   * @param array $fields
   *   See self::TABLE columns.
   *
   * @return bool
   *   Returns TRUE in case of success and FALSE otherwise.
   *
   * @throws Exception
   *   When record for given $fields['doi'] and $fields['source'] already exists.
   */
  public static function save(array $fields = []) {
    if (!empty($fields)) {
      $is_record_exists = PublicationDOI::isExists($fields['doi'], $fields['source']);
      if ($is_record_exists) {
        $message = 'Record already exists. DOI: @doi';
        $params = ['@doi' => $fields['doi']];
        throw new Exception(t($message, $params));
      }
      else {
        db_insert(self::TABLE)->fields($fields)->execute();
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Loads publication data from DB table.
   *
   * @param string $doi
   *   DOI (Digital Object Identifier).
   *
   * @return array
   *   Returns record from DB table for given DOI if exists.
   *   Returns empty array if record wasn't found.
   */
  public static function load($doi) {
    $data = db_select(self::TABLE, 't')
      ->fields('t', [])
      ->condition('doi', $doi)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
    if (empty($data)) {
      return [];
    }
    return $data;
  }

  /**
   * Gets DB table schema.
   *
   * Used in hook_install().
   * Table name: PublicationDOI::TABLE.
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
        'doi' => ['doi'],
      ],
    ];
  }

}
