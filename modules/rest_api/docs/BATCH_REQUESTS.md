# Concurrent Requests with `RestApi::doMultiRequest()`

The `RestApi::doMultiRequest()` method dispatches multiple HTTP requests concurrently using PHP's `curl_multi_*` engine. It shares DNS resolution (including an in-process 60-second validated DNS cache), TLS session tickets, and persistent TCP connections across requests, taking full advantage of HTTP/2 multiplexing where available.

---

## Method Signature

```php
/**
 * Executes multiple RestApi requests concurrently using curl_multi.
 *
 * @param RestApi[]|array[] $requests
 *   List of RestApi instances or associative request context arrays.
 * @param int $concurrency
 *   Maximum number of simultaneous connections (per host and total). Default: 10.
 *
 * @return RestApi[]
 *   List of completed RestApi instances, preserving the keys of $requests.
 */
public static function doMultiRequest(array $requests, $concurrency = 10);
```

---

## Usage Examples

### 1. Batch Dispatch via Context Arrays

Pass associative context arrays for rapid, lightweight batch execution (when `'method'` is omitted, `doMultiRequest()` defaults to `'GET'`):

```php
$requests = [
  'tg_user_1' => [
    'url' => 'https://api.telegram.org/bot123456:ABC-DEF/sendMessage',
    'method' => 'POST',
    'payload' => [
      'chat_id' => 1001,
      'text' => 'Notification for User 1',
    ],
  ],
  'tg_user_2' => [
    'url' => 'https://api.telegram.org/bot123456:ABC-DEF/sendMessage',
    'method' => 'POST',
    'payload' => [
      'chat_id' => 1002,
      'text' => 'Notification for User 2',
    ],
  ],
  'weather_kyiv' => [
    'url' => 'https://api.weatherapi.com/v1/current.json',
    'method' => 'GET',
    'query' => [
      'q' => 'Kyiv',
      'key' => 'YOUR_API_KEY',
    ],
    // Cache valid 2xx responses in the database
    'cache' => TRUE,
  ],
];

// Execute concurrently with a pool concurrency of up to 10 connections
$responses = RestApi::doMultiRequest($requests, 10);

foreach ($responses as $key => $api) {
  $status_code = $api->code;       // e.g. 200
  $body        = $api->response(); // Decoded JSON array or raw response string
  $duration_ms = $api->duration;   // Individual request execution time in ms
}
```

### 2. Fine-Grained Configuration via `RestApi` Instances

Instantiate `RestApi` objects when individual calls require custom headers, tags, localized timeouts, or SSRF protection overrides:

```php
$batch = [];

// Request 1: Internal microservice in LAN/Docker with SSRF guard disabled
$api_internal = new RestApi();
$api_internal->createRequest([
  'url' => 'http://internal-service.local/api/v1/sync',
  'method' => 'POST',
  'payload' => ['event' => 'user_signup'],
  'block_private_ips' => FALSE, // Bypass SSRF check for trusted internal endpoint
  'timeout' => 5,
  'tag' => 'Internal Service Sync',
]);
$batch['internal_sync'] = $api_internal;

// Request 2: External third-party API with bearer authentication
$api_external = new RestApi();
$api_external->createRequest([
  'url' => 'https://api.example.com/v2/items',
  'method' => 'GET',
  'header' => [
    'Authorization' => 'Bearer YOUR_ACCESS_TOKEN',
    'Accept' => 'application/json',
  ],
  'tag' => 'Fetch Catalog Items',
]);
$batch['external_catalog'] = $api_external;

// Dispatch concurrently with concurrency limit 5
$results = RestApi::doMultiRequest($batch, 5);

$internal_response = $results['internal_sync']->response();
$catalog_data      = $results['external_catalog']->response();
```

---

## Response Properties

Each element in the returned array is an instantiated `RestApi` object providing:

| Property / Method | Type | Description |
| :--- | :--- | :--- |
| `$api->code` | `int` | HTTP response status code (e.g., `200`, `204`, `400`). |
| `$api->response()` | `mixed` | Decoded response payload (associative array for JSON, or raw string). |
| `$api->duration` | `float` | Individual request latency in milliseconds (`total_time * 1000`). |
| `$api->message` | `string` | Human-readable HTTP status description or cURL error message. |
| `$api->curlInfo` | `array` | Complete metadata array returned by `curl_getinfo()`. |
| `$api->isCachedResponse()` | `bool` | Returns `TRUE` if the response was served from database cache. |

---

## Architectural & Execution Highlights

- **Database Cache Resolution**: Before opening a network socket, each request checks `{rest_api_response}` via `RestApiResponse::loadValid()` for an existing valid `2xx` response (respecting `rest_api_cache_ttl`). If a cached response is valid, it is loaded immediately without making an external HTTP connection.
- **In-Process DNS Resolution Cache**: Hostnames resolved by `validateAndResolveHost()` are cached in memory for 60 seconds (`RestApi::DNS_CACHE_TTL`), avoiding repetitive `dns_get_record()` lookups when dispatching large batches to the same host.
- **Automatic Payload Encoding**: Array payloads for `POST`, `PUT`, and `DELETE` requests are encoded to JSON via `prepareJsonPayload()` before transmission, even when custom headers (such as `Authorization`) are provided alongside the payload.
- **Per-Host & Total Concurrency Limits**: Configures both `CURLMOPT_MAX_HOST_CONNECTIONS` and `CURLMOPT_MAX_TOTAL_CONNECTIONS` to enforce the `$concurrency` limit across single-host and multi-host batches.
- **Accurate Latency Measurement**: `$api->duration` is computed from each cURL handle's `total_time` metric, reflecting actual network transfer duration rather than total batch elapsed time.
- **Failure Isolation**: If an individual request fails due to an HTTP error or network timeout, the exception is caught and logged to Drupal's watchdog via `watchdog_exception('rest_api', $e)`, preventing one failing call from interrupting the rest of the batch.
- **Connection Keep-Alive & HTTP/2**: Persistent handles leverage shared connection and SSL session caches (`curl_share_init`), reducing latency on repetitive calls to services such as Telegram Bot API or payment gateways.
