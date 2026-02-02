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

  /**
   * Adds record about usage of the file for certain study.
   *
   * @param object $file
   *   Drupal's Managed File object.
   * @param string $accession
   *   Submission's accession. Format: 'TGDRxxxx'.
   * @param int $count
   *   The number of references to add to the object. Defaults to 1.
   */
  public static function addUsage($file, $accession, $count = 1) {
    $key = str_replace('TGDR', '', $accession);
    file_usage_add($file, 'tpps', 'tpps_project', $key, $count);
  }

  /**
   * Removes record about usage of the file for certain study.
   *
   * @param object $file
   *   Drupal's Managed File object.
   * @param string $accession
   *   Submission's accession. Format: 'TGDRxxxx'.
   * @param int $count
   *   The number of references to add to the object. Defaults to 1.
   */
  public static function deleteUsage($file, $accession, $count = 1) {
    $key = str_replace('TGDR', '', $accession);
    file_usage_delete($file, 'tpps', 'tpps_project', $key, $count);
  }

  /**
   * Gets file usage for certain study.
   *
   * @param object $file
   *   Drupal's Managed File object.
   * @param string $accession
   *   Submission's accession. Format: 'TGDRxxxx'.
   *   Optional. Default is NULL.
   *
   * @return int
   *   Returns count of the usage for given study.
   *   When $accession is not empty then returns file usage for certain study.
   *   When $accession is empty returns usage for all studies returned by the
   *   file_usage_list() function. Format example (key is accession number):
   *   [
   *     1200 => 1,
   *     1245 => 1,
   *   ];
   */
  public static function getUsage($file, $accession = NULL) {
    $usage = file_usage_list($file);
    if (is_null($accession)) {
      return TppsArray::get($usage, ['tpps', 'tpps_project']);
    }
    else {
      $key = str_replace('TGDR', '', $accession);
      return TppsArray::get($usage, ['tpps', 'tpps_project', $key]);
    }
  }

  /**
   * Checks if file usage is empty for certain study.
   *
   * @param object $file
   *   Drupal's Managed File object.
   * @param string $accession
   *   Submission's accession. Format: 'TGDRxxxx'.
   *
   * @return bool
   *   Returns TRUE if file has no usage record and FALSE otherwise.
   */
  public static function isEmptyUsage($file, $accession) {
    return empty(TppsFile::getUsage($file, $accession));
  }

}
