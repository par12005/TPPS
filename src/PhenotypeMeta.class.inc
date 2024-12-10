<?php

/**
 * @file
 * Contains 'PhenotypeMeta' class.
 *
 * The main reason to have PhenotypeMeta class is that each organism could have
 * multiple number of phenotypes so form fields must be processed in a
 * different way.
 */

/**
 * Manages fields to manually add phenotypes on TPPS Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class PhenotypeMeta {

  // Keys for Phenotype Metadata file columns (data types) options.
  // See PhenotypeMeta::getColumnOptions().
  const DATA_TYPE_IDENTIFIER = 1;
  const DATA_TYPE_ATTRIBUTE = 2;
  const DATA_TYPE_DESCRIPTION = 3;
  const DATA_TYPE_UNIT = 4;
  const DATA_TYPE_STRUCTURE = 5;
  const DATA_TYPE_MINIMUM_VALUE = 6;
  const DATA_TYPE_MAXIMUM_VALUE = 7;
  const DATA_TYPE_IS_ENVIRONMENTAL = 8;

  /**
   * Builds list of options for phenotype file data type selector (columns).
   *
   * @param int $key
   *   Key of the item in the list. When non-empty name of the option could be
   *   obtained.
   *
   * @return mixed
   *   Returns list of options when $key is empty.
   *   Returns name of the option when $key is not empty and exists in the list
   *   of options.
   */
  public function getColumnOptions($key = NULL) {
    $list = [
      0 => t('N/A'),
      self::DATA_TYPE_IDENTIFIER       => t('Phenotype Name/Identifier'),
      self::DATA_TYPE_ATTRIBUTE        => t('Attribute'),
      self::DATA_TYPE_DESCRIPTION      => t('Description'),
      self::DATA_TYPE_UNIT             => t('Unit'),
      self::DATA_TYPE_STRUCTURE        => t('Structure'),
      self::DATA_TYPE_MINIMUM_VALUE    => t('Minimum Value'),
      self::DATA_TYPE_MAXIMUM_VALUE    => t('Maximum Value'),
      self::DATA_TYPE_IS_ENVIRONMENTAL => t('Environmental Phenotype'),
    ];
    return (!is_null($key) && isset($list[$key])) ? $list[$key] : $list;
  }

  /**
   * Gets list of required columns of phenotype metadata file.
   *
   * This list used to validate file. See tpps_validate_phenotype().
   *
   * @return array
   *   Returns list of required columns in format:
   *   <Group Name> => [<key> => [<data type key in the columns list>]].
   */
  public function getRequiredColumnList() {
    return [
      'Phenotype Id' => ['id' => [self::DATA_TYPE_IDENTIFIER]],
      'Attribute' => ['attr' => [self::DATA_TYPE_ATTRIBUTE]],
      'Description' => ['desc' => [self::DATA_TYPE_DESCRIPTION]],
      'Unit' => ['unit' => [self::DATA_TYPE_UNIT]],
      'Structure' => ['structure' => [self::DATA_TYPE_STRUCTURE]],
    ];
  }

}
