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
   * Adds record about usage of the file.
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
    if (TppsFile::isEmptyUsage($file, $accession)) {
      file_usage_add($file, 'tpps', 'tpps_project', $key, $count);
    }
  }

  /**
   * Removes record about usage of the file.
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
   * Gets file usage.
   *
   * @param object $file
   *   Drupal's Managed File object.
   * @param string $accession
   *   Submission's accession. Format: 'TGDRxxxx'.
   *
   * @return int
   *   Returns count of the usage for given study.
   */
  public static function getUsage($file, $accession) {
    $key = str_replace('TGDR', '', $accession);
    $usage = file_usage_list($file, 'tpps', 'tpps_project', $key);
    return TppsArray::getValue($usage, ['tpps', 'tpps_project', $key]);
  }

  /**
   * Checks if file usage is empty.
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

  /**
   * Builds link to file usage API page.
   *
   * @param int $fid
   *   File Id.
   *   Note: no check if file exists.
   * @param string $title
   *   Link title.
   * @param array $options
   *   Link options. See l().
   *
   * @return string
   *   Returns HTML code for hyperlink to file usage API page.
   *   Url includes access token which allows to access this page.
   *   Path example: 'api/file/20735/usage?token=xxxxx'.
   */
  public static function buildApiUsageLink($fid, $title = 'Usage', array $options = []) {
    $title = t($title);
    return l($title, 'api/file/' . $fid . '/usage', array_merge(
      [
        'query' => ['token' => variable_get('tpps_api_token')],
        'attributes' => ['title' => $title],
      ], $options
    ));
  }

}
