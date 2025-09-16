<?php

/**
 * @file
 * Contains 'TppsFile' class.
 */

/**
 * Manages Drupal's managed files.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 *
 * @todo Convert file /includes/file_utils.inc into methods.
 */
class TppsFile {

  /**
   * Gets number of columns in the file.
   *
   * @param int $fid
   *   Drupal managed file Id.
   *
   * @return int
   *   Returns number of columns (width) of the file.
   */
  public static function countColumns($fid) {
    return tpps_file_width($fid);
  }

  /**
   * Gets length of the file.
   *
   * @param int $fid
   *   Drupal managed file Id.
   *
   * @return int
   *   Returns length of the file.
   */
  public static function countRows($fid) {
    return tpps_file_len($fid);
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
