# Roadmap & Future Improvements

Here are high-level directions for future improvements across resilience, operations, and architecture:

## Major

* **Implement a Drupal-hook to hide secret data**: Allow other modules to decide which data must be hidden beside with common solution in this module.
* **Remove outdated invalid responses:** Remove all existing non-2xx responses if valid response with 2xx-code received or loaded.
* **Streaming and chunked file transfers**: Stream large file downloads directly to disk or output buffers (`CURLOPT_FILE`) to eliminate PHP memory ceilings.
* **Drush CLI integration**: Provide commands to purge logs, benchmark endpoint latency, replay requests, and inspect connection pools.
* **Standalone admin inspection and replay UI**: Build a built-in administrative interface under `admin/reports/rest_api` to filter, inspect headers/payloads, and manually replay logged API calls without external report modules.
* **Automated test suite**: Implement unit and integration tests (`DrupalWebTestCase`) with mock HTTP endpoints to regression-test SSRF protections, caching, and concurrency.

## Minor

* **Pluggable cache backends**: Offload response caching to Redis, Memcached, or standard Drupal cache bins (`cache_get()` / `cache_set()`) instead of relying exclusively on relational SQL tables.
* **Automatic retry with exponential backoff**: Retry transient network glitches (HTTP 429, 502, 503, 504, and cURL timeouts) with configurable jitter.
* **Circuit breaker pattern**: Fail fast on persistently degraded upstream hosts to preserve PHP-FPM workers and database connections.
* **Client-side rate limiting**: Enforce token-bucket or leaky-bucket request throttling to avoid exceeding third-party provider quotas.
* **Outbound HTTP / SOCKS5 proxy routing**: Support forward proxies, corporate egress gateways, and rotating IP pools.
* **Per-endpoint configuration presets**: Support reusable endpoint presets (base URLs, auth credentials, custom headers, timeouts) via CTools exportables.
* **Webhook signature verification toolkit**: Provide standardized helpers for validating incoming webhook signatures (HMAC-SHA256, RSA, Ed25519).
* **Prometheus and OpenTelemetry metrics**: Export request volumes, error ratios, and p95/p99 latency metrics to external observability platforms.
