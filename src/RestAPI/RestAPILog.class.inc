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
   *   To check if response exists:
   *     RestAPILog::isResponseExists($request_id).
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
   * Checks if the response exists in DB.
   *
   * @param string $request_id
   *   Unique request identificator.
   * @param int $started
   *   Optional. Unix timestamp of the moment when response was requested.
   *   When NULL last response will be used.
   *
   * @return bool
   *   Returns TRUE if record for given $request_id found in DB.
   *   Returns FALSE otherwise.
   */
  public static function isResponseExists($request_id, $started = NULL) {
    $query = db_select(self::RESPONSE_TABLE, 't')
      ->fields('t', ['request_id'])
      ->condition('request_id', $request_id)
      ->condition('code', HttpCode::OK)
      ->range(0, 1)
      ->orderBy('started', 'DESC');
    if (!is_null($started)) {
      $query->condition('started', $started);
    }
    return (bool) $query->execute()->fetchField();
  }

  /**
   * Loads Rest API response from database.
   *
   * Usage example:
   * extract(RestAPILog::loadResponse($request_id));
   *
   * @param string $request_id
   *   Unique request identificator.
   * @param int $started
   *   Optional. Unix timestamp of the moment when response was requested.
   *   When NULL last response will be used.
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
      ->condition('code', HttpCode::OK)
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

}
