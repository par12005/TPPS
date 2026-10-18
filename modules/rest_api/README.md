# REST API (Drupal 7 Module)

A high-performance, security-hardened HTTP and REST API client module for Drupal 7. Built on top of PHP's `libcurl`, it provides persistent connection reuse, TCP Keep-Alive, HTTP/2 multiplexing, SSRF and DNS Rebinding protection with in-process DNS caching, database-backed response caching and logging, and concurrent multi-request dispatching.

---

## Key Features

- **SSRF & DNS Rebinding Hardening**:
  - Restricts URL schemes strictly to `http` and `https` (`CURLPROTO_HTTP | CURLPROTO_HTTPS`).
  - Disables automatic HTTP redirects (`CURLOPT_FOLLOWLOCATION = FALSE`) and enforces strict TLS certificate verification (`CURLOPT_SSL_VERIFYPEER = TRUE`, `CURLOPT_SSL_VERIFYHOST = 2`).
  - Resolves hostnames beforehand via `dns_get_record()` / `gethostbynamel()` with a 60-second in-process static cache (`RestApi::DNS_CACHE_TTL`), validating that all resolved IPv4/IPv6 addresses are public and globally routable. Blocks `127.0.0.0/8`, `10.0.0.0/8`, `172.16.0.0/12`, `192.168.0.0/16`, `169.254.0.0/16` (including cloud metadata `169.254.169.254`), `::1`, `fe80::/10`, and `fc00::/7`.
  - Binds all verified IPv4 and bracketed IPv6 addresses to the cURL handle via `CURLOPT_RESOLVE` (`host:port:ip1,[ipv6]`) to eliminate Time-of-Check to Time-of-Use (TOCTOU) DNS Rebinding vulnerabilities while preserving multi-IP failover.
  - Can be bypassed per request for trusted internal microservices (`'block_private_ips' => FALSE`).

- **Connection Reuse & Low Latency**:
  - Reuses persistent easy handles across sequential requests in the same PHP worker.
  - Leverages shared session pools (`curl_share_init`) for DNS cache, SSL/TLS sessions, and active socket pools.
  - Enables TCP Keep-Alive (`CURLOPT_TCP_KEEPALIVE`, `CURLOPT_TCP_KEEPIDLE`, `CURLOPT_TCP_KEEPINTVL`) and HTTP/2 over TLS (`CURL_HTTP_VERSION_2TLS`), cutting 50–150 ms of TLS handshake overhead per call.

- **Deduplication, Caching & Log Retention**:
  - Automatically deduplicates and caches valid `2xx` responses in the database using `LONGTEXT` storage (`{rest_api_response}.body`, supporting payloads up to 4 GB).
  - Caches only idempotent `GET` requests by default; state-changing requests (`POST`, `PUT`, `DELETE`) bypass cache unless `'cache' => TRUE` is explicitly provided.
  - Uses lightweight metadata checks in `RestApiResponse::save()` and scoped `drupal_static_reset()` calls (`RestApiResponse::load`, `RestApiResponse::loadValid`) so core/contrib static caches are never flushed on API writes.
  - Configurable Cache TTL (`rest_api_cache_ttl`) and safe automated log pruning via `hook_cron()` in bounded batches of 1,000 records (deleting old `{rest_api_request}` records only when no responses remain).
  - Enforces foreign key referential integrity with cascading deletes (`ON DELETE CASCADE`).

- **Sensitive Data & Credential Redaction**:
  - Automatically sanitizes CR/LF characters (`\r`, `\n`) from all HTTP request headers.
  - Masks Telegram bot tokens in URLs (`#(/bot\d+:)[A-Za-z0-9_-]+#`), redacts secret URL query parameters (`key`, `api_key`, `apikey`, `access_token`, `token`, `auth_token`, `secret`, `client_secret`, `password`), and redacts sensitive HTTP headers (`Authorization`, `X-Api-Key`, `Api-Key`, `Proxy-Authorization`, `Cookie`, `Set-Cookie`, `X-Auth-Token`, `X-Csrf-Token`, `Client-Secret`) before writing to the database, exception alter contexts, or watchdog debug logs.

