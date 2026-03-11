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

}
