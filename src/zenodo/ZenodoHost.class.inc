<?php

/**
 * @file
 * Contains 'ZenodoHost' class.
 */

/**
 * Manages Zenodo Hosts (servers).
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class ZenodoHost {

  // Host name.
  // See: https://developers.zenodo.org/?shell#testing.
  const LIVE_HOST = 'zenodo.org';
  const SANDBOX_HOST = 'sandbox.zenodo.org';


  /**
   * Sets Zenodo host name and statically cache it.
   *
   * Allows to have different host even during single per page request.
   * This method is needed because we use static methods instead of instance
   * of class with own properties.
   *
   * @param string $host
   *   Posible values: 'live' and any non-null value.
   *   When value is 'live' then static variable 'tpps_zenodo_host'
   *   will have name of the Zenodo Live server.
   *   Any other value will populate it with name of Zenodo Sandbox server.
   */
  public static function set($host = NULL) {
    if (empty($host)) {
      return;
    }
    $zenodo_host = &drupal_static('tpps_zenodo_host');
    $zenodo_host = (strtolower($host) == 'live')
      ? static::LIVE_HOST : static::SANDBOX_HOST;
  }

  /**
   * Gets Zenodo host name.
   *
   * Allows to have different host name even during single per page request
   * because uses statically cached value which could be changed during even
   * the same page request multiple times (for example to show depostions on
   * both Live and Sandbox server at the same page).
   * To use Live host: ZenodoHost::set('live');
   * To use Sandbox host: ZenodoHost::set('sandbox');
   *
   * If host name wasn't set then global site settings will be used.
   * To reset or force usage of the global settings:
   * ZenodoHost::get($reset = TRUE);
   *
   * @return string
   *   Returns Zenodo host name.
   */
  public static function get($reset = FALSE) {
    $zenodo_host = &drupal_static('tpps_zenodo_host');
    if ($reset || empty($zenodo_host)) {
      $zenodo_host = (variable_get('tpps_zenodo_debug_mode', FALSE))
        ? static::SANDBOX_HOST : static::LIVE_HOST;
    }
    return $zenodo_host;
  }

  /**
   * Toggles Zenodo Host.
   *
   * @return string
   *   Returns new host.
   */
  public static function toggle() {
    $zenodo_host = &drupal_static('tpps_zenodo_host');
    $zenodo_host = ($zenodo_host == static::LIVE_HOST)
      ? static::SANDBOX_HOST : static::LIVE_HOST;

    // Hook will get value of the new host.
    module_invoke_all(static::getHookName('host_toggled'), $zenodo_host);
    return $zenodo_host;
  }

  /**
   * Generates message with Zenodo Host status.
   *
   * By default global host will be shown but if current host and global host
   * are different then both will be shown.
   *
   * @return string
   *   Returns HTML-string with Zenodo Hosts and hyperlink to change it.
   *   When global (default) host and current (page specific) differs then
   *   both will be shown.
   */
  public static function getStatus() {
    $global_host = variable_get('tpps_zenodo_debug_mode') ? 'sandbox' : 'live';
    $current_host = static::get() == self::LIVE_HOST ? 'live' : 'sandbox';
    $message = 'Zenodo Host: <strong>@global_host</strong> [!change_link]';
    if ($global_host != $current_host) {
      // Current and Global Hosts are different and both must be shown.
      $message = 'Zenodo Host (global / current): '
      . '<strong>@global_host</strong> [!change_link] '
      . '/ <strong>@current_host</strong> (page specific).';
    }
    return t($message, [
      '@current_host' => $current_host,
      '@global_host' => strtoupper($global_host),
      '!change_link' => l(t('Change'), 'admin/config/tpps/zenodo'),
    ]);
  }

  /**
   * Prints out Zenodo hosts.
   *
   * By default global host will be shown but if current host and global host
   * are different then both will be shown.
   *
   * Note:
   * 1. Uses drupal_set_message().
   * 2. There is no sense to use watchdog().
   */
  public static function printStatus() {
    $severity = (static::get() == self::LIVE_HOST) ? 'warning' : 'status';
    drupal_set_message(static::getStatus(), $severity);
  }

  /**
   * Checks if current (page specific) Zenodo Host is a sandbox-server.
   *
   * @return bool
   *   Returns TRUE when Zenodo Sandbox Host used on current page.
   */
  public static function isSandbox() {
    return (static::get() == static::SANDBOX_HOST);
  }

  /**
   * Checks if current (page specific) Zenodo Host is a live-server.
   *
   * @return bool
   *   Returns TRUE when Zenodo Live Host used on current page.
   */
  public static function isLive() {
    return (static::get() == static::LIVE_HOST);
  }

  /**
   * Checks if global (site wide) Zenodo Host is a sandbox-server.
   *
   * @return bool
   *   Returns TRUE when Zenodo Sandbox Host used on current page.
   */
  public static function isGlobalSandbox() {
    return (bool) variable_get('tpps_zenodo_debug_mode');
  }

  /**
   * Checks if global (site wide) Zenodo Host is a live-server.
   *
   * @return bool
   *   Returns TRUE when Zenodo Live Host used on current page.
   */
  public static function isGlobalLive() {
    return !static::isGlobalSandbox();
  }

  /**
   * Generate Drupal hook name based on class name.
   *
   * See Zenodo::getHookName().
   *
   * @param string $action
   *   Method name.
   *
   * @return string
   *   Returns name of the hook (without module name prefix).
   */
  public static function getHookName($action) {
    return 'zenodo_host_' . $action;
  }

  /**
   * Validate if Deposit Id belings to current Zenodo Host.
   *
   * There is a problem when 'ZenodoFile' class gets files by Deposit Id
   * created on other Zenodo Host by other account and having this files at
   * out pages is confusing.
   *
   * Note: Zenodo Files can't validate host because Zenodo allows to get
   * files data without any access token (files data are public). But only
   * Deposition data is accessible by owner.
   *
   * Note: This method throws an exception.
   *
   * @param array $data
   *   Data for ZenodoDeposit::retrieve() request.
   *   Required key is 'deposit_id'.
   */
  public static function isValid(array $data) {
    Zenodo::validateRequired(['deposit_id'], $data);
    try {
      $response = ZenodoDeposit::retrieve($data);
    }
    catch (Exception $e) {
      // In case of any errors assume that host is invalid.
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Gets Class Name.
   *
   * Since 5.5 you can use class keyword for the class name resolution,
   * which would be a lot faster than making function calls.
   * Also works with interfaces.
   *
   * C extends B extends A
   *
   * static::class  // MyNamespace\ClassC when run in A
   * self::class    // MyNamespace\ClassA when run in A
   * parent::class  // MyNamespace\ClassB when run in C
   * MyClass::class // MyNamespace\MyClass
   *
   * See https://stackoverflow.com/questions/506705
   *
   * @return string
   *   Returns class name when called from static method.
   */
  public static function getClassName() {
    return static::class;
  }
}
