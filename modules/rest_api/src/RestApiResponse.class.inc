<?php

/**
 * @file
 * Contains 'RestApiResponse' class.
 *
 * Note: Request and Response uses the same name 'created' but values could be
 * totally different.
 *
 * Request has 'payload'.
 * Response has 'body'.
 */

/**
 * Manages Rest API Responses stored in DB.
 *
 * @package rest_api
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestApiResponse {

  // Name of the DB table which stores responses.
  const TABLE = 'rest_api_response';

  /**
   * Saves RestApi Response.
   *
   * No need to check if the same response exists because it's body could have
   * some time-based items (like request time or cache protection).
   *
   * @param array $context
   *   Request's context.
   *   Keys: see RestApi->doRequest().
   *   To check if any response exists:
   *     RestApiResponse::isExists($hash).
   *   To check if valid response exists:
   *     RestApiResponse::isValidExists($request_id).
   *
   * @return mixed
   *   Returns 'response_id' if response was inserted (last insert id).
   *   Returns NULL if response was updated (not inserted).
   *   Returns FALSE if current request should be cached.
   *
   * @thrown Exception
   *   When $context['request_id'] is empty.
   */
  public static function save(array $context = []) {
    if (!$context['cache'] || !variable_get('rest_api_use_cache', TRUE)) {
      // Current request shouldn't be cached.
      return FALSE;
    }
    if (empty($context['request_id'])) {
      throw new Exception("Empty 'request_id'.");
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Prepare data to be stored in db table.
    if (!is_string($body = $context['body'])) {
      $body = json_encode($context['body'], TRUE);
    }
    $data = [
      'request_id' => $context['request_id'],
      'code' => $context['code'],
      'content_type' => $context['content_type'],
      'created' => time(),
      'duration' => intval(round($context['duration'])),
      'body' => trim($body),
      // Response Hash.
      'hash' => static::generateHash($context),
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if (RestApiResponse::isValidExists($data['request_id'])) {
      // Record exists - let's load response.
      $response_data = RestApiResponse::loadValid($data['request_id']);

      // Do not store the same responses multiple times.
      // @TODO Minor. Check also 'code'.
      if ($response_data['hash'] == $data['hash']) {
        // The same response.
        if ($data['request_id'] != $response_data['request_id']) {
          // Check value with existing request_id and update record in
          // db table RestApiResponse::TABLE if they are different.
          // This could happen when requests was removed but responses are left.
          db_update(static::TABLE)
            ->fields(['request_id' => $data['request_id']])
            ->condition('response_id', $response_data['response_id'])
            ->execute();
          return NULL;
        }
      }
      else {
        // New response.
        return db_insert(static::TABLE)->fields($data)->execute();
      }
    }
    else {
      // Load latest invalid response.
      $response_data = RestApiResponse::load($context['request_id']);
      if ($response_data['code'] == $context['code']) {
        // If we've got the same invalid response no need to update db record.
        return NULL;
      }
      // Response wasn't found by 'request_id'. Let's search by 'response_hash'.
      $response_id = static::getResponseId($data['hash']);
      if (empty($response_id)) {
        // No record - create new one.
        return db_insert(static::TABLE)->fields($data)->execute();
      }
      else {
        // Record exist but 'request_id' differs.
        // Check value with existing request_id and update record in
        // db table RestApiResponse::TABLE if they are different.
        // This could happen when requests was removed but responses are left.
        db_update(static::TABLE)
          ->fields(['request_id' => $data['request_id']])
          ->condition('response_id', $response_id)
          ->execute();
        return NULL;
      }
    }
  }

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
   * Gets response id by hash.
   *
   * @param string $hash
   *   Request's data md5 hash.
   *
   * @return mixed
   *   Returns integer 'response_id' if record for given $hash found.
   *   Returns FALSE otherwise.
   */
  public static function getResponseId($hash) {
    return db_select(static::TABLE, 't')
      ->fields('t', ['response_id'])
      ->condition('hash', $hash)
      ->range(0, 1)
      ->orderBy('created', 'DESC')
      ->execute()
      ->fetchField();
  }

  /**
   * Generates Response's data md5 hash.
   *
   * @param array $context
   *   Request's context.
   *   Keys: see RestApi->doRequest().
   *   Used 'body' element.
   *
   * @return string
   *   Returns hash's value.
   */
  public static function generateHash(array $context) {
    // Allow other modules to change $context before hash will be generated.
    // Used to remove time-based or dynamic data in response body to avoid
    // the same responses duplication.
    //
    // rest_api_response_hash_generation_alter().
    drupal_alter('rest_api_response_hash_generation', $context);
    return md5(json_encode(['body' => ($context['body'] ?? '')]));
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
    // Note: static::loadValid() uses static caching so calling it later
    // is NOT expensive.
    return (bool) static::loadValid($request_id);
  }

  /**
   * Loads Rest API response from database.
   *
   * Uses static caching.
   * WARNING: Status code won't be checked and invalid responses will be loaded.
   *
   * Usage example:
   * extract(RestApiResponse::load($request_id));
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
   *     'body',
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
        $result['body'] = static::decodeBody($result);
        $result['message'] = HttpCode::getMessage($result['code']);
      }
      $static_cache[$key] = $result;
    }

    if (function_exists('module_invoke_all')) {
      // rest_api_request_completed().
      $context = [
        'request_id' => $request_id,
        'response_id' => $static_cache[$key]['response_id'],
      ];
      module_invoke_all('rest_api_response_loaded', $context);
    }
    return $static_cache[$key];
  }

  /**
   * Decodes response body.
   *
   * @param array $response
   *   Response metadata from db table RestApiResponse::TABLE.
   *
   * @return mixed
   *   Returns response body.
   */
  public static function decodeBody(array $response) {
    if (strpos($response['content_type'], 'application/json') !== FALSE) {
      // JSON.
      $response['body'] = json_decode($response['body'], TRUE);
    }
    elseif (
      strpos($response['content_type'], 'application/xml') !== FALSE
      || strpos($response['content_type'], 'text/xml') !== FALSE
    ) {
      // XML.
      // Stored as string. No need to decode.
    }
    else {
      dpm('Unknown response content type.');
      $response['body'] = json_decode($response['body'], TRUE);
    }
    return $response['body'];
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
   * To load invalid responses use RestApiResponse::load() method.
   *
   * Usage example:
   * extract(RestApiResponse::loadValid($request_id));
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
   *     'body',
   */
  public static function loadValid($request_id, $reset = FALSE) {
    $static_cache = &drupal_static(__METHOD__);

    $key = $request_id;
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
        $result['body'] = static::decodeBody($result);
        $result['message'] = HttpCode::getMessage($result['code']);
      }
      if (function_exists('module_invoke_all')) {
        $context = [
          'request_id' => $request_id,
          'response_id' => $result['response_id'],
        ];
        // rest_api_response_loaded().
        module_invoke_all('rest_api_response_loaded', $context);
      }
      $static_cache[$key] = $result;
    }
    return $static_cache[$key];
  }

  /**
   * Loads Rest API response from database.
   *
   * WARNING: Status code won't be checked and invalid responses will be loaded.
   *
   * Usage example:
   * extract(RestApiResponse::load($request_id));
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
   *     'body',
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
      $item['body'] = static::decodeBody($item);
      $item['message'] = HttpCode::getMessage($item['code']);
    }

    // @todo ?Should I add the hook to have watchdog record?
    return $result;
  }

  /**
   * Gets db table schema.
   *
   * Table to log responses received by 'RestApi' class.
   *
   * @return array
   *   Returns array for hook_schema().
   */
  public static function getSchema() {
    // Table to log responses obtained by 'RestApi' class.
    // Note: There is no need to store message text because it could be
    // obtained  by the HTTP Response Code.
    // HttpCode->getMessage($code);
    return [
      'description' => "Stores REST API responses.",
      'fields' => [
        'response_id' => [
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
          'type' => 'int',
          'size' => 'normal',
          'unsigned' => TRUE,
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
          'type' => 'text',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'content_type' => [
          'description' => 'Response content type.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
          'length' => 255,
        ],
        'hash' => [
          'description' => "md5 hash of the response body.",
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
          'length' => 32,
        ],
        // @todo Minor. Add 'request_hash' to be able to find response by
        // 'request_hash' when record was removed from 'rest_api_request'
        // table but response was left. Otherwise orphan records in
        // 'rest_api_response' table will be left. But it's a rare case.
        // Currently this solved by using FK which removes related responses
        // when request was removed.
      ],
      'primary key' => ['response_id'],
      'unique keys' => ['request_id_created' => ['request_id', 'created']],
      'foreign keys' => [
        'request_id' => [
          'table' => 'rest_api_request',
          'columns' => ['request_id' => 'request_id'],
        ],
      ],
    ];
  }

  /**
   * Restores XML from JSON decoded string.
   *
   * XML response is stored as json string in db table's field
   * 'public.rest_api_response.body'.
   * To use it as XML it must be 'restored' first.
   *
   * @param string $json
   *   String in JSON format with escaped tags.
   *
   * @return string
   *   Returns XML-string.
   */
  public static function restoreXml($json) {
    // Remove leading/trailing quotes and whitespace.
    $data = trim($json, "\n\r\t \"'");
    // Decode JSON unicode escapes.
    $data = preg_replace_callback('/\\\\u([0-9a-fA-F]{4})/',
      function ($matches) {
        return mb_convert_encoding(pack('H*', $matches[1]), 'UTF-8', 'UTF-16BE');
      },
      $data
    );
    // Replace other JSON escapes.
    $data = str_replace(
      ['\\/', '\\"', '\\n', '\\r', '\\t'], ['/', '"', "\n", "\r", "\t"],
      $data
    );
    return $data;
  }

}