- **Concurrent Execution Engine & Concurrency Locking**:
  - Dispatches parallel HTTP requests via `RestApi::doMultiRequest()` powered by non-blocking `curl_multi_*` with both per-host (`CURLMOPT_MAX_HOST_CONNECTIONS`) and total (`CURLMOPT_MAX_TOTAL_CONNECTIONS`) concurrency caps.
  - Prevents duplicate concurrent in-flight requests via Drupal's native lock API (`lock_acquire('rest_api:' . $hash, 2.0)`) unless `'parallel' => TRUE` is specified.
  - Isolates individual request errors and calculates per-request network duration from `CURLINFO_TOTAL_TIME`.

---

## Requirements

- **Drupal**: 7.x
- **PHP**: 7.2+ (PHP 7.4 / 8.x compatible)
- **PHP Extensions**: `curl`, `json`, `filter`, `mbstring`

---

## Installation

1. Copy the `rest_api` module directory into your site's modules directory (e.g., `sites/all/modules/rest_api` or `sites/default/modules/rest_api`).
2. Enable the module via Drush or the Drupal administrative interface:
   ```bash
   drush en rest_api -y
   ```
3. Run pending database updates to apply schema and index upgrades (`rest_api_update_7101`, `rest_api_update_7102`, `rest_api_update_7103`):
   ```bash
   drush updatedb -y
   ```

---

## Configuration

Navigate to **Administration > Configuration > Web services > REST API** (`admin/config/services/rest_api`).

### 1. Security & SSRF Protection
- **Block private & reserved IP ranges (`rest_api_block_private_ips`)**: Enabled by default. Resolves hosts beforehand, validates public routing, and locks cURL via `CURLOPT_RESOLVE`. Keep enabled on production servers; uncheck globally only in isolated local Docker environments without internet access.

### 2. Request/Response Caching & Database Logging
- **Use database response cache (`rest_api_use_cache`)**: Reuses stored `2xx` responses for identical request hashes.
- **Cache TTL (`rest_api_cache_ttl`)**: Time-to-live in seconds (`0` = cache indefinitely). Recommended: `300`–`3600` for dynamic third-party APIs, or `0` for static taxonomy/reference data.
- **Log requests and responses (`rest_api_use_log`)**: Saves sanitized requests and responses to `{rest_api_request}` and `{rest_api_response}`.
- **Log retention period (`rest_api_log_retention_days`)**: Number of days before old logs are purged during cron (`0` = never purge). Default: `30` days.
- **Verbose watchdog debug logging (`rest_api_debug_mode`)**: Logs sanitized request dumps and informational save/cache-hit events to `{watchdog}`. Keep disabled in high-traffic production so only warnings and errors are written to `{watchdog}`.

### 3. Connection Reuse & TCP Keep-Alive
- **Reuse cURL connections & TLS sessions (`rest_api_keep_alive`)**: Enabled by default. Shares connection pools and keeps sockets warm.
- **TCP Keep-Alive Idle Time (`rest_api_tcp_keepidle`)**: Default `60` seconds.
- **TCP Keep-Alive Interval (`rest_api_tcp_keepintvl`)**: Default `30` seconds.

### 4. Timeouts & Slow Request Monitoring
- **Connection Timeout (`rest_api_connect_timeout`)**: Default `10` seconds (recommended: `3`–`5` for web requests, `10`–`15` for cron). Must be less than or equal to overall timeout.
- **Overall Request Timeout (`rest_api_timeout`)**: Default `30` seconds.
- **Slow Request Warning Threshold (`rest_api_max_execution_time`)**: Default `5000` ms (5 seconds). Requests exceeding this limit trigger a `WATCHDOG_WARNING` even if logging is off.

---

## Usage Guide

