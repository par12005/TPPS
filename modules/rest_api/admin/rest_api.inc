<?php

/**
 * @file
 * REST API Class Settings Form.
 */

/**
 * REST API Class Settings Form.
 */
function rest_api_settings_form(array $form, array &$form_state) {
  $ext_link_opts = [
    'attributes' => [
      'target' => '_blank',
      'rel' => 'noopener noreferrer',
    ],
  ];

  // 1. Security & SSRF Protection.
  $form['security'] = [
    '#type' => 'fieldset',
    '#title' => t('Security & SSRF Protection'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $owasp_url = 'https://cheatsheetseries.owasp.org/cheatsheets/'
    . 'Server_Side_Request_Forgery_Prevention_Cheat_Sheet.html';
  $form['security']['rest_api_block_private_ips'] = [
    '#type' => 'checkbox',
    '#title' => t(
      'Block requests to private, loopback, link-local, and reserved IP '
      . 'ranges (SSRF & DNS Rebinding protection)'
    ),
    '#default_value' => variable_get('rest_api_block_private_ips', TRUE),
    '#description' => t(
      '<strong>Default:</strong> Checked (strongly recommended for '
      . 'production).<br /><strong>How it works:</strong><ul>'
      . '<li>Restricts URLs to <code>http</code> and <code>https</code> '
      . 'schemes only (<code>CURLPROTO_HTTP | CURLPROTO_HTTPS</code>), '
      . 'disables automatic HTTP redirects (<code>CURLOPT_FOLLOWLOCATION = '
      . 'FALSE</code>), and enforces strict TLS certificate validation '
      . '(<code>CURLOPT_SSL_VERIFYPEER = TRUE</code>, '
      . '<code>CURLOPT_SSL_VERIFYHOST = 2</code>).</li>'
      . '<li>Blocks internal hostnames (<code>localhost</code>, '
      . '<code>*.localhost</code>, <code>*.local</code>, '
      . '<code>*.internal</code>) and resolves domain names beforehand via '
      . '<code>dns_get_record()</code> (with fallback to '
      . '<code>gethostbynamel()</code>).</li>'
      . '<li>Validates every resolved IPv4 and IPv6 address using '
      . '<code>filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE '
      . '| FILTER_FLAG_NO_RES_RANGE)</code>, blocking loopback '
      . '(<code>127.0.0.0/8</code>, <code>::1</code>), private LAN ranges '
      . '(<code>10.0.0.0/8</code>, <code>172.16.0.0/12</code>, '
      . '<code>192.168.0.0/16</code>), link-local and cloud metadata '
      . 'endpoints (<code>169.254.0.0/16</code> including '
      . '<code>169.254.169.254</code>, <code>fe80::/10</code>), and IPv6 '
      . 'unique local addresses (<code>fc00::/7</code>).</li>'
      . '<li>Pins the verified public IP addresses directly to the cURL '
      . 'handle via <code>CURLOPT_RESOLVE</code> '
      . '(<code>host:port:ip1,ip2</code>) so libcurl connects to the exact '
      . 'validated IP, preventing DNS Rebinding (TOCTOU) attacks.</li>'
      . '</ul><strong>When to disable or override:</strong> Keep this enabled '
      . 'on public-facing servers. Uncheck globally only in isolated local '
      . 'development environments, or keep global protection enabled and '
      . 'override per request for trusted internal microservices in '
      . 'LAN/Docker/VPN by passing <code>\'block_private_ips\' =&gt; '
      . 'FALSE</code> in <code>new RestApi($context)</code>.'
      . '<br /><strong>References:</strong> !owasp | !rfc1918 | !rfc6890.',
      [
        '!owasp' => l(
          t('OWASP SSRF Prevention Cheat Sheet'),
          $owasp_url,
          $ext_link_opts
        ),
        '!rfc1918' => l(
          t('RFC 1918 (Private IPv4 Address Allocation)'),
          'https://datatracker.ietf.org/doc/html/rfc1918',
          $ext_link_opts
        ),
        '!rfc6890' => l(
          t('RFC 6890 (Special-Purpose IP Address Registries)'),
          'https://datatracker.ietf.org/doc/html/rfc6890',
          $ext_link_opts
        ),
      ]
    ),
  ];

  // 2. Request/Response Caching & Database Logging.
  $form['cache'] = [
    '#type' => 'fieldset',
    '#title' => t('Request/Response Caching & Database Logging'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $form['cache']['rest_api_use_cache'] = [
    '#type' => 'checkbox',
    '#title' => t('Use database response cache'),
    '#default_value' => variable_get('rest_api_use_cache', TRUE),
    '#description' => t(
      '<strong>Default:</strong> Checked.<br />When enabled, valid cached '
      . '<code>2xx</code> responses stored in <code>{rest_api_response}</code> '
      . 'are reused instead of performing a live HTTP call. By default, '
      . 'caching is enabled only for idempotent <code>GET</code> requests; '
      . 'state-changing requests (<code>POST</code>, <code>PUT</code>, '
      . '<code>DELETE</code>) bypass the cache unless <code>\'cache\' =&gt; '
      . 'TRUE</code> is explicitly passed in the request context.'
    ),
  ];
  $form['cache']['rest_api_cache_ttl'] = [
    '#type' => 'textfield',
    '#title' => t('Cache TTL (Time-To-Live), seconds'),
    '#default_value' => variable_get('rest_api_cache_ttl', 0),
    '#description' => t(
      '<strong>Default:</strong> <code>0</code> (cached responses never '
      . 'expire).<br />Maximum age in seconds for a stored <code>2xx</code> '
      . 'response to be considered valid by '
      . '<code>RestApiResponse::loadValid()</code>. Set to <code>0</code> for '
      . 'permanent caching of immutable reference data (such as external '
      . 'taxonomy lookups), or set a positive value (e.g. <code>300</code>–'
      . '<code>3600</code> seconds) when querying dynamic third-party APIs.'
    ),
  ];
  $form['cache']['rest_api_use_log'] = [
    '#type' => 'checkbox',
    '#title' => t('Log requests and responses in the database'),
    '#default_value' => variable_get('rest_api_use_log', TRUE),
    '#description' => t(
      '<strong>Default:</strong> Checked.<br />Stores request metadata in '
      . '<code>{rest_api_request}</code> and response payloads in '
      . '<code>{rest_api_response}</code>. Sensitive credentials (such as '
      . 'Telegram Bot API tokens, URL secret parameters, and '
      . '<code>Authorization</code>, <code>X-Api-Key</code>, '
      . '<code>Cookie</code>, and <code>Proxy-Authorization</code> headers) '
      . 'are automatically redacted via '
      . '<code>RestApiRequest::sanitizeContext()</code> before saving.'
    ),
  ];
  $form['cache']['rest_api_log_retention_days'] = [
    '#type' => 'textfield',
    '#title' => t('Database log retention period, days'),
    '#default_value' => variable_get('rest_api_log_retention_days', 30),
    '#description' => t(
      '<strong>Default:</strong> <code>30</code> days (set to <code>0</code> '
      . 'to keep records indefinitely).<br />During <code>hook_cron()</code> '
      . '(<code>rest_api_cron()</code>), records older than the configured '
      . 'number of days are automatically purged in batches of 1,000 rows to '
      . 'prevent unbounded growth of <code>{rest_api_request}</code> and '
      . '<code>{rest_api_response}</code>.'
    ),
  ];
  $watchdog_url = 'https://api.drupal.org/api/drupal/'
    . 'includes%21bootstrap.inc/function/watchdog/7.x';
  $form['cache']['rest_api_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('Enable verbose watchdog debug logging'),
    '#default_value' => variable_get('rest_api_debug_mode', FALSE),
    '#description' => t(
      '<strong>Default:</strong> Unchecked.<br />When enabled, logs '
      . 'sanitized request contexts and informational cache/save events to '
      . 'Drupal\'s system log via !watchdog_link. Enable temporarily when '
      . 'troubleshooting API integrations; keep disabled in high-traffic '
      . 'production environments to avoid <code>{watchdog}</code> table bloat.',
      [
        '!watchdog_link' => l(
          t('Drupal 7 watchdog()'),
          $watchdog_url,
          $ext_link_opts
        ),
      ]
    ),
  ];

  // 3. Connection Reuse & TCP Keep-Alive.
  $form['connection'] = [
    '#type' => 'fieldset',
    '#title' => t('Connection Reuse & TCP Keep-Alive'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $form['connection']['rest_api_keep_alive'] = [
    '#type' => 'checkbox',
    '#title' => t('Reuse cURL connections and TLS sessions (Keep-Alive)'),
    '#default_value' => variable_get('rest_api_keep_alive', TRUE),
    '#description' => t(
      '<strong>Default:</strong> Checked.<br />Reuses a persistent cURL '
      . 'handle, shared DNS/TLS session/connection pools (!curl_share_link), '
      . 'HTTP/2 multiplexing over TLS, and TCP Keep-Alive (!keepalive_link) '
      . 'to eliminate repeated TCP and TLS handshakes (saving 50–150 ms per '
      . 'request) when calling the same host repeatedly (e.g., '
      . '<code>https://api.telegram.org</code>).',
      [
        '!curl_share_link' => l(
          'curl_share_init()',
          'https://www.php.net/manual/en/function.curl-share-init.php',
          $ext_link_opts
        ),
        '!keepalive_link' => l(
          'CURLOPT_TCP_KEEPALIVE',
          'https://curl.se/libcurl/c/CURLOPT_TCP_KEEPALIVE.html',
          $ext_link_opts
        ),
      ]
    ),
  ];
  $form['connection']['rest_api_tcp_keepidle'] = [
    '#type' => 'textfield',
    '#title' => t('TCP Keep-Alive Idle Time (CURLOPT_TCP_KEEPIDLE), seconds'),
    '#default_value' => variable_get('rest_api_tcp_keepidle', 60),
    '#description' => t(
      '<strong>Default:</strong> <code>60</code> seconds.<br />The idle time '
      . 'in seconds before libcurl starts sending TCP keep-alive probes on a '
      . 'persistent socket (!keepidle_link). For long-running queue workers '
      . 'behind NAT gateways or cloud firewalls that drop idle connections '
      . 'after 60–120 seconds, set this between <code>30</code> and '
      . '<code>60</code> seconds.',
      [
        '!keepidle_link' => l(
          'CURLOPT_TCP_KEEPIDLE',
          'https://curl.se/libcurl/c/CURLOPT_TCP_KEEPIDLE.html',
          $ext_link_opts
        ),
      ]
    ),
    '#element_validate' => ['element_validate_integer_positive'],
  ];
  $form['connection']['rest_api_tcp_keepintvl'] = [
    '#type' => 'textfield',
    '#title' => t(
      'TCP Keep-Alive Interval (CURLOPT_TCP_KEEPINTVL), seconds'
    ),
    '#default_value' => variable_get('rest_api_tcp_keepintvl', 30),
    '#description' => t(
      '<strong>Default:</strong> <code>30</code> seconds.<br />The interval '
      . 'in seconds between subsequent TCP keep-alive probes once idle '
      . 'probing begins (!keepintvl_link). Values of <code>15</code>–'
      . '<code>30</code> seconds allow background workers to detect '
      . 'half-open sockets quickly before the next request is dispatched.',
      [
        '!keepintvl_link' => l(
          'CURLOPT_TCP_KEEPINTVL',
          'https://curl.se/libcurl/c/CURLOPT_TCP_KEEPINTVL.html',
          $ext_link_opts
        ),
      ]
    ),
    '#element_validate' => ['element_validate_integer_positive'],
  ];

  // 4. Timeouts & Slow Request Monitoring.
  $form['time'] = [
    '#type' => 'fieldset',
    '#title' => t('Timeouts & Slow Request Monitoring'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $form['time']['rest_api_connect_timeout'] = [
    '#type' => 'textfield',
    '#title' => t('Connection timeout (CURLOPT_CONNECTTIMEOUT), seconds'),
    '#default_value' => variable_get('rest_api_connect_timeout', 10),
    '#description' => t(
      '<strong>Default:</strong> <code>10</code> seconds.<br />Maximum time '
      . 'in seconds allowed for the initial DNS lookup and TCP/TLS '
      . 'connection phase (!connect_timeout_link). Recommended: '
      . '<code>3</code>–<code>5</code> seconds for synchronous web page '
      . 'requests to prevent PHP-FPM worker exhaustion, or <code>10</code>–'
      . '<code>15</code> seconds for background Cron/CLI jobs. Must be less '
      . 'than or equal to the overall request timeout.',
      [
        '!connect_timeout_link' => l(
          'CURLOPT_CONNECTTIMEOUT',
          'https://curl.se/libcurl/c/CURLOPT_CONNECTTIMEOUT.html',
          $ext_link_opts
        ),
      ]
    ),
    '#element_validate' => ['element_validate_integer_positive'],
  ];
  $form['time']['rest_api_timeout'] = [
    '#type' => 'textfield',
    '#title' => t('Overall request timeout (CURLOPT_TIMEOUT), seconds'),
    '#default_value' => variable_get('rest_api_timeout', 30),
    '#description' => t(
      '<strong>Default:</strong> <code>30</code> seconds.<br />Maximum total '
      . 'time in seconds allowed for the entire cURL transfer to complete, '
      . 'including connection and payload download (!timeout_link). '
      . 'Recommended: <code>5</code>–<code>15</code> seconds for synchronous '
      . 'web requests, or <code>30</code>–<code>120</code> seconds for bulk '
      . 'background tasks in Cron/Drush.',
      [
        '!timeout_link' => l(
          'CURLOPT_TIMEOUT',
          'https://curl.se/libcurl/c/CURLOPT_TIMEOUT.html',
          $ext_link_opts
        ),
      ]
    ),
    '#element_validate' => ['element_validate_integer_positive'],
  ];
  $form['time']['rest_api_max_execution_time'] = [
    '#type' => 'textfield',
    '#title' => t('Slow request warning threshold, milliseconds'),
    '#default_value' => variable_get('rest_api_max_execution_time', 5000),
    '#description' => t(
      '<strong>Default:</strong> <code>5000</code> ms (5 seconds).<br />Any '
      . 'request whose execution duration meets or exceeds this threshold is '
      . 'logged to <code>watchdog()</code> with <code>WATCHDOG_WARNING</code> '
      . 'severity even when database logging or debug mode is disabled. Use '
      . '<code>1000</code>–<code>3000</code> ms to monitor latency-sensitive '
      . 'user flows, or <code>5000</code>+ ms for external batch APIs.'
    ),
    '#element_validate' => ['element_validate_integer_positive'],
  ];

  $form['#validate'][] = 'rest_api_settings_form_validate';

  return system_settings_form($form);
}

/**
 * Form validation handler for rest_api_settings_form().
 */
function rest_api_settings_form_validate(array $form, array &$form_state) {
  $values = $form_state['values'];

  $non_neg_fields = ['rest_api_cache_ttl', 'rest_api_log_retention_days'];
  foreach ($non_neg_fields as $field) {
    $val = $values[$field] ?? '';
    if (
      $val === ''
      || !is_numeric($val)
      || intval($val) != $val
      || (int) $val < 0
    ) {
      form_set_error(
        $field,
        t(
          '@field must be a non-negative integer (0 or greater).',
          ['@field' => $field]
        )
      );
    }
  }

  $connect_timeout = (int) ($values['rest_api_connect_timeout'] ?? 0);
  $timeout = (int) ($values['rest_api_timeout'] ?? 0);
  if ($connect_timeout > 0 && $timeout > 0 && $connect_timeout > $timeout) {
    form_set_error(
      'rest_api_connect_timeout',
      t(
        'The connection timeout (@connect s) cannot exceed the overall '
        . 'request timeout (@timeout s).',
        [
          '@connect' => $connect_timeout,
          '@timeout' => $timeout,
        ]
      )
    );
  }
}
