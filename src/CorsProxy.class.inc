<?php

/**
 * @file
 * Contains 'CorsProxy' class.
 */

/**
 * Manages CORS Proxy server.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class CorsProxy {

  /**
   * Gets URL of the CORS Proxy server.
   *
   * @return string
   *   Returns URL.
   *
   * @throws Exception
   *   When URL is empty.
   */
  public static function getUrl() {
    if (empty($url = variable_get('tpps_doi_lookup_cors_proxy_url'))) {
      throw new Exception('Empty CORS Proxy URL.');
    }
    return $url;
  }

  /**
   * Gets setting which controls if proxy server must be kept alive.
   *
   * @return bool
   *   Returns TRUE if proxy server must be alive and FALSE otherwise.
   */
  public static function keepAlive() {
    return (bool) variable_get('tpps_doi_lookup_cors_proxy_keep_alive');
  }

  /**
   * Checks if CORS Proxy server is up-and-running.
   *
   * @return bool
   *   Returns TRUE if server is up-and-running and FALSE otherwise.
   */
  public static function isUp() {
    $response = NULL;
    try {
      $response = (new RestApi([
        'url' => static::getUrl() . '/iscorsneeded/',
      ]))->getContent();
    }
    catch (Exception $e) {
      watchdog('tpps_cron', $e->getMessage());
    }
    return (bool) ($response == 'no');
  }

}
