<?php

/**
 * @file
 * Contains 'HttpCode' class.
 *
 * Theme:
 *   $formatted = theme('http_code', ['code' => $code]);
 */

/**
 * Manages HTTP response status codes and human-readable descriptions.
 *
 * @package rest_api
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class HttpCode {

  /**
   * HTTP 200 OK status code.
   */
  const OK = 200;

  /**
   * Gets message by HTTP response code.
   *
   * @param int|null $code
   *   Optional HTTP response code. When NULL, returns all official codes.
   *
   * @return string|array
   *   Human-readable response status message, or full list of codes when $code
   *   is NULL.
   */
  public static function getMessage($code = NULL) {
    $http_code_list = [
      // 1xx Informational.
      100 => 'Continue',
      101 => 'Switching Protocols',
      // WebDAV; RFC 2518.
      102 => 'Processing',
      // RFC 8297.
      103 => 'Early Hints',
      // 2xx Success.
      200 => 'OK',
      201 => 'Created',
      202 => 'Accepted',
      // Since HTTP/1.1.
      203 => 'Non-Authoritative Information',
      204 => 'No Content',
      205 => 'Reset Content',
      // RFC 7233.
      206 => 'Partial Content',
      // WebDAV; RFC 4918.
      207 => 'Multi-Status',
      // WebDAV; RFC 5842.
      208 => 'Already Reported',
      // RFC 3229.
      226 => 'IM Used',
      // 3xx Redirection.
      300 => 'Multiple Choices',
      301 => 'Moved Permanently',
      // Previously "Moved temporarily".
      302 => 'Found',
      // Since HTTP/1.1.
      303 => 'See Other',
      // RFC 7232.
      304 => 'Not Modified',
      // Since HTTP/1.1.
      305 => 'Use Proxy',
      306 => 'Switch Proxy',
      // Since HTTP/1.1.
      307 => 'Temporary Redirect',
      // RFC 7538.
      308 => 'Permanent Redirect',
      // 4xx Client Error.
      400 => 'Bad Request',
      // RFC 7235.
      401 => 'Unauthorized',
      402 => 'Payment Required',
      403 => 'Forbidden',
      404 => 'Not Found',
      405 => 'Method Not Allowed',
      406 => 'Not Acceptable',
      // RFC 7235.
      407 => 'Proxy Authentication Required',
      408 => 'Request Timeout',
      409 => 'Conflict',
      410 => 'Gone',
      411 => 'Length Required',
      // RFC 7232.
      412 => 'Precondition Failed',
      // RFC 7231.
      413 => 'Payload Too Large',
      // RFC 7231.
      414 => 'URI Too Long',
      // RFC 7231.
      415 => 'Unsupported Media Type',
      // RFC 7233.
      416 => 'Range Not Satisfiable',
      417 => 'Expectation Failed',
      // RFC 2324, RFC 7168.
      418 => 'I\'m a teapot',
      // RFC 7540.
      421 => 'Misdirected Request',
      // WebDAV; RFC 4918.
      422 => 'Unprocessable Entity',
      // WebDAV; RFC 4918.
      423 => 'Locked',
      // WebDAV; RFC 4918.
      424 => 'Failed Dependency',
      // RFC 8470.
      425 => 'Too Early',
      426 => 'Upgrade Required',
      // RFC 6585.
      428 => 'Precondition Required',
      // RFC 6585.
      429 => 'Too Many Requests',
      // RFC 6585.
      431 => 'Request Header Fields Too Large',
      // RFC 7725.
      451 => 'Unavailable For Legal Reasons',
      // 5xx Server Error.
      500 => 'Internal Server Error',
      501 => 'Not Implemented',
      502 => 'Bad Gateway',
      503 => 'Service Unavailable',
      504 => 'Gateway Timeout',
      505 => 'HTTP Version Not Supported',
      // RFC 2295.
      506 => 'Variant Also Negotiates',
      // WebDAV; RFC 4918.
      507 => 'Insufficient Storage',
      // WebDAV; RFC 5842.
      508 => 'Loop Detected',
      // RFC 2774.
      510 => 'Not Extended',
      // RFC 6585.
      511 => 'Network Authentication Required',
    ];

    if (!is_null($code)) {
      if ($code === 0) {
        return 'cURL request failed or timed out.';
      }
      if (isset($http_code_list[$code])) {
        return $http_code_list[$code];
      }
      return static::getUnofficialMessage($code) ?? 'Unknown';
    }
    return $http_code_list;
  }

  /**
   * Gets unofficial message by HTTP response code.
   *
   * Content from https://en.wikipedia.org/wiki/List_of_HTTP_status_codes.
   *
   * @param int|null $code
   *   Optional HTTP response code.
   *
   * @return string|array
   *   Human-readable response status message, or full list of unofficial codes
   *   when $code is NULL.
   */
  public static function getUnofficialMessage($code = NULL) {
    $http_code_list = [
      103 => 'Checkpoint',
      // Apache Web Server.
      218 => 'This is fine',
      // Laravel Framework.
      419 => 'Page Expired',
      // Spring Framework / Twitter.
      420 => 'Method Failure / Enhance Your Calm',
      // Shopify.
      430 => 'Request Header Fields Too Large',
      // IIS.
      440 => 'Login Time-out',
      // Nginx.
      444 => 'No Response',
      // IIS.
      449 => 'Retry With',
      // Microsoft.
      450 => 'Blocked by Windows Parental Controls',
      // IIS.
      451 => 'Redirect',
      // Nginx.
      494 => 'Request header too large',
      // Nginx.
      495 => 'SSL Certificate Error',
      // Nginx.
      496 => 'SSL Certificate Required',
      // Nginx.
      497 => 'HTTP Request Sent to HTTPS Port',
      // Esri.
      498 => 'Invalid Token',
      // Nginx / Esri.
      499 => 'Client Closed Request / Token Required',
      // Apache Web Server / cPanel.
      509 => 'Bandwidth Limit Exceeded',
      // Cloudflare.
      520 => 'Web Server Returned an Unknown Error',
      // Cloudflare.
      521 => 'Web Server Is Down',
      // Cloudflare.
      522 => 'Connection Timed Out',
      // Cloudflare.
      523 => 'Origin Is Unreachable',
      // Cloudflare.
      524 => 'A Timeout Occurred',
      // Cloudflare.
      525 => 'SSL Handshake Failed',
      // Cloudflare and Cloud Foundry's gorouter.
      526 => 'Invalid SSL Certificate',
      // Cloudflare.
      527 => 'Railgun Error',
      // Qualys in the SSLLabs.
      529 => 'Site is overloaded',
      // Pantheon web platform.
      530 => 'Site is frozen',
      // Informal convention.
      598 => 'Network read timeout error',
    ];
    if (!is_null($code)) {
      return $http_code_list[$code] ?? 'Unknown';
    }
    return $http_code_list;
  }

}