### 1. Basic GET Request

```php
// Fluent syntax:
$data = (new RestApi([
  'url' => 'https://api.example.com/v1/users',
  'query' => ['status' => 'active'],
  'tag' => 'Active users list',
]))->get()->response();

// Object syntax:
$api = new RestApi();
$api->createRequest([
  'url' => 'https://api.example.com/v1/users/42',
  'method' => 'GET',
]);
$api->doRequest();

$status_code = $api->code;       // 200
$user_data   = $api->response(); // Decoded JSON array or string
$duration_ms = $api->duration;   // Execution duration in milliseconds
```

### 2. POST / PUT / DELETE with JSON Payload

Array payloads are automatically JSON-encoded, and `Content-Type: application/json` is added automatically whenever no explicit `Content-Type` header is supplied:

```php
$api = new RestApi();
$api->createRequest([
  'url' => 'https://api.example.com/v1/users',
  'method' => 'POST',
  'payload' => [
    'name' => 'Alice',
    'email' => 'alice@example.com',
  ],
  'header' => [
    'Authorization' => 'Bearer sample_access_token_123',
  ],
]);
$api->doRequest();

if ($api->code === 201) {
  $created_user = $api->response();
}
```

### 3. Binary File Download

To download files without database payload bloat or caching:

```php
$api = new RestApi(['url' => 'https://example.com/files/report.pdf']);
$binary_content = $api->getContent();
file_unmanaged_save_data($binary_content, 'public://report.pdf', FILE_EXISTS_REPLACE);
```

### 4. Bypassing SSRF Guard for Internal Microservices

If you need to query internal container or LAN endpoints while keeping global SSRF protection enabled:

```php
$api = new RestApi([
  'url' => 'http://billing-service.internal:8080/invoice/generate',
  'method' => 'POST',
  'payload' => ['order_id' => 9812],
  'block_private_ips' => FALSE, // Explicitly allow private/internal hostnames
]);
$api->doRequest();
```

### 5. Concurrent Requests (`RestApi::doMultiRequest`)

Dispatch multiple requests simultaneously over multiplexed HTTP/2 or pipelined connections (defaults to `GET` when `'method'` is omitted):

```php
$requests = [
  'item_1' => [
    'url' => 'https://api.example.com/items/1',
    'method' => 'GET',
    'cache' => TRUE,
  ],
  'item_2' => [
    'url' => 'https://api.example.com/items/2',
    'method' => 'GET',
    'cache' => TRUE,
  ],
  'notify' => [
    'url' => 'https://api.telegram.org/bot123456:ABC-DEF/sendMessage',
    'method' => 'POST',
    'payload' => ['chat_id' => 1001, 'text' => 'Batch processing started'],
  ],
];

// Execute up to 10 connections in parallel
$results = RestApi::doMultiRequest($requests, 10);

$item1_data = $results['item_1']->response();
$notify_res = $results['notify']->response();
```

### 6. Duplicate Request Locking (`parallel` Option)

By default (`'parallel' => FALSE`), `RestApi::doRequest()` acquires a 2-second semaphore lock (`lock_acquire('rest_api:' . $hash, 2.0)`) to prevent concurrent processes from firing identical requests simultaneously. If a lock cannot be acquired, `doRequest()` logs a `WATCHDOG_ERROR` and returns `FALSE`. To allow identical requests to run in parallel across processes, pass `'parallel' => TRUE` in the request context.

---

## Response Object Inspection

