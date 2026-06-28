<?php

/**
 * @file
 * Contains 'RestAPIRequest' class.
 *
 * Request has 'payload'.
 * Response has 'body'.
 */

/**
 * Manages Rest API Requests stored in database.
 *
 * To avoid request caching make URL unique (add unique param like 'z=' . time()).
 *
 * To get db table fields:
 *   $fields = array_keys(RestAPIRequest::getSchema()['fields']);
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestAPIRequest {

  // Name of the DB table which stores requests.
  const TABLE = 'tpps_rest_api_request';

  /**
   * Checks if record already exists in DB.
   *
   * @param string $hash
   *   Request's data md5 hash.
   *
   * @return bool
   *   Returns TRUE if record for given $hash found in DB.
   *   Returns FALSE otherwise.
   */
  public static function isExists($hash) {
    return (bool) static::getRequestId($hash);
  }

  /**
   * Gets request id by hash.
   *
   * @param string $hash
   *   Request's data md5 hash.
   *
   * @return mixed
   *   Returns integer request id if record for given $hash found.
   *   Returns FALSE otherwise.
   */
  public static function getRequestId($hash) {
    return db_select(static::TABLE, 't')
      ->fields('t', ['request_id'])
      ->condition('hash', $hash)
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
   * @return mixed
   *   Returns last insert id (integer) when new record was created.
   *   Returns NULL if record with the same hash already exists.
   *
   * @thrown Exception
   *   When $context['request_id'] is empty.
   */
  public static function save(array $context = []) {
    if (empty($context['hash'])) {
      throw new Exception("Empty 'hash'.");
    }
    if ($request_id = static::getRequestId($context['hash'])) {
      watchdog('tpps_rest_api', 'Record exists: @request_id.',
        ['@request_id' => $request_id]
      );
      return $request_id;
    }
    else {
      $field_list = array_keys(RestAPIRequest::getSchema()['fields']);
      foreach ($field_list as $field) {
        if ($field == 'request_id') {
          continue;
        }
        elseif ($field == 'created') {
          $data[$field] = $context[$field] ?? time();
        }
        elseif (
          in_array($field, ['payload', 'header'])
          && !is_string($context[$field])
        ) {
          $data[$field] = json_encode($context[$field], TRUE);
        }
        else {
          $data[$field] = $context[$field] ?? NULL;
        }
      }
      return db_insert(static::TABLE)->fields($data)->execute();
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
   * @return mixed
   *   Returns FALSE if record wasn't found in DB.
   *   Returns associative array with keys:
   *     'request_id',
   *     'created',
   *     'code',
   *     'duration',
   *     'info',
   *     'body',
   */
  public static function load($request_id) {
    $data = db_select(static::TABLE, 't')
      ->fields('t', [])
      ->condition('request_id', $request_id)
      ->range(0, 1)
      ->orderBy('created', 'DESC')
      ->execute()
      ->fetchAssoc();

    if (!empty($data['payload'])) {
      $data['payload'] = json_decode($data['payload'], 1);
    }
    if (!empty($data['header'])) {
      $data['header'] = json_decode($data['header'], 1);
    }

    return $data;
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
          'type' => 'serial',
          'size' => 'normal',
          'unsigned' => TRUE,
          'not null' => TRUE,
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
        'hash' => [
          'description' => "md5 hash of the request's data.",
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
      ],
      'unique keys' => ['hash' => ['hash']],
      'primary key' => ['request_id'],
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
