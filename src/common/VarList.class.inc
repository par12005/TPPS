<?php

/**
 * @file
 * Class 'VarList' which manages arrays.
 *
 * Each variable which stores list has the same prefix.
 * Values could be integer and string but not arrays/objects.
 * Key-value list could be managed.
 *
 * @todo Implemnt method to get all variabled with this prefix.
 */

/**
 * Manages lists in Drupal variables.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class VarList {

  /**
   * The prefix for Drupal variables used by this class.
   */
  const VAR_PREFIX = 'var_list_';

  /**
   * Adds new item to a list.
   *
   * Works like 'merge' (not update or insert).
   *
   * @param string $list_name
   *   The name of the list.
   * @param mixed $value
   *   The value to add.
   * @param mixed $key
   *   Optional. Key.
   */
  public static function add($list_name, $value, $key = NULL) {
    $value = $value;
    $list = self::getList($list_name);
    if (is_null($key)) {
      if (!in_array($value, $list, TRUE)) {
        $list[] = $value;
      }
    }
    else {
      $list[$key] = $value;
    }
    self::saveList($list_name, $list);
  }

  /**
   * Removes an integer from a list.
   *
   * @param string $list_name
   *   The name of the list.
   * @param mixed $value
   *   The value to remove.
   * @param mixed $key
   *   Optional. Key.
   *
   * @return bool
   *   TRUE if the value was removed, FALSE if it wasn't found.
   */
  public static function remove($list_name, $value, $key = NULL) {
    $list = self::getList($list_name);
    if (is_null($key)) {
      $key = array_search($value, $list, TRUE);
      if ($key === FALSE) {
        return FALSE;
      }
    }
    else {
      if (!isset($list[$key])) {
        return FALSE;
      }
    }
    unset($list[$key]);
    self::saveList($list_name, $list);
    return TRUE;
  }

  /**
   * Searches for an integer in a list.
   *
   * @param string $list_name
   *   The name of the list.
   * @param mixed $value
   *   The value to search for.
   * @param mixed $key
   *   Optional. Key.
   *
   * @return int|FALSE
   *   The index of the value if found, FALSE otherwise.
   */
  public static function search($list_name, $value, $key = NULL) {
    $list = self::getList($list_name);

    if (is_null($key)) {
      return array_search($value, $list);
    }
    else {
      return ($list[$key] ?? NULL);
    }
  }

  /**
   * Checks if an integer exists in a list.
   *
   * @param string $list_name
   *   The name of the list.
   * @param mixed $value
   *   The value to check for.
   * @param mixed $key
   *   Optional. Key.
   *
   * @return bool
   *   TRUE if the value exists, FALSE otherwise.
   */
  public static function isExists($list_name, $value, $key = NULL) {
    return (bool) (self::search($list_name, $value, $key) !== FALSE);
  }

  /**
   * Retrieves the full list of integers.
   *
   * @param string $list_name
   *   The name of the list.
   *
   * @return array
   *   An array of integers.
   */
  public static function getList($list_name) {
    $list = variable_get(self::VAR_PREFIX . $list_name, []);
    return is_array($list) ? $list : [];
  }

  /**
   * Saves the list of integers to a Drupal variable.
   *
   * @param string $list_name
   *   The name of the list.
   * @param array $list
   *   The array of integers to save.
   */
  public static function saveList($list_name, array $list) {
    $var_name = self::VAR_PREFIX . $list_name;
    variable_set($var_name, $list);
  }

  /**
   * Deletes an entire list.
   *
   * @param string $list_name
   *   The name of the list.
   */
  public static function deleteList($list_name) {
    $var_name = self::VAR_PREFIX . $list_name;
    variable_del($var_name);
  }

  /**
   * Clears all integers from a list.
   *
   * @param string $list_name
   *   The name of the list.
   */
  public static function clearList($list_name) {
    self::saveList($list_name, []);
  }

  /**
   * Retrieves all lists managed by this class.
   *
   * @return array
   *   An associative array keyed by list name (without the prefix),
   *   where each value is an array of integers.
   */
  public static function getAllLists() {
    $result = [];
    $prefix_length = strlen(self::VAR_PREFIX);
    $query = db_select('variable', 'v')
      ->fields('v', ['name', 'value'])
      ->condition('name', db_like(self::VAR_PREFIX) . '%', 'LIKE');
    $records = $query->execute()->fetchAll();
    foreach ($records as $record) {
      $list_name = substr($record->name, $prefix_length);
      $value = unserialize($record->value);
      $result[$list_name] = is_array($value) ? $value : [];
    }
    return $result;
  }

}
