<?php

/**
 * @file
 * Contains \StateList.
 *
 * Buffers all list reads and mutations in static memory during a request
 * and flushes changes once on request shutdown to a dedicated database table
 * ({state_list}) or a Drupal cache bin ({cache_state_list}).
 */

/**
 * Manages persistent lists with in-memory request buffering in Drupal 7.
 *
 * Completely eliminates the use of Drupal 7 variables for list storage.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class StateList {

  /**
   * Database table name for the dedicated table backend.
   */
  const TABLE_NAME = 'state_list';

  /**
   * Persistent storage backends:
   * - STORAGE_TABLE: Stores lists in the dedicated {state_list} table (default).
   *   Permanent across cache clears and isolated from bootstrap variables.
   * - STORAGE_CACHE: Stores lists in Drupal's Cache API (cache_get / cache_set)
   *   using {cache_state_list} (or Memcache/Redis).
   */
  const STORAGE_TABLE = 'table';
  const STORAGE_CACHE = 'cache';

  /**
   * Active persistent storage backend.
   *
   * @var string
   */
  protected static $storageBackend = self::STORAGE_TABLE;

  /**
   * Cache bin used when $storageBackend is set to self::STORAGE_CACHE.
   *
   * @var string
   */
  protected static $cacheBin = 'cache_state_list';

  /**
   * In-memory buffer of loaded/modified lists for the current request.
   *
   * @var array<string, array>
   */
  protected static $lists = [];

  /**
   * Tracks list names that have unsaved changes in memory.
   *
   * @var array<string, bool>
   */
  protected static $dirty = [];

  /**
   * Tracks list names that were deleted during the current request.
   *
   * @var array<string, bool>
   */
  protected static $deleted = [];

  /**
   * Whether the shutdown flush callback has been registered for this request.
   *
   * @var bool
   */
  protected static $shutdownRegistered = FALSE;

  /**
   * Whether getAllLists() has already loaded all existing lists into memory.
   *
   * @var bool
   */
  protected static $allLoaded = FALSE;

  /**
   * Configures the persistent storage backend ('table' or 'cache').
   *
   * @param string $backend
   *   One of self::STORAGE_TABLE or self::STORAGE_CACHE.
   * @param string $cache_bin
   *   Optional cache bin when using self::STORAGE_CACHE
   *   (defaults to 'cache_state_list').
   */
  public static function setStorageBackend($backend, $cache_bin = 'cache_state_list') {
    $allowed = [
      self::STORAGE_TABLE,
      self::STORAGE_CACHE,
    ];
    if (in_array($backend, $allowed, TRUE)) {
      self::$storageBackend = $backend;
      self::$cacheBin = $cache_bin;
    }
  }

  /**
   * Adds a new item to a list in memory (flushed to storage on shutdown).
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
    $list = self::getList($list_name);
    if (is_null($key)) {
      if (!in_array($value, $list, TRUE)) {
        $list[] = $value;
        self::saveList($list_name, $list);
      }
    }
    else {
      if (!array_key_exists($key, $list) || $list[$key] !== $value) {
        $list[$key] = $value;
        self::saveList($list_name, $list);
      }
    }
  }

  /**
   * Gets value from the list.
   *
   * @param string $list_name
   *   The name of the list.
   * @param mixed $key
   *   Key.
   */
  public static function get($list_name, $key) {
    $list = self::getList($list_name);
    return $list[$key] ?? NULL;
  }

  /**
   * Removes an item from a list in memory (flushed to storage on shutdown).
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
   * Searches for a value or key in a list.
   *
   * @param string $list_name
   *   The name of the list.
   * @param mixed $value
   *   The value to search for.
   * @param mixed $key
   *   Optional. Key.
   *
   * @return mixed
   *   The key of the value if $key is NULL (or FALSE if not found), or the
   *   value at $key if $key is provided (or FALSE if not found).
   */
  public static function search($list_name, $value, $key = NULL) {
    $list = self::getList($list_name);

    if (is_null($key)) {
      return array_search($value, $list);
    }
    else {
      return array_key_exists($key, $list) ? $list[$key] : FALSE;
    }
  }

  /**
   * Checks if a value or key exists in a list.
   *
   * @param string $list_name
   *   The name of the list.
   * @param mixed $value
   *   The value to check for.
   * @param mixed $key
   *   Optional. Key.
   *
   * @return bool
   *   TRUE if the value/key exists, FALSE otherwise.
   */
  public static function isExists($list_name, $value, $key = NULL) {
    return (bool) (self::search($list_name, $value, $key) !== FALSE);
  }

  /**
   * Retrieves the full list from the in-memory buffer or persistent storage.
   *
   * @param string $list_name
   *   The name of the list.
   *
   * @return array
   *   An array of items in the list.
   */
  public static function getList($list_name) {
    if (array_key_exists($list_name, self::$lists)) {
      return self::$lists[$list_name];
    }
    if (isset(self::$deleted[$list_name])) {
      return [];
    }

    $list = [];

    if (self::$storageBackend === self::STORAGE_TABLE) {
      $serialized = db_select(self::TABLE_NAME, 'sl')
        ->fields('sl', ['value'])
        ->condition('name', $list_name)
        ->execute()
        ->fetchField();
      if ($serialized !== FALSE) {
        $unserialized = @unserialize($serialized);
        $list = is_array($unserialized) ? $unserialized : [];
      }
    }
    elseif (self::$storageBackend === self::STORAGE_CACHE) {
      $cache = cache_get($list_name, self::$cacheBin);
      if ($cache && isset($cache->data) && is_array($cache->data)) {
        $list = $cache->data;
      }
    }

    self::$lists[$list_name] = $list;
    return $list;
  }

  /**
   * Buffers the list in memory and schedules a single shutdown flush.
   *
   * @param string $list_name
   *   The name of the list.
   * @param array $list
   *   The array of items to save.
   */
  public static function saveList($list_name, array $list) {
    self::$lists[$list_name] = $list;
    self::$dirty[$list_name] = TRUE;
    unset(self::$deleted[$list_name]);

    self::ensureShutdownRegistered();
  }

  /**
   * Marks an entire list for deletion (flushed on shutdown).
   *
   * @param string $list_name
   *   The name of the list.
   */
  public static function deleteList($list_name) {
    unset(self::$lists[$list_name], self::$dirty[$list_name]);
    self::$deleted[$list_name] = TRUE;

    self::ensureShutdownRegistered();
  }

  /**
   * Clears all items from a list.
   *
   * @param string $list_name
   *   The name of the list.
   */
  public static function clearList($list_name) {
    self::saveList($list_name, []);
  }

  /**
   * Retrieves all lists managed by this class, merging buffered changes.
   *
   * @return array
   *   An associative array keyed by list name.
   */
  public static function getAllLists() {
    if (!self::$allLoaded) {
      if (self::$storageBackend === self::STORAGE_TABLE) {
        $records = db_select(self::TABLE_NAME, 'sl')
          ->fields('sl', ['name', 'value'])
          ->execute()
          ->fetchAll();
        foreach ($records as $record) {
          $list_name = $record->name;
          if (!array_key_exists($list_name, self::$lists) && !isset(self::$deleted[$list_name])) {
            $value = @unserialize($record->value);
            self::$lists[$list_name] = is_array($value) ? $value : [];
          }
        }
      }
      elseif (self::$storageBackend === self::STORAGE_CACHE) {
        $cids = db_select(self::$cacheBin, 'c')
          ->fields('c', ['cid'])
          ->execute()
          ->fetchCol();
        if (!empty($cids)) {
          $cached_items = cache_get_multiple($cids, self::$cacheBin);
          foreach ($cached_items as $cid => $item) {
            if (!array_key_exists($cid, self::$lists) && !isset(self::$deleted[$cid])) {
              self::$lists[$cid] = (isset($item->data) && is_array($item->data)) ? $item->data : [];
            }
          }
        }
      }

      self::$allLoaded = TRUE;
    }

    return self::$lists;
  }

  /**
   * Registers the shutdown function once per request to flush buffered changes.
   */
  protected static function ensureShutdownRegistered() {
    if (!self::$shutdownRegistered) {
      drupal_register_shutdown_function([__CLASS__, 'flush']);
      self::$shutdownRegistered = TRUE;
    }
  }

  /**
   * Flushes all dirty and deleted lists to persistent storage once.
   *
   * Called automatically at the end of the request via
   * drupal_register_shutdown_function(), or can be called manually if needed.
   */
  public static function flush() {
    if (empty(self::$dirty) && empty(self::$deleted)) {
      return;
    }

    if (self::$storageBackend === self::STORAGE_TABLE) {
      $now = defined('REQUEST_TIME') ? REQUEST_TIME : time();
      foreach (array_keys(self::$dirty) as $list_name) {
        if (array_key_exists($list_name, self::$lists)) {
          db_merge(self::TABLE_NAME)
            ->key(['name' => $list_name])
            ->fields([
              'value' => serialize(self::$lists[$list_name]),
              'updated' => $now,
            ])
            ->execute();
        }
      }

      if (!empty(self::$deleted)) {
        db_delete(self::TABLE_NAME)
          ->condition('name', array_keys(self::$deleted), 'IN')
          ->execute();
      }
    }
    elseif (self::$storageBackend === self::STORAGE_CACHE) {
      foreach (array_keys(self::$dirty) as $list_name) {
        if (array_key_exists($list_name, self::$lists)) {
          cache_set($list_name, self::$lists[$list_name], self::$cacheBin, CACHE_PERMANENT);
        }
      }

      foreach (array_keys(self::$deleted) as $list_name) {
        cache_clear_all($list_name, self::$cacheBin);
      }
    }

    self::$dirty = [];
    self::$deleted = [];
  }

}
