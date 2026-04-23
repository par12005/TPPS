<?php

/**
 * @file
 * Contains 'RestAPIRequest' class.
 */

/**
 * Manages Rest API Requests stored in database.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestAPIRequest {

  // Name of the DB table which stores requests.
  const TABLE = 'tpps_rest_api_request';

  /**
   * Checks if the request exists in DB.
   *
   * @param string $request_id
   *   Unique request identificator.
   *
   * @return bool
   *   Returns TRUE if record for given $request_id found in DB.
   *   Returns FALSE otherwise.
   */
  public static function isExists($request_id) {
    return (bool) db_select(static::TABLE, 't')
      ->fields('t', ['request_id'])
      ->condition('request_id', $request_id)
      ->range(0, 1)
      ->orderBy('created', 'DESC')
      ->execute()
      ->fetchField();
  }

  /**
   * Saves RestAPI Request.
   *
   * Called from class 'RestAPI'.
   * Class 'RestAPI' decides if response from DB will be used.
   * This method just saves any request.
   *
   * @param array $context
   *   Request's context.
   *   Keys: see RestAPI->doRequest().
   *   'tag' is optional.
   *
   * @return bool
   *   Returns TRUE when new record was created and FALSE when the same
   *   request already exists in DB table.
   *
   * @thrown Exception
   *   When $context['request_id'] is empty.
   */
  public static function save(array $context = []) {
    $key = 'request_id';
    if (empty($context['request_id'])) {
      throw new Exception("Empty 'request_id'.");
    }
    if (static::isExists($context['request_id'])) {
      return FALSE;
    }
    else {
      $data = $context;
      unset($data['cache_request']);
      $data['created'] = $data['created'] ?? time();
      // Convert arrays into JSON format.
      foreach (['payload', 'header'] as $name) {
        $data[$name] = json_encode($context[$name], TRUE);
      }
      $data['use_cached_response'] = (int) $data['use_cached_response'];
      db_insert(static::TABLE)->fields($data)->execute();
      return TRUE;
    }
  }

  /**
   * Loads only valid Rest API response from database.
   *
   * Valis response:
   * 1. code 2xx
   * 2. Not empty body.
   * 3. Latest response (no 'created' date).
   *
   * To load invalid responses use RestAPIResponse::load() method.
   *
   * Usage example:
   * extract(RestAPIResponse::loadValid($request_id));
   *
   * @param string $request_id
   *   Unique request identificator.
   *
   * @return array
   *   Returns associative array with keys:
   *     'request_id',
   *     'created',
   *     'code',
   *     'duration',
   *     'info',
   *     'response',
   */
  public static function load($request_id) {
    return db_select(static::TABLE, 't')
      ->fields('t', [])
      ->condition('request_id', $request_id)
      ->range(0, 1)
      ->orderBy('created', 'DESC')
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Gets db table schema.
   *
   * Table to log requests made by 'RestAPI' class.
   *
   * @return array
   *   Returns array for hook_schema().
   */
  public static function getSchema() {
    return [
      'description' => "Stores REST API requests.",
      'fields' => [
        // Request Id is an md5 hash of all request's data to be able to find
        // exactly the same request in log.
        'request_id' => [
          'description' => 'The unique request identifier.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'tag' => [
          'description' => 'Request description for humans.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
        ],
        'cache_response' => [
          'type' => 'int',
          'size' => 'tiny',
          'not null' => TRUE,
          'default' => 0,
          'description' => 'Controls if response could be cached. 0 = FALSE, 1 = TRUE.',
        ],
        'use_cached_response' => [
          'type' => 'int',
          'size' => 'tiny',
          'not null' => TRUE,
          'default' => 0,
          'description' => 'Controls if cached response could be used. 0 = FALSE, 1 = TRUE.',
        ],
        'created' => [
          'description' => 'The first time request was created. Format: UNIX timestamp.',
          'type' => 'int',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'method' => [
          // @TODO Use 'enum'?
          'description' => 'HTTP method: post, get, put, delete.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'url' => [
          'description' => 'API Endpoint.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'payload' => [
          'description' => "POST request's payload. Format: JSON-string.",
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'header' => [
          'description' => 'HTTP Headers. Format: JSON-string.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
      ],
      'unique keys' => [
        // It's possible that the same request will bring different API response.
        // For example response is time-based.
        'request_id_created' => ['request_id', 'created'],
      ],
    ];
  }

  /**
   * Gets all request ids.
   *
   * @return array
   *   Returns list of all the request Ids.
   */
  public static function getAll() {
    return db_select(static::TABLE, 't')
      ->fields('t', ['request_id'])
      ->execute()
      ->fetchCol('request_id');
  }

}
