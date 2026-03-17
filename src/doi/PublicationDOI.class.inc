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

  // Tag used to label data received from Google Scholar website.
  // https://scholar.google.com/
  // Stored: PublicationDOI::TABLE, column 'source'.
  const SOURCE_GS = 'Google Scholar';

  // Tag used to label data received from PubMed website.
  // https://pubmed.ncbi.nlm.nih.gov/
  // Stored: PublicationDOI::TABLE, column 'source'.
  const SOURCE_PM = 'PubMed';

  /**
   * Checks if the record for given DOI exists in DB table.
   *
   * @param string $doi
   *   DOI.
   * @param string $source
   *   Source of the publication data.
   *   Possible values: see class's constants SOURCE_*.
   *
   * @return bool
   *   Returns TRUE if record exists and FALSE otherwise.
   *
   * @throws Exception
   *   When $source is invalid.
   */
  public static function isExists($doi, $source = NULL) {
    $query = db_select(self::TABLE, 't')
      ->fields('t', ['doi'])
      ->condition('doi', $doi)
      ->range(0, 1);
    if (!is_null($source)) {
      if (!in_array($source, self::getSourceList())) {
        $message = 'Invalid source: ' . check_plain($source);
        throw new Exception($message);
      }
      $query->condition('source', $source);
    }
    return (bool) $query->execute()->fetchField();
  }

  /**
   * Gets list of the source websites for the publication data.
   *
   * @return array
   *   Returns list of the source's tags (not URLs).
   */
  public static function getSourceList() {
    return [
      PublicationDOI::SOURCE_GS,
      PublicationDOI::SOURCE_PM
    ];
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
    if (!empty($data) && !empty($data['doi'])) {
      $data['source'] = $data['source'] ?? PublicationDOI::SOURCE_GS;
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
    return FALSE;
  }

  /**
   * Loads publication data from DB table.
   *
   * @param string $doi
   *   DOI (Digital Object Identifier).
   * @param string $source
   *   Source of the publication data.
   *   Possible values: see class's constants SOURCE_*.
   *
   * @return array
   *   Returns record from DB table for given DOI if exists.
   *   Returns empty array if record wasn't found.
   *
   * @throws Exception
   *   When $source is invalid.
   */
  public static function load($doi, $source = NULL) {
    // Build query.
    $query = db_select(self::TABLE, 't')
      ->fields('t', [])
      ->condition('doi', $doi)
      ->range(0, 1);
    // Build 'source' field.
    if (is_null($source)) {
      $source = PublicationDOI::SOURCE_GS;
    }
    else {
      if (!in_array($source, self::getSourceList())) {
        $message = 'Invalid source: ' . check_plain($source);
        throw new Exception($message);
      }
    }
    $query->condition('source', $source);
    // Get data.
    $data = $query->execute()->fetchAssoc();
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
   * Table name: PublicationDOI::TABLE.
   *
   * Usage example:
   *   $field_list = array_keys(PublicationDOI::getSchema()['fields']);
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
          'description' => 'Source of the data: "Google Scholar", "PubMed", etc.',
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
