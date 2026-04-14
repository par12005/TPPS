<?php

/**
 * @file
 * Contains 'RestAPI' class.
 *
 * WARNING:
 * Do not use any caching in class. Caching must be done outside the class
 * because not all responses and not all methods could be cached.
 *
 *  How to create an instnace of the 'RestAPI' class?
 *    $data['response'] = (new RestAPI($data))->get()->response;
 *    or
 *    $api = new RestAPI($context);
 *    $api->doRequest();
 *    or
 *    $api = tpps_rest_api($context)->doRequest();
 *    or
 *    $api = tpps_rest_api()
 *      ->url($url)
 *      ->header($header)
 *      // ...
 *      ->get();
 *
 * How to add 'tag'?
 *   $response = (new RestAPI([
 *     'url' => $url,
 *     'tag' => tag,
 *   ])->get()->response();
 *   or
 *   $api = new RestAPI($context);
 *   $api->tag = $tag;
 *
 * DB Tables:
 *   - public.tpps_rest_api_request
 *   - public.tpps_rest_api_response.
 *
 * Logging:
 *   See 'RestAPIResponse' and 'RestAPIRequest' classes.
 *   Request is stored when doRequest() was called.
 *   Response is stored when request was received.
 *
 *  @todo Change structure of the $header to array with key-value format
 *  instead of list of the strings.
 */

