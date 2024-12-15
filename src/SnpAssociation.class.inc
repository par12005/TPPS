<?php

/**
 * @file
 * Contains 'SnpAssociation' class.
 */

/**
 * Manages SNP Association file field.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SnpAssociation {

  // Data types for 'SNP Association' field columns.
  const DATA_TYPE_SNP_ID = 1;
  const DATA_TYPE_SCAFFOLD = 2;
  const DATA_TYPE_POSITION = 3;
  const DATA_TYPE_ALLELE = 4;
  const DATA_TYPE_ASSOCIATED_TRAIT = 5;
  const DATA_TYPE_CONFIDENCE_VALUE = 6;
  const DATA_TYPE_GENE_ID = 7;
  const DATA_TYPE_ANNOTATION = 8;
  const DATA_TYPE_YEAR = 9;

  /**
   * Gets list of column data types.
   *
   * @param int $org_num
   *   Ordinal number of organism.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   *
   * @return array
   *   Returns list of options with column data types.
   */
  public static function getColumnOptions() {
    return [
      self::DATA_TYPE_SNP_ID => 'SNP ID',
      self::DATA_TYPE_SCAFFOLD => 'Scaffold',
      self::DATA_TYPE_POSITION => 'Position',
      self::DATA_TYPE_ALLELE => 'Allele',
      self::DATA_TYPE_ASSOCIATED_TRAIT => 'Associated Trait',
      self::DATA_TYPE_CONFIDENCE_VALUE => 'Confidence Value',
      self::DATA_TYPE_GENE_ID => 'Gene ID',
      self::DATA_TYPE_ANNOTATION => 'Annotation',
      // Option 'year' must be shown only when Phenotype data file has 'year'
      // column. Since phenotype data file could be added/removed at the same
      // page beside 'SNP Association' file field we add this option at
      // server side and will add/remove it on client's side (in browser).
      self::DATA_TYPE_YEAR => 'Year',
    ];
  }

  /**
   * Builds form fields for Phenotype Data file.
   *
   * @param int $id
   *   Organism Id.
   * @param array $form
   *   Drupal Form API Array.
   * @param array $form_state
   *   Drupal Form API State Array.
   */
  public static function build($id, array &$form, array $form_state) {
    $page4_values = $form_state['saved_values']['4'];

  }

}
