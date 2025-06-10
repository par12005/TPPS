<?php

/**
 * @file
 * Contains 'ChadoDb' class.
 */

/**
 * Manages Chado DBs.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class ChadoDb {

  /**
   * Gets Db Id.
   *
   * @return int
   *   Returns DB Id for given database name.
   */
  public static function getId($name, $reset = FALSE) {
    $key = $name;
    $static_cache = &drupal_static(__METHOD__);
    if ($reset || !isset($static_cache[$key])) {
      $value = db_select('chado.db', 't')
        ->fields('t', ['db_id'])
        ->condition('name', $name)
        ->range(0, 1)
        ->execute()
        ->fetchField();
      $static_cache[$key] = $value;
    }
    return $static_cache[$key];
  }

}
