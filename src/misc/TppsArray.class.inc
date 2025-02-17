<?php

/**
 * @file
 * Contains 'TppsArray' class.
 *
 * 'Array' and 'List' are a reserved keyword and couldn't be used.
 *
 * @todo Minor. Use namespace and rename class to 'Array' after migration
 * to D8+ because D7 doesn't support namespaced classes.
 */

/**
 * Manages PHP Arrays.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class TppsArray {

  /**
   * Compare 1st level of values of 2 arrays.
   *
   * WARNING: Can't be used for multidimentional arrays. Only simple lists!
   *
   * Note:
   * 1. Values could be in random order.
   * 2. Order of arrays doesn't matter.
   * 3. Duplicate items won't be removed.
   *
   * @param array $array1
   *   tppsArray #1 to compare.
   * @param array $array2
   *   tppsArray #2 to compare.
   *
   * @return bool
   *   Returns TRUE if both arrays are empty.
   *   Returns TRUE if $array2 has all the values of the $array1 and length
   *   of the arrays are the same.
   *   Returns FALSE otherwise.
   */
  public static function compareValues(array $array1, array $array2) {
    // @TODO ? What if type of items are different? String and integer.
    // @TODO Should duplicate items be removed?
    if (count($array1) != count($array2)) {
      return FALSE;
    }
    if (empty($array1) && empty($array2)) {
      // @TODO ? Is this correct behavior?
      return TRUE;
    }
    // Exclude keys from processing.
    $values1 = array_values($array1);
    $values2 = array_values($array2);
    sort($values1);
    sort($values2);
    return (bool) ($required_keys === $selected_id_list);
  }

  /**
   * Replaces $old with $new in array's values and keys.
   *
   * @param array $array
   *   Array to be updated.
   * @param mixed $old
   *   Old token.
   * @param mixed $new
   *   New token.
   *
   * @return array
   *   Returns updated array.
   */
  public static function replace(array $array, $old, $new) {
    // Source: tpps_change_tgdr_number();
    $state_json = json_encode($array);
    $state_json = str_replace($old, $new, $state_json);
    return json_decode($state_json, TRUE);
  }

}
