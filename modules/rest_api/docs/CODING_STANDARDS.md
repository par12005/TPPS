### 1. Drupal 7 Coding Standards (Drupal CS)
- **Formatting**: 2 spaces indentation (no tabs), single space after control keywords (`if (...) {`, `foreach (...) {`), and closing braces on their own line.
- **String Concatenation**: Always place spaces around the dot operator: `$output = $prefix . ' ' . $suffix;`.
- **Arrays**: In multi-line arrays, each item must be on its own line and end with a trailing comma.
- **Doxygen Documentation**:
  - Every file must begin with `/** @file ... */`.
  - Every function, method, class, constant, and property must have a complete docblock (`/** ... */`).
  - Hook documentation belongs in `rest_api.api.php`.
  - Standard hook implementations must use: `/** Implements hook_foo(). */` (single line, no `@param` repetition unless altering parameters).
- **Translation & Placeholders**: In `t()` and `watchdog()`:
  - `@var`: Escaped via `check_plain()` (use for plain strings, IDs, numbers).
  - `%var`: Emphasized and escaped via `drupal_placeholder()` (rendered in `<em>`).
  - `!var`: Raw HTML or sanitized URLs (use strictly for `l()` links or pre-escaped markup).
- **Theme API**: Never call `check_plain()` directly on variables passed to `l()` (as `l()` already sanitizes titles and URLs unless `'html' => TRUE` is passed). Validate optional external report paths with `drupal_valid_path()` before rendering links.

### 2. High-Performance Architecture & Resource Efficiency
- **Locking Without Cache Invalidation**: Use Drupal's native lock API (`lock_acquire()` / `lock_release()`) instead of `variable_set()` / `variable_del()`. Calling `variable_set()` clears `cache_bootstrap`, forcing a full database reload on every single API request.
- **Scoped Static Cache Resets**: Never call `drupal_static_reset()` without arguments, as that clears all static caches across Drupal core and contrib modules. Always pass the specific method name (`drupal_static_reset('RestApiResponse::load')`, `drupal_static_reset('RestApiResponse::loadValid')`).
- **Indexed Database Lookups & Lightweight Deduplication**:
  - Never query `TEXT` or `BLOB` columns with conditions like `->condition('body', '', '!=')` (unindexed table scans). Load the indexed record first, then validate the body in PHP.
  - When checking the latest response in `RestApiResponse::save()`, select only lightweight metadata columns (`['response_id', 'hash', 'code']`) rather than loading and decoding the full `body` (`LONGTEXT`) column.
  - Do not add `orderBy('created', 'DESC')` when querying by unique or single-column hash keys (`hash`, `request_id`).
  - In `getAll()`, enforce pagination via `->range($offset, $limit)` to prevent memory exhaustion on large datasets.
- **Safe Batched Cron Deletions**: Always bound cron purges with `->range(0, 1000)` and delete in deterministic primary-key order. Only purge `{rest_api_request}` rows when no child rows remain in `{rest_api_response}` (`LEFT JOIN ... isNull('r.response_id')`) so recurring requests are never deleted alongside active responses.
- **Static Caching with `drupal_static()`**: Store loaded database request IDs (`RestApiRequest::getRequestId`) and responses (`RestApiResponse::load`, `RestApiResponse::loadValid`) in `drupal_static(__METHOD__)` with an explicit `$reset = FALSE` argument to avoid repetitive queries within the same request lifecycle.
- **Debug-Gated Watchdog Logging**: Gate informational `WATCHDOG_INFO` entries behind `variable_get('rest_api_debug_mode', FALSE)` so production traffic only writes to `{watchdog}` on slow requests (`WATCHDOG_WARNING`) or errors (`WATCHDOG_ERROR`).
- **Selective Caching**: Cache only idempotent `GET` requests by default. Do not waste database I/O writing/reading cache entries for state-changing calls (`POST`, `PUT`, `DELETE`) unless explicitly flagged.
- **cURL Connection Reuse, DNS Caching & TCP Keep-Alive**:
  - Cache resolved public host IPs in memory (`RestApi::$dnsCache`, `DNS_CACHE_TTL = 60`) to eliminate redundant `dns_get_record()` calls across sequential or batched requests.
  - Reuse persistent cURL handles across requests to the same endpoint (`CURLOPT_FORBID_REUSE = FALSE`).
  - Share DNS, TLS sessions, and connection sockets across handles via `curl_share_init()`, eliminating 50–150 ms TLS handshake overhead on repeat requests.
  - Enable TCP Keep-Alive (`CURLOPT_TCP_KEEPALIVE = 1`, `CURLOPT_TCP_KEEPIDLE = 60`, `CURLOPT_TCP_KEEPINTVL = 30`) to keep sockets alive behind NAT firewalls.
  - Use HTTP/2 multiplexing over TLS (`CURL_HTTP_VERSION_2TLS`) where supported.