/**
 * Performs Rest API requiests using curl.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class RestAPI {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Request related properties.

  /**
   * Required. API Endpoint URL.
   *
   * For GET request $query property could be used.
   *
   * @var string
   */
  protected $url;

  /**
   * Optional. Query part of the URL.
   *
   * Used to build API Endpoint URL for GET requests.
   *
   * @var array
   */
  protected $query = [];

  /**
   * Required. HTTP method. Possible values are: 'POST', 'GET', 'PUT', 'DELETE'.
   *
   * Use uppercase.
   *
   * @var string
   */
  protected $method;

  /**
   * HTTP Headers.
   *
   * Required for POST-requests.
   *
   * List of headers (stored as strings).
   * Stored in 'tpps_rest_api_request.header' column.
   *
   * @var array
   */
  protected $header = [];

  /**
   * Optional. Paylod for POST, PUT requests.
   *
   * Also used by GET request to have 'query'.
   *
   * To send JSON use header: 'Content-Type: application/json'.
   * Stored in 'tpps_rest_api_request.payload' column.
   *
   * @var mixed
   *   String or array.
   */
  protected $payload = [];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Extra request data.

  /**
   * Unique hash (md5) of the all the request data.
   *
   * Automatically generated.
   * Having this id allows to find the same requests.
   * Stored in columns:
   * - 'tpps_rest_api_request.request_id',
   * - 'tpps_rest_api_response.request_id'.
   *
   * @var string
   */
  protected $requestId;

  /**
   * Optional. Label for request to make requests searchable by humans.
   *
   * Stored in tpps_rest_api_response.tag column.
   *
   * @var string
   */
  protected $tag = NULL;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Response related properties.

  /**
   * HTPP Code of the response status.
   *
   * See 'HttpCode' class.
   * Stored in 'tpps_rest_api_response.code' column.
   *
   * @var int
   */
  protected $code;

  /**
   * Human readable message of the the HTTP Code of the response.
   *
   * Message doesn't stored in DB but could always be obtained by $code:
   * HttpCode::getMessage($code).
   *
   * @var string
   */
  protected $message;

  /**
   * Unix timestamp of the moment when request was created.
   *
   * Stored in 'tpps_rest_api_response.created' column.
   *
   * @var int
   */
  protected $created;

  /**
   * Request processing time in msec.
   *
   * Time between sending request and getting response.
   * Stored in 'tpps_rest_api_response.duration' column.
   *
   * @var int
   */
  protected $duration;


  /**
   * Remote server's response.
   *
   * Stored in 'tpps_rest_api_response.body' column.
   *
   * @var mixed
   */
  protected $response;

  /**
   * Result of the curl_getinfo().
   *
   * @var array
   */
  protected $curlInfo;

  /**
   * Controls if request must be stored in DB.
   *
   * When TRUE request will be stored.
   * See class 'RestAPIRequest' which manages responses stored in DB.
   * When FALSE then responses won't be saved into DB.
   * Global settings controlled by variable: 'tpps_rest_api_cache_requests'.
   * Each request could overwrite global setting.
   *
   * @var bool
   */
  protected $cacheRequest;

  /**
   * Controls if response must be stored in DB.
   *
   * When TRUE response will be stored.
   *   See class 'RestAPIResponse' which manages responses stored in DB.
   * When FALSE then responses won't be saved into DB.
   * Global settings controlled by variable: 'tpps_rest_api_cache_responses'.
   * Each request could overwrite global setting.
   *
   * @var bool
   */
  protected $cacheResponse;

  /**
   * Allow to use cached response instead of actual API Request.
   *
   * When TRUE latest valid response will be used (if exists).
   * When FALSE then API will be request to get most up-to-date data.
   * Global settings controlled by variable: 'tpps_rest_api_use_cached_responses'.
   * Each request could overwrite global setting.
   *
   * @var bool
   */
  protected $useCachedResponse;

  /**
   * Creates request from $context data.
   *
   * @param array $context
   *   Associative array. Keys are:
   *   - 'url' string
   *      Required. API Endpoint URL.
   *   - 'method' string
   *     Required. HTTP Method.
   *     One of 'GET', 'POST', 'PUT' or 'DELETE'.
   *   - 'payload' array
   *     Optional. Payload for POST and PUT requests.
   *   - 'header' array
   *     Optional. List of HTTP headers (as strings).
   *   - 'cache_request' bool
   *     Optional. Default is TRUE.
   *     When TRUE then request will be stored in database.
   *     Global setting controlled by variable: 'tpps_rest_api_cache_requests'.
   *   - 'cache_response' bool
   *     Optional. Default is TRUE.
   *     When TRUE then response will be stored in database.
   *     Global setting controlled by variable: 'tpps_rest_api_cache_responses'.
   *     When FALSE any response won't be stored in database.
   *   - 'use_cached_response' bool
   *     Optional. Default is FALSE.
   *     When TRUE latest cached and valid response will be used instead of
   *     actual REST API Request.
   *
   * @return object
   *   Returns $this to allow method chaining.
   */
  public function createRequest(array $context) {
    // Required.
    $this->url = $context['url'] ?? $this->url ?? '';
    $this->method = strtoupper($context['method'] ?? $this->method ?? '');
    // Conditional.
    $this->payload = $context['payload'] ?? $this->payload ?? [];
    $this->header = $context['header'] ?? $this->header ?? [];
    $this->query = $context['query'] ?? $this->query ?? [];
    // Optional.
    $this->tag = $context['tag'] ?? $this->tag ?? NULL;

    $this->cacheRequest = $context['cache_request']
      ?? variable_get('tpps_rest_api_cache_requests', TRUE);
    $this->cacheResponse = $context['cache_response']
      ?? variable_get('tpps_rest_api_cache_responses', TRUE);
    $this->useCachedResponse = $context['use_cached_response']
      ?? variable_get('tpps_rest_api_use_cached_responses', FALSE);
    // Generated.
    $this->created = time();

    return $this;
  }

  /**
   * Loads existing request from DB.
   *
   * @param string $request_id
   *   Unique Request Id.
   *
   * @return object
   *   Returns $this to allow method chaining.
   */
  public function loadRequest($request_id) {
    $this->createRequest(
      RestAPIRequest::load($request_id)
    );
    return $this;
  }

  /**
   * Generates unique id for request.
   *
   * All request data are used to be able to find the same requests in log.
   *
   * Creates md5 hash of the all the request data.
   *
   * @return object
   *   Returns $this to allow method chaining.
   */
  public function createRequestId() {
    $new_request_id = md5(json_encode([
      'method' => strtoupper($this->method),
      // WARNING:
      // Be sure to remove time-based data from the request's payload and URL
      // to avoid duplicate records in DB.
      'url' => $this->url,
      'payload' => $this->payload,
      'header' => $this->header,
      // Using this extra data allows to have multiple almost the same requests
      // but they are not the same. Also it solves issue with updating existing
      // records in DB. Currently we do not update but only insert because it's
      // easier and faster.
      'cache_response' => $this->cacheResponse,
      'use_cached_response' => $this->useCachedResponse,
      // No sense to use 'cacheRequest' because if it's FALSE there is no
      // record in DB.
    ]));
    if (!empty($this->requestId) && $this->requestId != $new_request_id) {
      // Case when rules of the RequestId generation was changed and we have
      // outdated values of the request Id in database.
      // And request's data was loaded using request Id from DB (not created
      // from $context). Both DB tables (request and response) must be updated.
      $this->replaceRequestId($new_request_id);
    }
    $this->requestId = $new_request_id;

    return $this;
  }

  /**
   * Setter for $tag property.
   *
   * @param mixed $value
   *
   *   Optional value of the tag.
   *   When empty tag will be generated automatically using request's data.
   *
   * @return object
   *   Returns $this to allow method chaining.
   */
  public function generateTag($value = NULL) {
    if (!is_null($value)) {
      $this->tag = $value;
    }
    elseif (empty($this->tag)) {
      // Respect existing value of the tag.
      // Url without query.
      $url = strtok($this->url, '?');
      // Remove schema from url.
      $protocols = ['http://', 'https://'];
      $url = str_replace($protocols, '', $url);
      $this->tag = implode(' ', [$this->method, $url]);
    }
    return $this;
  }

  /**
   * Checks if required properties was set.
   *
   * @throws Exception
   *   When required properties wasn't set or empty.
   *
   * @todo Convert to private method if it's for internal usage only or
   * return $this to support method chaining.
   */
  public function validateRequest() {
    if (empty($this->url)) {
      throw new Exception("Required property 'url' is empty.");
    }
    if (empty($this->method)) {
      throw new Exception("Required property 'method' is empty.");
    }
  }

  /**
   * Class constructor.
   *
   * @param mixed $context
   *   Request's metadata.
   *   When array then it must have class's property as a key and value
   *     will be set to this class's property.
   *   When string (or integer) then it's treated as a Request Id and metadata
   *     will be loaded from DB if there is a record with this 'Request Id'.
   *
   * @return object
   *   Returns $this to allow method chaining.
   */
  public function __construct($context = NULL) {
    if (!empty($context)) {
      if (is_array($context)) {
        // Request's metadata.
        $this->createRequest($context);
      }
      else {
        // Request's Id.
        $this->loadRequest($request_id);
      }
    }
    else {
      // If $context is empty it's not an error because request's metadata
      // could be set later.
    }
    return $this;
  }

  /**
   * Magic setter.
   *
   * @param string $name
   *   Class property name.
   * @param mixed $value
   *   Value to be set.
   *
   * @return object
   *   Returns $this to allow method chaining.
   *
   * @throws Exception
   *   When requested non-existing property.
   */
  public function __set($name, $value) {
    // Allow to overwrite magic with custom setter.
    // Method naming rule: 'set<PropertyName>'.
    $func = tpps_snake_to_camel_case('set_' . $name);
    if (method_exists($this, $func)) {
      $this->$func($value);
    }
    else {
      if (property_exists($this, $name)) {
        $this->$name = $value;
      }
      else {
        throw new Exception("Inexistent property: $name");
      }
    }
    return $this;
  }

  /**
   * Magic getter.
   *
   * @param string $name
   *   Class property name.
   *
   * @return mixed
   *   Returns property value.
   *
   * @throws Exception
   *   When requested non-existing property.
   */
  public function &__get($name) {
    // Allow to overwrite magic with custom getter.
    $func = tpps_snake_to_camel_case('get_' . $name);
    if (method_exists($this, $func)) {
      return $this->$func();
    }
    else {
      if (property_exists($this, $name)) {
        return $this->$name;
      }
      else {
        throw new Exception("Inexistent property: $name");
      }
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Methods.

  /**
   * Performs POST-request.
   *
   * Could be used to update/create entity.
   *
   * @return object
   *   Returns current object to allow mehtod chaining.
   */
  public function post() {
    $this->method = 'POST';
    if (empty($this->header)) {
      $this->header = ['Content-Type: application/json'];
    }
    // @TODO Minor. Implement more complex check. Regexp?
    if (in_array('Content-Type: application/json', $this->header)) {
      $this->payload = json_encode($this->payload);
    }
    $this->doRequest();

    return $this;
  }

  /**
   * Performs GET-request.
   *
   * Could be used to get lists or load entity.
   *
   * @return object
   *   Returns current object to allow mehtod chaining.
   */
  public function get() {
    $this->method = 'GET';
    $this->doRequest();

    return $this;
  }

  /**
   * Performs PUT-request.
   *
   * @return object
   *   Returns current object to allow mehtod chaining.
   */
  public function put() {
    if (empty($this->header)) {
      $this->header = ['Content-Type: application/json'];
    }
    // @TODO Minor. Implement more complex check. Regexp?
    if (in_array('Content-Type: application/json', $this->header)) {
      $this->payload = json_encode($this->payload);
    }

    $this->method = 'DELETE';
    $this->doRequest();

    return $this;
  }

  /**
   * Performs DELETE-request.
   *
   * @return object
   *   Returns current object to allow mehtod chaining.
   */
  public function delete() {
    $this->method = 'DELETE';
    $this->doRequest();

    return $this;
  }

  /**
   * Performs REST API calls.
   *
   * @return object
   *   Returns instance to allow method chaining.
   *
   * @throws Exception
   *   When response code is not 200-204.
   */
  public function doRequest() {
    $this->validateRequest();
    $this->method = strtoupper($this->method);
    $this->createRequestId();
    if (
      $this->useCachedResponse
      && RestAPIResponse::isValidExists($this->requestId)
    ) {
      // Reuse stored response if possible.
      foreach (RestAPIResponse::loadValid($this->requestId) as $key => $vlaue) {
        $this->$key = $value;
      }
    }
    else {
      $curl = curl_init();
      // Why not use CURLOPT_POS: https://stackoverflow.com/a/74812403
      curl_setopt($curl, CURLOPT_CUSTOMREQUEST, $this->method);
      curl_setopt($curl, CURLOPT_HTTPHEADER, $this->header);

      if (!empty($this->payload)) {
        if (in_array($this->method, ['POST', 'DELETE', 'PUT'])) {
          curl_setopt($curl, CURLOPT_SAFE_UPLOAD, TRUE);
          // Different processing of array and string:
          // https://stackoverflow.com/questions/5224790
          curl_setopt($curl, CURLOPT_POSTFIELDS, $this->payload);
        }
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      $this->url = url($this->url, [
        'https' => TRUE,
        'external' => TRUE,
        'query' => $this->query ?? [],
      ]);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Allow other modules to react on start of the request.
      if (function_exists('module_invoke_all')) {
        // tpps_rest_api_request_started().
        $this->generateTag();
        $context = [
          'request_id' => $this->requestId,
          'created' => $this->created,
          'method' => $this->method,
          'url' => $this->url,
          'payload' => $this->payload ?? NULL,
          'header' => $this->header ?? NULL,
          'tag' => $this->tag ?? NULL,
          // Caching settings.
          'cache_request' => $this->cacheRequest,
          'cache_response' => $this->cacheResponse,
          'use_cached_response' => $this->useCachedResponse,
        ];
        module_invoke_all('rest_api_request_started', $context);
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // OPTIONS:
      // See http://www.php.net/manual/en/function.curl-setopt.php
      curl_setopt($curl, CURLOPT_URL, $this->url);
      curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
      curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
      // The maximum number of seconds for curl operation.
      // Note: disabled because valid requests failed because of timeout.
      //curl_setopt($curl, CURLOPT_TIMEOUT, $this->getTimeout());
      //curl_setopt($curl, CURLOPT_CONNECTTIMEOUT, $this->getTimeout());

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // EXECUTE:
      $execution_time_start = microtime(1);
      $response = curl_exec($curl);
      $this->curlInfo = curl_getinfo($curl);
      curl_close($curl);

      // Validate HTTP request code.
      $this->code = $this->curlInfo['http_code'];
      $this->message = HttpCode::getMessage($this->code);
      // Execution time.
      $this->duration = (microtime(1) - $execution_time_start) * 1000;

      // Prepare response.
      $this->response = empty($response) ? NULL : $response;
      if (
        !empty($response)
        && $this->curlInfo['content_type'] == 'application/json'
      ) {
        // @todo Minor. Use try and catch block.
        // PHP's json_decode() could throw an exception of invalid JSON.
        $this->response = json_decode($this->response, TRUE);
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Allow other modules to react on request's finish.
      if (function_exists('module_invoke_all')) {
        // tpps_rest_api_request_completed().
        $context = [
          'request' => $context,
          'request_id' => $this->requestId,
          'code' => $this->code,
          'created' => $this->created,
          // 'message' => $message,
          'duration' => $this->duration,
          'body' => $this->response,
          // Caching settings.
          'cache_request' => $this->cacheRequest,
          'cache_response' => $this->cacheResponse,
          'use_cached_response' => $this->useCachedResponse,
        ];
        module_invoke_all('rest_api_request_completed', $context);
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    if ($this->code == 0 && empty($this->response)) {
      throw new Exception($this->message);
    }
    if ($this->code < 200  && $this->code > 204) {
      // Codes: https://developers.zenodo.org/?shell#http-status-codes
      $exception_data = [
        'payload' => empty($this->payload) ? [] : json_decode($this->payload),
        'url' => $this->url,
        'method' => $this->method,
        'header' => $this->header,
        'response' => $this->response,
        'code' => $this->code,
        'message' => $this->message,
        'curl_info' => $this->curlInfo,
        'backtrace' => debug_backtrace(
          !DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS
        ),
      ];
      if (function_exists('drupal_alter')) {
        // tpps_rest_api_exception_alter().
        drupal_alter('rest_api_exception', $this->message, $exception_data);
      }
      throw new Exception('RestAPI HTTP-request: ' . $this->message, $this->code);
    }

    return $this;
  }

  /**
   * Gets timeout value.
   *
   * The maximum number of seconds to allow cURL functions to execute.
   * To set in milliseconds use: CURLOPT_TIMEOUT_MS.
   * https://curl.se/libcurl/c/CURLOPT_TIMEOUT.html.
   *
   * @return int
   *   Returns value of the timeout in milliseconds.
   *
   * @todo Minor. ?Convert into private method?
   */
  public function getTimeout() {
    return variable_get('tpps_rest_api_timeout', 30);
  }

  /**
   * Replaces Request Id in 2 DB tables (request and response).
   *
   * @param string $new_request_id
   *   New Request Id. See method 'createRequestId()'.
   */
  public function replaceRequestId($new_request_id) {
    if (
      empty($request_id = $this->requestId)
      || $request_id == $new_request_id
    ) {
      return;
    }
    // Update 2 tables.
    $fields = ['request_id' => $new_request_id];
    $table_list = [RestAPIRequest::TABLE, RestAPIResponse::TABLE];

    $transaction = db_transaction();
    try {
      foreach ($table_list as $table_name) {
        db_update($table_name)
          ->fields($fields)
          ->condition('request_id', $request_id)
          ->execute();
      }
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('tpps_rest_api', $e);
    }
  }

}
