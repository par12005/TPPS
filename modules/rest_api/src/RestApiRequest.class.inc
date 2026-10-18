<?php

/**
 * @file
 * Contains 'RestApiRequest' class.
 *
 * Request has 'payload'.
 * Response has 'body'.
 */

/**
 * Manages Rest API Requests stored in database.
 *
 * To avoid request caching make URL unique (e.g. add 'z=' . time()).
 *
 * To get db table fields:
 *   $fields = array_keys(RestApiRequest::getSchema()['fields']);
 *
 * @package rest_api
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestApiRequest {

  /**
   * Name of the database table that stores requests.
   */
  const TABLE = 'rest_api_request';

  /**
   * Sensitive HTTP header names whose values must be redacted in logs and DB.
   */
  const SENSITIVE_HEADERS = [
    'authorization',
    'x-api-key',
    'api-key',
    'proxy-authorization',
    'cookie',
    'set-cookie',
    'x-auth-token',
    'x-csrf-token',
    'client-secret',
  ];

  /**
   * Sensitive URL query parameter names whose values must be redacted in logs.
   */
  const SENSITIVE_QUERY_PARAMS = [
    'key',
    'api_key',
    'apikey',
    'access_token',
    'token',
    'auth_token',
    'secret',
    'client_secret',
    'password',
  ];

  /**
   * Masks secrets in URLs and redacts sensitive HTTP headers in context.
   *
   * @param array $context
   *   Request context array.
   *
   * @return array
   *   Sanitized request context array safe for logging and DB storage.
   */
  public static function sanitizeContext(array $context) {
    if (!empty($context['url']) && is_string($context['url'])) {
      // Mask Telegram Bot API tokens in URL path.
      $context['url'] = preg_replace(
        '~(/bot\d+:)[A-Za-z0-9_-]+~',
        '$1***REDACTED***',
        $context['url']
      );

      // Mask common secret query parameters in URL.
      $param_pattern = implode('|', static::SENSITIVE_QUERY_PARAMS);
      $context['url'] = preg_replace(
        '~([?&](?:' . $param_pattern . ')=)[^&#]+~i',
        '$1***REDACTED***',
        $context['url']
      );
    }

    if (!empty($context['header'])) {
      $headers = $context['header'];
      $was_json = FALSE;
      if (is_string($headers)) {
        $decoded = json_decode($headers, TRUE);
        if (json_last_error() === JSON_ERROR_NONE && is_array($decoded)) {
          $headers = $decoded;
          $was_json = TRUE;
        }
      }

      if (is_array($headers)) {
        foreach ($headers as $key => $value) {
          if (is_string($key)) {
            $lower_key = strtolower(trim($key));
            if (in_array($lower_key, static::SENSITIVE_HEADERS, TRUE)) {
              $headers[$key] = '***REDACTED***';
            }
          }
          elseif (is_string($value) && strpos($value, ':') !== FALSE) {
            list($h_name,) = explode(':', $value, 2);
            $lower_name = strtolower(trim($h_name));
            if (in_array($lower_name, static::SENSITIVE_HEADERS, TRUE)) {
              $headers[$key] = trim($h_name) . ': ***REDACTED***';
            }
          }
        }
        $json_flags = JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES;
        $context['header'] = $was_json
          ? json_encode($headers, $json_flags)
          : $headers;
      }
    }

    return $context;
  }

  /**
   * Checks if record already exists in DB.
   *
   * @param string $hash
   *   Request's data md5 hash.
   *
   * @return bool
   *   Returns TRUE if record for given $hash is found in DB, FALSE otherwise.
   */
  public static function isExists($hash) {
    return (bool) static::getRequestId($hash);
  }

  /**
   * Gets request ID by hash with static caching.
   *
   * @param string $hash
   *   Request's data md5 hash.
   * @param bool $reset
   *   TRUE to reset static cache. Default is FALSE.
   *
   * @return int|string|bool
   *   Returns request_id if record for given $hash is found, FALSE otherwise.
   */
  public static function getRequestId($hash, $reset = FALSE) {
    if (empty($hash)) {
      return FALSE;
    }
    $static_cache = &drupal_static(__METHOD__, []);
    if ($reset || !array_key_exists($hash, $static_cache)) {
      $static_cache[$hash] = db_select(static::TABLE, 't')
        ->fields('t', ['request_id'])
        ->condition('hash', $hash)
        ->range(0, 1)
        ->execute()
        ->fetchField();
    }
    return $static_cache[$hash];
  }

  /**
   * Saves RestApi Request.
   *
   * Called from class 'RestApi'.
   * Class 'RestApi' decides if response from DB will be used.
   * This method saves unique request metadata and updates static cache.
   *
   * @param array $context
   *   Request's context.
   *   Keys: see RestApi->doRequest().
   *   'tag' is optional.
   *
   * @return int|string
   *   Returns last insert id when a new record is created, or existing
   *   request_id if a record with the same hash already exists.
   *
   * @throws Exception
   *   When $context['hash'] is empty.
   */
  public static function save(array $context = []) {
    if (empty($context['hash'])) {
      throw new Exception("Empty 'hash'.");
    }
    $context = static::sanitizeContext($context);
    if ($request_id = static::getRequestId($context['hash'])) {
      return $request_id;
    }

    $data = [];
    $json_flags = JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES;
    $field_list = array_keys(static::getSchema()['fields']);
    foreach ($field_list as $field) {
      if ($field === 'request_id') {
        continue;
      }
      elseif ($field === 'created') {
        $data[$field] = $context[$field] ?? time();
      }
      elseif (
        in_array($field, ['payload', 'header'], TRUE)
        && !is_string($context[$field] ?? NULL)
      ) {
        $data[$field] = json_encode($context[$field] ?? [], $json_flags);
      }
      else {
        $data[$field] = $context[$field] ?? NULL;
      }
    }

    $request_id = db_insert(static::TABLE)->fields($data)->execute();
    $static_cache = &drupal_static('RestApiRequest::getRequestId', []);
    $static_cache[$context['hash']] = $request_id;
    return $request_id;
  }

  /**
   * Loads Rest API request from database.
   *
   * @param int|string $request_id
   *   Unique request identifier.
   *
   * @return array|bool
   *   Associative array with request fields, or FALSE if not found in DB.
   */
  public static function load($request_id) {
    if (empty($request_id)) {
      return FALSE;
    }
    $data = db_select(static::TABLE, 't')
      ->fields('t', [])
      ->condition('request_id', $request_id)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();

    if (!empty($data['payload'])) {
      $decoded = json_decode($data['payload'] ?? '', TRUE);
      if (json_last_error() === JSON_ERROR_NONE) {
        $data['payload'] = $decoded;
      }
    }
    if (!empty($data['header'])) {
      $decoded = json_decode($data['header'] ?? '', TRUE);
      if (json_last_error() === JSON_ERROR_NONE) {
        $data['header'] = $decoded;
      }
    }

    return $data;
  }

  /**
   * Gets database table schema.
   *
   * Table to log requests made by 'RestApi' class.
   *
   * @return array
   *   Schema definition array for hook_schema().
   */
  public static function getSchema() {
    return [
      'description' => 'Stores REST API requests.',
      'fields' => [
        'request_id' => [
          'description' => 'The unique request identifier.',
          'type' => 'serial',
          'size' => 'normal',
          'unsigned' => TRUE,
          'not null' => TRUE,
        ],
        'created' => [
          'description' => 'First time request was created (UNIX timestamp).',
          'type' => 'int',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'method' => [
          'description' => 'HTTP method: POST, GET, PUT, DELETE.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
          'length' => 10,
        ],
        'url' => [
          'description' => 'API Endpoint.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
          'length' => 2048,
        ],
        'payload' => [
          'description' => "Request's payload. Format: JSON-string.",
          'type' => 'text',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'header' => [
          'description' => 'HTTP Headers. Format: JSON-string.',
          'type' => 'text',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'hash' => [
          'description' => "MD5 hash of the request's data.",
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
          'length' => 32,
        ],
        'tag' => [
          'description' => 'Request description for humans.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
          'length' => 255,
        ],
      ],
      'unique keys' => ['hash' => ['hash']],
      'indexes' => [
        'created' => ['created'],
      ],
      'primary key' => ['request_id'],
    ];
  }

  /**
   * Gets request IDs with pagination.
   *
   * @param int $limit
   *   Maximum number of request IDs to return. Default 1000.
   * @param int $offset
   *   Number of records to skip. Default 0.
   *
   * @return array
   *   List of request IDs ordered from newest to oldest.
   */
  public static function getAll($limit = 1000, $offset = 0) {
    $query = db_select(static::TABLE, 't')
      ->fields('t', ['request_id'])
      ->orderBy('request_id', 'DESC');
    if ($limit > 0) {
      $query->range((int) $offset, (int) $limit);
    }
    return $query->execute()->fetchCol('request_id');
  }

}
