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
 * @todo Minor.
 * Implement method to theme links to API usage and info pages (with token).
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
   * More safe way to load managed file object.
   *
   * Processing correctly cases (unlike file_load()):
   * 1. $fid is NULL.
   * 2. File size is zero.
   *
   * Usage example:
   *   if (
   *     // Note: Use parentheses to avoid notices or 'and' instead of the '&&'.
   *     ($file = TppsFile::load($fid))
   *     && $file->status != $status
   *   ) { ... }
   * No need to use static cache because file_load() uses it already.
   *
   * @param int $fid
   *   File Id.
   *
   * @return mixed
   *   Returns FALSE if:
   *   1. $fid is empty or NULL.
   *   2. $fid is not numiric.
   *   3. No record in 'file_managed' table.
   *   4. File has zero size.
   *   Returns 'Managed File' object otherwise.
   */
  public static function load($fid = NULL) {
    if (
      !empty($fid)
      && is_numeric($fid)
      && ($file = file_load($fid))
      && ($file->filesize ?? FALSE)
    ) {
      return $file;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Removes managed by Drupal file.
   *
   * @param int $fid
   *   Managed File Id.
   *
   * @return bool
   *   Returns TRUE for success,
   *   FALSE in the event of an error,
   *   or an array if the file is being used by any modules.
   */
  public static function remove($fid) {
    if ($file = TppsFile::load($fid)) {
      $result = file_delete($file, $force = TRUE);
      // tpps_file_removed().
      if (function_exists('module_invoke_all')) {
        module_invoke_all('file_removed', $file);
      }
      return $result;
    }
    return FALSE;
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

  /**
   * Checks if Drupal managed file exist and could be removed.
   *
   * Note: doesn't check if file exists in filesystem but checks
   * if other studies are using this file.
   *
   * @param object $file
   *   Drupal's Managed File object.
   * @param string $accession
   *   Submission's accession. Format: 'TGDRxxxx'.
   *
   * @return bool
   *   Returns TRUE if file could be safely removed and FALSE otherwise.
   */
  public static function isFileRemoveable($file, $accession) {
    if (empty($file) || empty($accession)) {
      // File doesn't exist.
      return FALSE;
    }
    else {
      $usage = TppsFile::getUsage($file);
      $key = str_replace('TGDR', '', $accession);
      if (
        // More then 2 studies are using this file.
        count($usage) > 1
        // Current study uses more the once. It's not normal but could happen.
        || (!empty($usage[$key]) && $usage[$key] > 1)
        // Only one study uses this file but not current study.
        || (count($usage) == 1 && empty($usage[$key]))
      ) {
        return FALSE;
      }
      else {
        return TRUE;
      }
    }
  }

  /**
   * Downloads file using URL and creates Drupal managed file.
   *
   * @param string $url
   *   URL to the file.
   * @param string $destination
   *   Stream Wrapper URI (not path to filesystem).
   *   For example: 'public://<folder_name>' or 'private://<folder_name>'.
   *   If no value is provided, a randomized name will be generated and the
   *   file will be saved using Drupal's default files scheme, usually "public://".
   *   See
   *   https://api.drupal.org/api/drupal/includes%21file.inc/function/file_save_data/7.x.
   *
   * @return object
   *   Returns 'Drupal Managed File' object.
   */
  public static function download($url, $destination = NULL) {
    $file_data = file_get_contents($url);
    if (strpos($file_data, 'Not Found') === FALSE) {
      // Prepare folder.
      $folder = drupal_dirname($destination);
      if (!file_prepare_directory($folder, FILE_CREATE_DIRECTORY)) {
        $message = t('Could not create directory: @path',
          ['@folder' => $folder]);
        drupal_set_message($message, 'error');
      }
      return file_save_data($file_data, $destination, FILE_EXISTS_REPLACE);
    }
    else {
      // Note: $file_data in this case is JSON response.
      throw new Exception($url . ': [404] Not found');
    }
  }

  /**
   * Builds URL to the file.
   *
   * @param int $file_id
   *   Drupal managed file id.
   *
   * @return string
   *   Returns URL.
   */
  public static function getUrl($file_id) {
    if (!empty($file = TppsFile::load($file_id))) {
      return file_create_url($file->uri);
    }
  }

}
