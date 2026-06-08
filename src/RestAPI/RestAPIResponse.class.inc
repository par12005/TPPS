<?php

/**
 * @file
 * Contains 'RestAPIResponse' class.
 *
 * Note: Request and Response uses the same name 'created' but values could be
 * totally different.
 */

/**
 * Manages Rest API Responses stored in DB.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestAPIResponse {

  // Name of the DB table which stores responses.
  const TABLE = 'tpps_rest_api_response';

  /**
   * Saves RestAPI Response.
   *
   * No need to check if the same response exists because it's body could have
   * some time-based items (like request time or cache protection).
   *
   * @param array $context
   *   Request's context.
   *   Keys: see RestAPI->doRequest().
   *   To check if any response exists:
   *     RestAPIResponse::isExists($request_id).
   *   To check if valid response exists:
   *     RestAPIResponse::isValidExists($request_id).
   *
   * @return bool
   *   Returns TRUE in case of success and FALSE otherwise.
   *
   * @thrown Exception
   *   When $context['request_id'] is empty.
   */
  public static function save(array $context = []) {
    if (empty($context['request_id'])) {
      throw new Exception("Empty 'request_id'.");
    }

    $data = $context;
    unset($data['cache_request']);
    unset($data['cache_response']);
    unset($data['use_cached_response']);
    unset($data['request']);

    // Store Response data.
    // Note: $context['created'] is a moment when request (not response) was
    // created. Sometimes request would be created a long time ago but response
    // created just now.
    $data['created'] = time();
    $data['duration'] = intval(round($data['duration']));
    $data['body'] = json_encode($data['body'], TRUE);
    // Note: No need to check if the same response exists because it could
    // have time-based items (like request time) and this check will never
    // give any result. Just store every response.
    db_insert(static::TABLE)->fields($data)->execute();
    return TRUE;
  }

  /**
   * Checks if any response exists in DB.
   *
   * WARNING: Invalid requests also logged and will be counted..
   *
   * @param string $request_id
   *   Unique request identificator.
   * @param int $created
   *   Optional. Unix timestamp of the moment when response was requested.
   *   When empty then latest response will be loaded.
   *
   * @return bool
   *   Returns TRUE if record for given $request_id found in DB.
   *   Returns FALSE otherwise.
   */
  public static function isExists($request_id, $created = NULL) {
    return (bool) static::load($request_id, $created);
  }

  /**
   * Checks if valid response exists in DB.
   *
   * Valid request:
   * 1. Code 2xx
   * 2. Non-empty body of the response.
   *
   * Use method isExists() to get any response.
   *
   * @param string $request_id
   *   Unique request identificator.
   *
   * @return bool
   *   Returns TRUE if record for given $request_id found in DB.
   *   Returns FALSE otherwise.
   */
  public static function isValidExists($request_id) {
    return (bool) static::loadValid($request_id);
  }

  /**
   * Loads Rest API response from database.
   *
   * Uses static caching.
   * WARNING: Status code won't be checked and invalid responses will be loaded.
   *
   * Usage example:
   * extract(RestAPIResponse::load($request_id));
   *
   * @param string $request_id
   *   Unique request identificator.
   * @param int $created
   *   Optional. Unix timestamp of the moment when response was requested.
   *   When empty then latest response will be loaded.
   * @param bool $reset
   *   TRUE to reset static cache. Default is FALSE.
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
  public static function load($request_id, $created = NULL, $reset = FALSE) {

    $static_cache = &drupal_static(__METHOD__);
    $key = md5(json_encode(array_slice(func_get_args(), 0, -1)));
    if ($reset || !isset($static_cache[$key])) {
      $query = db_select(static::TABLE, 't')
        ->fields('t', [])
        ->condition('request_id', $request_id)
        ->range(0, 1)
        ->orderBy('created', 'DESC');
      if (!is_null($created)) {
        $query->condition('created', $created);
      }
      $result = $query->execute()->fetchAssoc();
      if (!empty($result)) {
        // Curl info doesn't store in DB so we just create empty variable.
        $result['info'] = [];
        $result['response'] = json_decode($result['body'], TRUE);
        unset($result['body']);
        $result['message'] = HttpCode::getMessage($result['code']);
      }
      $static_cache[$key] = $result;
    }

    if (function_exists('module_invoke_all')) {
      // tpps_rest_api_request_completed().
      $context = [
        'request_id' => $request_id,
        'response_id' => $static_cache[$key]['id'],
      ];
      module_invoke_all('rest_api_response_loaded', $context);
    }
    return $static_cache[$key];
  }

  /**
   * Loads only valid Rest API response from database.
   *
   * Uses static caching.
   * Valid response:
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
   * @param bool $reset
   *   TRUE to reset static cache. Default is FALSE.
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
  public static function loadValid($request_id, $reset = FALSE) {
    $static_cache = &drupal_static(__METHOD__);
    $key = md5(json_encode(array_slice(func_get_args(), 0, -1)));
    if ($reset || !isset($static_cache[$key])) {
      $result = db_select(static::TABLE, 't')
        ->fields('t', [])
        ->condition('request_id', $request_id)
        ->condition('code', 200, '>=')
        ->condition('code', 299, '<=')
        ->condition('body', '', '!=')
        ->range(0, 1)
        ->orderBy('created', 'DESC')
        ->execute()
        ->fetchAssoc();
      if (!empty($result)) {
        // Curl info doesn't store in DB so we just create empty variable.
        $result['info'] = [];
        $result['response'] = json_decode($result['body'], TRUE);
        unset($result['body']);
        $result['message'] = HttpCode::getMessage($result['code']);
      }
      $static_cache[$key] = $result;
    }

    if (function_exists('module_invoke_all')) {
      // tpps_rest_api_request_completed().
      $context = [
        'request_id' => $request_id,
        'response_id' => $static_cache[$key]['id'],
      ];
      module_invoke_all('rest_api_response_loaded', $context);
    }

    return $static_cache[$key];
  }

  /**
   * Loads Rest API response from database.
   *
   * WARNING: Status code won't be checked and invalid responses will be loaded.
   *
   * Usage example:
   * extract(RestAPIResponse::load($request_id));
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
  public static function loadAll($request_id) {
    $result = db_select(static::TABLE, 't')
      ->fields('t', [])
      ->condition('request_id', $request_id)
      // ->orderBy('created', 'DESC')
      ->execute()
      ->fetchAllAssoc('created', PDO::FETCH_ASSOC);

    foreach ($result as &$item) {
      // Curl info doesn't store in DB so we just create empty variable.
      $item['info'] = [];
      // @todo Use original 'body' instead of new 'response'.
      $item['response'] = json_decode($item['body'], TRUE);
      unset($item['body']);
      $item['message'] = HttpCode::getMessage($item['code']);
    }

    // @todo ?Should I add the hook to have watchdog record?
    return $result;
  }

  /**
   * Gets db table schema.
   *
   * Table to log responses received by 'RestAPI' class.
   *
   * @return array
   *   Returns array for hook_schema().
   */
  public static function getSchema() {
    // Table to log responses obtained by 'RestAPI' class.
    // Note: There is no need to store message text because it could be
    // obtained  by the HTTP Response Code.
    // HttpCode->getMessage($code);
    return [
      'description' => "Stores REST API responses.",
      'fields' => [
        'id' => [
          'description' => 'The unique response identifier.',
          'type' => 'serial',
          'size' => 'normal',
          'unsigned' => TRUE,
          'not null' => TRUE,
        ],
        // Request Id is an md5 hash of all request's data to be able to find
        // exactly the same request in log.
        'request_id' => [
          'description' => 'The unique request identifier.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'created' => [
          'description' => 'The moment when getting response was started. Format: UNIX timestamp.',
          'type' => 'int',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'code' => [
          'description' => "HTTP Response Code.",
          'type' => 'int',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'duration' => [
          'description' => 'Request processing time, msec.',
          'type' => 'int',
          'size' => 'small',
          'not null' => TRUE,
        ],
        'body' => [
          'description' => 'Body of the API response. Format: JSON-string.',
          // varchar/normal: mysql: 64KB, postgresql: 1GB.
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'content_type' => [
          'description' => 'Response content type.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
      ],
      'primary key' => [
        'id',
      ],
      'unique keys' => [
        // It's possible that the same request will bring different API response.
        // For example response is time-based.
        'request_id_created' => ['request_id', 'created'],
      ],
      'foreign keys' => [
        'request_id' => [
          'table' => 'tpps_rest_api_request',
          'columns' => ['request_id' => 'request_id'],
        ],
      ],
    ];
  }

}
