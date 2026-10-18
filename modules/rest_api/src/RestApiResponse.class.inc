<?php

/**
 * @file
 * Contains 'RestApiResponse' class.
 *
 * Note: Request and Response use the same column name 'created' but values
 * represent when each record was captured.
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

  /**
   * Name of the database table that stores responses.
   */
  const TABLE = 'rest_api_response';

  /**
   * Saves RestApi Response.
   *
   * Checks only the latest response metadata (response_id, hash, code) for the
   * current $data['request_id'] without loading or decoding the body blob: if
   * its hash and HTTP status code match the new response, updates its 'created'
   * and 'duration' fields and resets the response static caches; otherwise
   * inserts a new row.
   *
   * @param array $context
   *   Request's context.
   *   Keys: see RestApi->doRequest().
   *
   * @return int|string|null|bool
   *   Returns 'response_id' if response was inserted (last insert id).
   *   Returns NULL if existing response was updated (not inserted).
   *   Returns FALSE if logging is disabled.
   *
   * @throws Exception
   *   When $context['request_id'] is empty.
   */
  public static function save(array $context = []) {
    if (empty($context['log']) || !variable_get('rest_api_use_log', TRUE)) {
      // Request shouldn't be logged.
      return FALSE;
    }
    if (empty($context['request_id'])) {
      throw new Exception("Empty 'request_id'.");
    }

    // Prepare data to be stored in db table.
    $body = $context['body'] ?? '';
    if (!is_string($body)) {
      $json_flags = JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES;
      $body = json_encode($body, $json_flags);
    }
    $data = [
      'request_id' => $context['request_id'],
      'code' => (int) $context['code'],
      'content_type' => $context['content_type'] ?? '',
      'created' => time(),
      'duration' => max(0, intval(round($context['duration'] ?? 0))),
      'body' => trim((string) $body),
      'hash' => static::generateHash($context),
    ];

    // Query only lightweight metadata columns for the latest response on this
    // request_id to avoid fetching/decoding large body blobs or firing hooks.
    $latest_response = db_select(static::TABLE, 't')
      ->fields('t', ['response_id', 'hash', 'code'])
      ->condition('request_id', $data['request_id'])
      ->orderBy('created', 'DESC')
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();

    if (
      !empty($latest_response)
      && ($latest_response['hash'] ?? NULL) === $data['hash']
      && (int) ($latest_response['code'] ?? 0) === $data['code']
    ) {
      db_update(static::TABLE)
        ->fields([
          'created' => $data['created'],
          'duration' => $data['duration'],
        ])
        ->condition('response_id', $latest_response['response_id'])
        ->execute();
      drupal_static_reset('RestApiResponse::load');
      drupal_static_reset('RestApiResponse::loadValid');
      return NULL;
    }

    $response_id = db_insert(static::TABLE)->fields($data)->execute();
    drupal_static_reset('RestApiResponse::load');
    drupal_static_reset('RestApiResponse::loadValid');
    return $response_id;
  }

  /**
   * Checks if record already exists in DB.
   *
   * @param string $hash
   *   Response's data md5 hash.
   *
   * @return bool
   *   Returns TRUE if record for given $hash is found in DB, FALSE otherwise.
   */
  public static function isExists($hash) {
    return (bool) static::getResponseId($hash);
  }

  /**
   * Gets response ID by hash.
   *
   * @param string $hash
   *   Response's data md5 hash.
   *
   * @return int|string|bool
   *   Returns 'response_id' if record for given $hash is found, FALSE
   *   otherwise.
   */
  public static function getResponseId($hash) {
    if (empty($hash)) {
      return FALSE;
    }
    return db_select(static::TABLE, 't')
      ->fields('t', ['response_id'])
      ->condition('hash', $hash)
      ->range(0, 1)
      ->execute()
      ->fetchField();
  }

  /**
   * Generates Response's data md5 hash.
   *
   * @param array $context
   *   Request's context.
   *   Keys: see RestApi->doRequest().
   *   Uses 'body' element.
   *
   * @return string
   *   Returns MD5 hash string.
   */
  public static function generateHash(array $context) {
    // Allow other modules to change $context before hash is generated.
    // Used to strip time-based or dynamic data in response body to avoid
    // duplicating identical responses.
    // Invokes hook_rest_api_response_hash_generation_alter().
    drupal_alter('rest_api_response_hash_generation', $context);
    return md5(json_encode(['body' => ($context['body'] ?? '')]));
  }

  /**
   * Checks if valid response exists in DB.
   *
   * Valid response:
   * 1. Code 200..299.
   * 2. Non-empty body (or HTTP 204 No Content).
   * 3. Within cache TTL (rest_api_cache_ttl) if TTL > 0.
   *
   * @param int|string $request_id
   *   Unique request identifier.
   *
   * @return bool
   *   Returns TRUE if valid record for $request_id is found, FALSE otherwise.
   */
  public static function isValidExists($request_id) {
    if (empty($request_id)) {
      return FALSE;
    }
    return (bool) static::loadValid($request_id);
  }

  /**
   * Loads Rest API response from database.
   *
   * Uses static caching.
   * WARNING: Status code won't be checked and invalid responses will be loaded.
   *
   * @param int|string $request_id
   *   Unique request identifier.
   * @param int|null $created
   *   Optional Unix timestamp when response was captured. When NULL, the
   *   latest response is loaded.
   * @param bool $reset
   *   TRUE to reset static cache. Default is FALSE.
   *
   * @return array|bool
   *   Associative array of response data, or FALSE if not found.
   */
  public static function load($request_id, $created = NULL, $reset = FALSE) {
    if (empty($request_id)) {
      return FALSE;
    }
    $static_cache = &drupal_static(__METHOD__);
    $key = md5(json_encode([$request_id, $created]));
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
        // cURL info isn't stored in DB so initialize an empty array.
        $result['info'] = [];
        $result['body'] = static::decodeBody($result);
        $result['message'] = HttpCode::getMessage($result['code']);
      }
      $static_cache[$key] = $result;
    }

    if (
      function_exists('module_invoke_all')
      && !empty($static_cache[$key]['response_id'])
    ) {
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
   *   Decoded response body (array for JSON, string for XML/text).
   */
  public static function decodeBody(array $response) {
    $content_type = $response['content_type'] ?? '';
    if (stripos($content_type, 'application/json') !== FALSE) {
      // JSON.
      $decoded = json_decode($response['body'] ?? '', TRUE);
      if (json_last_error() === JSON_ERROR_NONE) {
        $response['body'] = $decoded;
      }
    }
    elseif (
      stripos($content_type, 'application/xml') !== FALSE
      || stripos($content_type, 'text/xml') !== FALSE
    ) {
      // XML is stored as string; no need to decode.
    }
    else {
      $decoded = json_decode($response['body'] ?? '', TRUE);
      if (json_last_error() === JSON_ERROR_NONE) {
        $response['body'] = $decoded;
      }
    }
    return $response['body'];
  }

  /**
   * Loads only valid Rest API response from database.
   *
   * Uses static caching.
   * Valid response conditions:
   * 1. Code 200..299.
   * 2. Non-empty body (or code 204 No Content).
   * 3. Not expired according to variable 'rest_api_cache_ttl' (when > 0).
   *
   * @param int|string $request_id
   *   Unique request identifier.
   * @param bool $reset
   *   TRUE to reset static cache. Default is FALSE.
   *
   * @return array|bool
   *   Associative array of valid response data, or FALSE if not found.
   */
  public static function loadValid($request_id, $reset = FALSE) {
    if (empty($request_id)) {
      return FALSE;
    }
    $static_cache = &drupal_static(__METHOD__);

    $key = $request_id;
    if ($reset || !isset($static_cache[$key])) {
      $query = db_select(static::TABLE, 't')
        ->fields('t', [])
        ->condition('request_id', $request_id)
        ->condition('code', 200, '>=')
        ->condition('code', 299, '<=');

      $cache_ttl = (int) variable_get('rest_api_cache_ttl', 0);
      if ($cache_ttl > 0) {
        $query->condition('created', time() - $cache_ttl, '>=');
      }

      $result = $query
        ->range(0, 1)
        ->orderBy('created', 'DESC')
        ->execute()
        ->fetchAssoc();

      if (!empty($result)) {
        $raw_body = trim((string) ($result['body'] ?? ''));
        if ((int) $result['code'] !== 204 && $raw_body === '') {
          $result = FALSE;
        }
        else {
          // cURL info isn't stored in DB so initialize an empty array.
          $result['info'] = [];
          $result['body'] = static::decodeBody($result);
          $result['message'] = HttpCode::getMessage($result['code']);

          if (
            function_exists('module_invoke_all')
            && !empty($result['response_id'])
          ) {
            $context = [
              'request_id' => $request_id,
              'response_id' => $result['response_id'],
            ];
            module_invoke_all('rest_api_response_loaded', $context);
          }
        }
      }
      $static_cache[$key] = $result;
    }
    return $static_cache[$key];
  }

  /**
   * Loads all Rest API responses for a request from database.
   *
   * WARNING: Status code won't be checked and invalid responses will be loaded.
   *
   * @param int|string $request_id
   *   Unique request identifier.
   *
   * @return array
   *   Associative array of responses keyed by 'created'.
   */
  public static function loadAll($request_id) {
    $result = db_select(static::TABLE, 't')
      ->fields('t', [])
      ->condition('request_id', $request_id)
      ->orderBy('created', 'DESC')
      ->execute()
      ->fetchAllAssoc('created', PDO::FETCH_ASSOC);

    foreach ($result as &$item) {
      $item['info'] = [];
      $item['body'] = static::decodeBody($item);
      $item['message'] = HttpCode::getMessage($item['code']);
    }

    return $result;
  }

  /**
   * Gets database table schema.
   *
   * Table to log responses received by 'RestApi' class.
   *
   * @return array
   *   Schema definition array for hook_schema().
   */
  public static function getSchema() {
    return [
      'description' => 'Stores REST API responses.',
      'fields' => [
        'response_id' => [
          'description' => 'The unique response identifier.',
          'type' => 'serial',
          'size' => 'normal',
          'unsigned' => TRUE,
          'not null' => TRUE,
        ],
        'request_id' => [
          'description' => 'The unique request identifier.',
          'type' => 'int',
          'size' => 'normal',
          'unsigned' => TRUE,
          'not null' => TRUE,
        ],
        'created' => [
          'description' => 'Timestamp when response was captured (UNIX time).',
          'type' => 'int',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'code' => [
          'description' => 'HTTP Response Code.',
          'type' => 'int',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'duration' => [
          'description' => 'Request processing time, msec.',
          'type' => 'int',
          'size' => 'normal',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'default' => 0,
        ],
        'body' => [
          'description' => 'Body of the API response (LONGTEXT up to 4GB).',
          'type' => 'text',
          'size' => 'big',
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
          'description' => 'MD5 hash of the response body.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
          'length' => 32,
        ],
      ],
      'primary key' => ['response_id'],
      'indexes' => [
        'hash' => ['hash'],
        'request_id_created' => ['request_id', 'created'],
        'request_id_code' => ['request_id', 'code', 'created'],
      ],
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
   * @param string $json
   *   String in JSON format with escaped tags.
   *
   * @return string
   *   Restored XML string.
   */
  public static function restoreXml($json) {
    // Remove leading/trailing quotes and whitespace.
    $data = trim($json, "\n\r\t \"'");
    // Decode JSON unicode escapes.
    $data = preg_replace_callback(
      '/\\\\u([0-9a-fA-F]{4})/',
      function ($matches) {
        $packed = pack('H*', $matches[1]);
        return mb_convert_encoding($packed, 'UTF-8', 'UTF-16BE');
      },
      $data
    );
    // Replace other JSON escapes.
    $data = str_replace(
      ['\\/', '\\"', '\\n', '\\r', '\\t'],
      ['/', '"', "\n", "\r", "\t"],
      $data
    );
    return $data;
  }

}