- **Concurrent Request Latency Measurement**: In `curl_multi` batches, measure individual duration using `curl_getinfo($ch, CURLINFO_TOTAL_TIME) * 1000` rather than the total loop wall-clock time.

### 3. Editor & Vim Standards
- **Line Width**: Strictly wrap code and comments within 80 characters.
- **Indentation & Whitespace**: Configure `set tabstop=2 shiftwidth=2 expandtab textwidth=80 autoindent`.
- **Clean Diffs**: Ensure Unix line endings (`set fileformat=unix`), single trailing newline at EOF, and no trailing whitespace.

### 4. OOP Design & Safe Magic Methods
- **No Dynamic Method Calls in `__get()` / `__set()`**: Never invoke `$this->$func()` or dynamically resolve `get*()` / `set*()` methods inside magic accessors, and do not return properties by reference (`&__get`) to prevent external mutation bypassing validation.
- **Strict Property Validation**: Magic methods must only access existing class properties and validate critical properties (`url`, `method`, `header`). Throw an exception for undefined properties.
- **State Independence**: Reset `$requestId = NULL`, `$responseId = NULL`, and `$hash = NULL` whenever request properties (`url`, `method`, `header`, `payload`, `query`) change, preventing mutation of historical records.

### 5. Security & SSRF Protection
- **Scheme Whitelist**: Allow only `http` and `https` (`ALLOWED_SCHEMES`).
- **Private IP Blocking**: Resolve hostnames and block loopback, link-local, cloud metadata, and RFC 1918 ranges via `filter_var(..., FILTER_FLAG_NO_PRIV_RANGE | FILTER_FLAG_NO_RES_RANGE)`.
- **DNS Rebinding Guard**: Bind all validated public IPv4 and bracketed IPv6 addresses to cURL handles via `CURLOPT_RESOLVE` (`host:port:ip1,[ipv6]`).
- **cURL Isolation**: Enforce `CURLPROTO_HTTP | CURLPROTO_HTTPS`, disable automatic redirects (`CURLOPT_FOLLOWLOCATION = FALSE`), and require strict TLS certificate verification (`CURLOPT_SSL_VERIFYPEER = TRUE`, `CURLOPT_SSL_VERIFYHOST = 2`).
- **Header & Secret Sanitization**: Strip `\r` and `\n` from headers. Mask bot tokens (`#(/bot\d+:)[A-Za-z0-9_-]+#`) and secret query parameters (`key`, `api_key`, `access_token`, `token`, `secret`, `password`) in URLs, and redact sensitive headers (`Authorization`, `X-Api-Key`, `Api-Key`, `Proxy-Authorization`, `Cookie`, `Set-Cookie`, `X-Auth-Token`, `X-Csrf-Token`, `Client-Secret`) prior to logging, exception context altering, or DB storage.

### 6. Admin UI & Settings UX
- **Grouped Fieldsets**: Organize settings into logical, collapsible fieldsets (Security, Caching/Logging, Connection Reuse, Timeouts).
- **Actionable Descriptions**: Provide default value indicators, operational mechanics, and guidance on production vs. local development.
- **Authoritative Standards**: Include outbound documentation links (RFC 1918, RFC 6890, OWASP SSRF Cheat Sheet, libcurl options).
- **Form Validation**: Validate with `#element_validate` and cross-field checks (e.g., `connect_timeout <= timeout`).
