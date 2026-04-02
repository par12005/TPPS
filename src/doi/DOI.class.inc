<?php

/**
 * @file
 * Contains static 'DOI' class.
 */

/**
 * Manages DOI.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class DOI {

  // Single DOI Regexp which could be used in preg_match() as $pattern.
  const REGEX = '/^10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+$/';
  // Multiple DOIs Regex to find all matches.
  const SEARCH_REGEX = '/10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+/';

  /**
   * Prints DOI as hyperlink.
   *
   * @param string $doi
   *   Optional. DOI.
   *
   * @return string
   *   Returns rendered HTML tag <A> where anchor is a DOI and href is a link to
   *   https://doi.org/ site which will redirect to the publication.
   *   If $doi is empty returns empty string.
   */
  public static function print($doi = NULL) {
    if (!empty($doi)) {
      $output = theme('doi_link', ['doi' => $doi]);
    }
    return $output ?? '';
  }

  /**
   * Gets first DOI in given string.
   *
   * @param string $string
   *   String where DOI will be searched.
   *
   * @return string
   *   Returns first DOI found in string.
   */
  public static function searchFirst($string) {
    preg_match(static::SEARCH_REGEX, $string, $matches);
    return $matches[0] ?? NULL;
  }

  /**
   * Searches DOI in given string.
   *
   * @param string $string
   *   String where DOI will be searched.
   *
   * @return string
   *   Returns first found
   */
  public static function search($string) {
    preg_match_all(static::SEARCH_REGEX, $string, $matches);
    return $matches[0] ?? NULL;
  }

  /**
   * Checks if given string is a valid DOI.
   *
   * WARNING:
   * Use trim() before call this method.
   * Leading/trailing spaces are not allowed.
   *
   * @param string $string
   *   String where DOI will be searched.
   *
   * @return bool
   *   Returns TRUE if given string is a valid DOI and FALSE otherwise.
   */
  public static function isValid($string) {
    return (bool) preg_match(static::REGEX, $string);
  }

}
