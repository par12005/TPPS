<?php

/**
 * @file
 * Class 'CVTerm' which manages CVTerms.
 */

/**
 * Manages CVTerms.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class CVTerm {

  /**
   * Gets CVTerm Id for given name.
   *
   * Note: special care for CVTerm 'year' which doesn't exist on dev-site.
   *
   * @param string $name
   *   CVTerm name.
   *
   * @return int
   *   Returns CV Term Id.
   */
  public static function getId($name) {
    try {
      $id = tpps_load_cvterm($name)->cvterm_id;
    }
    catch (Exception $e) {
      if ($name == 'year') {
        // Note: cvterm 54188 (year) doesn't work at dev-server.
        // CVTerm must exist even if phenotype data file has no 'year' column.
        $id = variable_get('tpps_submitall_cvterm_phenotype_year', '');
      }
    }
    return $id ?? NULL;
  }

}
