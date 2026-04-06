<?php

/**
 * @file
 * Contains 'RestAPILog' class.
 */

/**
 * Manages log for 'RestAPI' class requests/responses.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestAPILog {

  // Name of the DB table which stores requests.
  const REQUEST_TABLE = 'tpps_rest_api_request';

  // Name of the DB table which stores responses.
  const RESPONSE_TABLE = 'tpps_rest_api_response';

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
   *     RestAPILog::isResponseExists($request_id).
   *   To check if valid response exists:
   *     RestAPILog::isValidResponseExists($request_id).
   *
   * @return bool
   *   Returns TRUE in case of success and FALSE otherwise.
   *
   * @thrown Exception
   *   When $context['request_id'] is empty.
   */
  public static function saveResponse(array $context = []) {
    if (empty($context['request_id'])) {
      throw new Exception("Empty 'request_id'.");
    }
    // Get request's data.
    $request = $context['request'];
    unset($context['request']);
    // Store Response data.
    $context['started'] = time();
    $context['duration'] = intval(round($context['duration']));
    $context['body'] = json_encode($context['body'], TRUE);
    // Note: No need to check if the same response exists because it could
    // have time-based items (like request time) and this check will never
    // give any result. Just store every response.
    db_insert(self::RESPONSE_TABLE)->fields($context)->execute();
    return TRUE;
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
  public static function saveRequest(array $context = []) {
    $key = 'request_id';
    if (empty($context['request_id'])) {
      throw new Exception("Empty 'request_id'.");
    }
    if (self::isRequestExists($context['request_id'])) {
      return FALSE;
    }
    else {
      $context['created'] = $context['created'] ?? time();
      // Convert arrays into JSON format.
      foreach (['payload', 'header'] as $name) {
        $context[$name] = json_encode($context[$name], TRUE);
      }
      db_insert(self::REQUEST_TABLE)->fields($context)->execute();
      return TRUE;
    }
  }

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
  public static function isRequestExists($request_id) {
    return (bool) db_select(self::REQUEST_TABLE, 't')
      ->fields('t', ['request_id'])
      ->condition('request_id', $request_id)
      ->range(0, 1)
      ->orderBy('created', 'DESC')
      ->execute()
      ->fetchField();
  }

  /**
   * Checks if any response exists in DB.
   *
   * WARNING: Invalid requests also logged and will be counted..
   *
   * @param string $request_id
   *   Unique request identificator.
   * @param int $started
   *   Optional. Unix timestamp of the moment when response was requested.
   *   When empty then latest response will be loaded.
   *
   * @return bool
   *   Returns TRUE if record for given $request_id found in DB.
   *   Returns FALSE otherwise.
   */
  public static function isResponseExists($request_id, $started = NULL) {
    $query = db_select(self::RESPONSE_TABLE, 't')
      ->fields('t', ['request_id'])
      ->condition('request_id', $request_id)
      ->range(0, 1)
      ->orderBy('started', 'DESC');
    if (!is_null($started)) {
      $query->condition('started', $started);
    }
    return (bool) $query->execute()->fetchField();
  }

  /**
   * Checks if valid response exists in DB.
   *
   * Valid request:
   * 1. Code 2xx
   * 2. Non-empty body of the response.
   *
   * Use method isResponseExists() to get any response.
   *
   * @param string $request_id
   *   Unique request identificator.
   *
   * @return bool
   *   Returns TRUE if record for given $request_id found in DB.
   *   Returns FALSE otherwise.
   */
  public static function isValidResponseExists($request_id) {
    return (bool) db_select(self::RESPONSE_TABLE, 't')
      ->fields('t', ['request_id'])
      ->condition('request_id', $request_id)
      ->condition('code', 200, '>=')
      ->condition('code', 299, '<=')
      ->condition('body', '', '!=')
      ->range(0, 1)
      ->orderBy('started', 'DESC')
      ->execute()
      ->fetchField();
  }

  /**
   * Loads Rest API response from database.
   *
   * WARNING: Status code won't be checked and invalid responses will be loaded.
   *
   * Usage example:
   * extract(RestAPILog::loadResponse($request_id));
   *
   * @param string $request_id
   *   Unique request identificator.
   * @param int $started
   *   Optional. Unix timestamp of the moment when response was requested.
   *   When empty then latest response will be loaded.
   *
   * @return array
   *   Returns associative array with keys:
   *     'request_id',
   *     'started',
   *     'code',
   *     'duration',
   *     'info',
   *     'response',
   */
  public static function loadResponse($request_id, $started = NULL) {
    $query = db_select(self::RESPONSE_TABLE, 't')
      ->fields('t', [])
      ->condition('request_id', $request_id)
      ->range(0, 1)
      ->orderBy('started', 'DESC');
    if (!is_null($started)) {
      $query->condition('started', $started);
    }
    $result = $query->execute()->fetchAssoc();

    // Curl info doesn't store in DB so we just create empty variable.
    $result['info'] = [];
    $result['response'] = json_decode($result['body'], TRUE);
    unset($result['body']);
    $result['message'] = HttpCode::getMessage($result['code']);

    return $result;
  }

  /**
   * Loads only valid Rest API response from database.
   *
   * Valis response:
   * 1. code 2xx
   * 2. Not empty body.
   * 3. Latest response (no 'started' date).
   *
   * To load invalid responses use loadResponse() method.
   *
   * Usage example:
   * extract(RestAPILog::loadValidResponse($request_id));
   *
   * @param string $request_id
   *   Unique request identificator.
   *
   * @return array
   *   Returns associative array with keys:
   *     'request_id',
   *     'started',
   *     'code',
   *     'duration',
   *     'info',
   *     'response',
   */
  public static function loadValidResponse($request_id) {
    $result = db_select(self::RESPONSE_TABLE, 't')
      ->fields('t', [])
      ->condition('request_id', $request_id)
      ->condition('code', 200, '>=')
      ->condition('code', 299, '<=')
      ->condition('body', '', '!=')
      ->range(0, 1)
      ->orderBy('started', 'DESC')
      ->execute()
      ->fetchAssoc();

    // Curl info doesn't store in DB so we just create empty variable.
    $result['info'] = [];
    $result['response'] = json_decode($result['body'], TRUE);
    unset($result['body']);
    $result['message'] = HttpCode::getMessage($result['code']);

    return $result;
  }


  /**
   * Loads Rest API response from database.
   *
   * WARNING: Status code won't be checked and invalid responses will be loaded.
   *
   * Usage example:
   * extract(RestAPILog::loadResponse($request_id));
   *
   * @param string $request_id
   *   Unique request identificator.
   * @param int $started
   *   Optional. Unix timestamp of the moment when response was requested.
   *   When empty then latest response will be loaded.
   *
   * @return array
   *   Returns associative array with keys:
   *     'request_id',
   *     'started',
   *     'code',
   *     'duration',
   *     'info',
   *     'response',
   */
  public static function loadAllResponses($request_id) {
    $result = db_select(self::RESPONSE_TABLE, 't')
      ->fields('t', [])
      ->condition('request_id', $request_id)
      // ->orderBy('started', 'DESC')
      ->execute()
      ->fetchAllAssoc('started', PDO::FETCH_ASSOC);

    foreach ($result as &$item) {
      // Curl info doesn't store in DB so we just create empty variable.
      $item['info'] = [];
      // @TODO Use original 'body' instead of new 'response'.
      $item['response'] = json_decode($item['body'], TRUE);
      unset($item['body']);
      $item['message'] = HttpCode::getMessage($item['code']);
    }

    return $result;
  }

  public static function getSchema($type) {
    // Request.
    // Table to log requests made by 'RestAPI' class.
    if ($type == 'request') {
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Response.
    if ($type == 'response') {
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
          'started' => [
            'description' => 'The moment request was started. Format: UNIX timestamp.',
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
        ],
        'primary key' => [
          'id',
        ],
        'unique keys' => [
          // It's possible that the same request will bring different API response.
          // For example response is time-based.
          'request_id_started' => ['request_id', 'started'],
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

}