| Property / Method | Type | Description |
| :--- | :--- | :--- |
| `$api->code` | `int` | HTTP response status code (e.g., `200`, `204`, `404`). |
| `$api->response()` | `mixed` | Decoded response body (associative array for JSON, string for text/XML). |
| `$api->body` | `mixed` | Direct access to the decoded or raw response body. |
| `$api->duration` | `float` | Individual request latency in milliseconds (`total_time * 1000`). |
| `$api->message` | `string` | Status message (e.g., `"OK"`, `"Not Found"`) or cURL error description. |
| `$api->curlInfo` | `array` | Complete metadata array returned by `curl_getinfo()`. |
| `$api->isCachedResponse()`| `bool` | Returns `TRUE` if loaded from `{rest_api_response}` database cache. |
| `$api->requestId` | `int\|string\|null` | Database ID of the saved request record in `{rest_api_request}`. |
| `$api->responseId` | `int\|string\|null` | Database ID of the saved response record in `{rest_api_response}`. |

---

## Drupal Hooks & Alter Points

See `rest_api.api.php` for full hook signatures and parameter documentation:

### `hook_rest_api_request_started(array $context)`
Invoked right before a live or cached request is executed. Allows inspection or recording of initial request metadata (`request_id`, `response_id`, `hash`, `created`, `method`, `url`, `payload`, `header`, `tag`, `cache`, `log`).

### `hook_rest_api_response_received(array $context)`
Invoked immediately after cURL completes and response headers/body are parsed. Contains `'request'`, `'request_id'`, `'code'`, `'created'`, `'body'`, `'duration'`, `'content_type'`, `'curl_info'`, `'response_id'`, `'cache'`, and `'log'`.

### `hook_rest_api_response_loaded(array $context)`
Fired when an existing cached response is loaded from the database (`'request_id'` and `'response_id'`) instead of executing an external network call.

### `hook_rest_api_exception_alter(&$message, array $exception_context)`
Allows custom modules to inspect third-party error formats (e.g., Telegram Bot API error descriptions) and modify the exception message thrown by `$api->doRequest()`. URLs and headers in `$exception_context` are automatically sanitized via `RestApiRequest::sanitizeContext()`.

### `hook_rest_api_response_hash_generation_alter(array &$context)`
Allows altering response body data before hashing (for example, stripping dynamic timestamps or CSRF tokens) so identical responses are not duplicated in the database.

---

## Database Architecture

### `{rest_api_request}`
Logs unique outgoing API requests.
- `request_id`: Serial primary key (`INT UNSIGNED`).
- `created`: Unix timestamp when the request was first created.
- `method`: HTTP method (`VARCHAR(10)`: `GET`, `POST`, `PUT`, `DELETE`).
- `url`: Sanitized full endpoint URL with query params (`VARCHAR(2048)`).
- `payload`: Request payload JSON string (`TEXT`).
- `header`: Sanitized and normalized HTTP request headers (`TEXT`).
- `hash`: MD5 hash of method, URL, payload, and headers (`VARCHAR(32)` - `UNIQUE KEY`).
- `tag`: Human-readable identifier or label (`VARCHAR(255)`).

### `{rest_api_response}`
Logs API responses linked to requests.
- `response_id`: Serial primary key (`INT UNSIGNED`).
- `request_id`: Foreign key referencing `{rest_api_request}.request_id` (`ON DELETE CASCADE`).
- `created`: Unix timestamp when the response was captured.
- `code`: HTTP status code (`INT`).
- `duration`: Request execution time in milliseconds (`INT UNSIGNED`, default `0`).
- `body`: Response body payload (`LONGTEXT` / `'size' => 'big'`, up to 4 GB).
- `content_type`: Response MIME type (`VARCHAR(255)`).
- `hash`: MD5 hash of the response body (`VARCHAR(32)`).

---

## Security & Coding Standards

This module strictly adheres to:
- **Drupal 7 Coding Standards**: Checked via `phpcs --standard=Drupal` and documented in `docs/CODING_STANDARDS.md`.
- **OWASP SSRF Guidelines**: Pre-resolution IP validation, DNS Rebinding prevention via `CURLOPT_RESOLVE` with IPv6 bracket formatting, and sensitive header/query/token masking.
- **Resource Protection**: Uses Drupal's `lock_acquire()` API to prevent concurrent thundering herds and scoped `drupal_static_reset()` calls to preserve bootstrap and entity caches.
