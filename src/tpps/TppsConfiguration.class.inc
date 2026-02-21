<?php

/**
 * @file
 * Contains 'TppsConfiguration' class.
 *
 * Usage example:
 *
 * global $tpps;
 * dpm(
 *   // Instead of variable_get('tpps_watchdog_cleanup', FALSE).
 *   $tpps->watchdog_cleanup
 * );
 */

/**
 * Manages variables and constants of the TPPS module.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class TppsConfiguration {

  /**
   * Class constructor.
   */
  public function __construct() {
  }

  /**
   * Magic setter.
   *
   * @param string $name
   *   Class property name.
   * @param mixed $value
   *   Value to be set.
   */
  public function __set($name, $value) {
    global $conf;
    if (strtoupper($name) == $name) {
      // PHP Constant.
      return;
    }
    else {
      // Drupal Variable.
      if (!isset($conf[$name])) {
        $name = $this->map($name);
      }
      variable_set($name, $value);
      // Using $config probably allows to change value in memory without
      // DB update: $config[$name] = $value.
    }
  }

  /**
   * Magic getter.
   *
   * @param string $name
   *   Constant or variable name.
   *
   * @return mixed
   *   Returns constant/variable value.
   *   Returns NULL if constant/variable wan't found by name or alias.
   */
  public function __get($name) {
    global $conf;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Constant.
    if (strtoupper($name) == $name) {
      if (!defined($name)) {
        if (defined('TPPS_' . $name)) {
          $name = 'TPPS_' . $name;
        }
        else {
          $name = $this->map($name);
        }
      }
      if (defined($name)) {
        return constant($name);
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Drupal variables.
    else {
      if (!isset($conf[$name])) {
        if (isset($conf['tpps_' . $name])) {
          $name = 'tpps_' . $name;
        }
        else {
          $name = $this->map($name);
        }
      }
      if (isset($conf[$name])) {
        return $conf[$name];
      }
    }
    return NULL;
  }

  /**
   * Mapping of the variables and constants.
   *
   * @param string $name
   *   Constant or variable name.
   *
   * @return string
   *   Returns alias.
   */
  public function map($name) {
    // Format: alias -> real variable/constant.
    $map = [
      // Drupal Variables.
      'phenotype_metadata_validate_synonyms' =>
        'tpps_page_4_phenotype_metadata_validate_synonyms',
    ];
    if (!isset($map[$name])) {
      // Not found by name or synonym.
      $message = 'TppsConfig "@name" not found. Check spelling or create an alias.';
      $params = ['@name' => $name];
      watchdog('tpps', $message, $params, WATCHDOG_WARNING);
      return NULL;
    }
    return $map[$name];
  }

}
