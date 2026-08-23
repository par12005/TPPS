<?php

/**
 * @file
 * Class 'TppsArray' which manages arrays.
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
   * Checks if values of two arrays are equal.
   *
   * WARNING: Can't be used for multidimentional arrays. Only simple lists!
   *
   * Note:
   * 1. Values could be in random order.
   * 2. Order of arrays doesn't matter.
   * 3. Duplicate items won't be removed.
   * 4. Two empty arrays are equal.
   *
   * @param array $array1
   *   Array #1.
   * @param array $array2
   *   Array #2.
   *
   * @return bool
   *   Returns TRUE when arrays are equal and FALSE otherwise.
   */
  public static function isEqual(array $array1, array $array2) {
    return (
      empty(array_diff($array1, $array2))
      && empty(array_diff($array2, $array1))
    );
  }

  /**
   * Checks if the Array #2 includes all values of the Array #2.
   *
   * WARNING: Can't be used for multidimentional arrays. Only simple lists!
   *
   * @param array $array1
   *   Array #1.
   * @param array $array2
   *   Array #2.
   *
   * @return bool
   *   Returns TRUE when Array #2 includes all values of the Array #1.
   *   Returns FALSE otherwise.
   */
  public static function contains(array $array1, array $array2) {
    // Items which both arrays has.
    $intersection = array_intersect($array1, $array2);
    // If Array #1 and $intersections are the same then Array #2 includes
    // all values of the Array #1.
    return self::isEqual($intersection, $array1);
  }

  /**
   * Replaces $old with $new in array's values and keys.
   *
   * WARNING: $old and $new must have the same type (see gettype()).
   * Type of the $new will be set to the type of $old.
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
    // Note: if $old is integer but $new is string it cause an error
    // because JSON representation are different.
    // Both must have the same type.
    settype($new, gettype($old));
    // Using json_encode() for $old and $new allows to have slashes and
    // other special chars in them.
    $json = str_replace(
      json_encode($old),
      json_encode($new),
      json_encode($array)
    );
    return json_decode($json, TRUE);
  }

  /**
   * Parses huge array to show a well-formatted dump.
   *
   * Good for huge arrays. For small use dpm() or similar.
   *
   * @param array $array
   *   Array to be dumped.
   * @param bool $sticky
   *   Should table has sticky header or not.
   *
   * @return array
   *   Returns array with keys:
   *    'plain_items' and names of sub-array items.
   */
  public static function dump(array $array, $sticky = TRUE) {
    $output = [];
    // Get list of sub-arrays ($part list)
    foreach (array_keys($array) as $key) {
      if (is_array($array[$key])) {
        $part_list[$key] = count($array[$key]);
      }
      else {
        $plain[] = $key;
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Plain Items.;
    if (!empty($plain ?? NULL)) {
      ksort($plain);
      foreach ($plain as $line => $key) {
        $plain_rows[] = [
          [
            'data' => $line + 1,
            'style' => 'padding: 0 10px; font-weight: bold;',
          ],
          [
            'data' => check_plain($key),
            'style' => 'padding: 0 10px; font-weight: bold;',
          ],
          [
            'data' => check_plain(var_export($array[$key], 1)),
            'style' => 'padding: 0 10px; width:100%;',
          ],
        ];
      }
      $output['plain_items'] = [
        'title' => t('Plain Items'),
        'data' => theme('table', ['rows' => $plain_rows, 'sticky' => $sticky]),
        'count' => count($plain),
      ];
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Sub-arrays.
    if (!empty($part_list ?? NULL)) {
      ksort($part_list);
      foreach ($part_list as $key => $count) {
        $rows = [];
        $state_part = $array[$key] ?? [];
        $state_array = preg_split("/\r\n|\n|\r/", var_export($state_part, 1));
        if ($state_part) {
          foreach ($state_array as $line => $content) {
            $rows[] = [
              [
                'data' => $line + 1,
                'style' => 'padding: 0 10px; font-weight: bold;',
              ],
              [
                'data' => tpps_string_format_indent($content),
                'style' => 'padding: 0 10px; width:100%;',
              ],
            ];
          }
          $output[$key] = [
            'title' => $key,
            'data' => theme('table', ['rows' => $rows, 'sticky' => $sticky]),
            'count' => count($array[$key]),
          ];
        }
      }
    }
    return $output;
  }

  /**
   * Applies given list of changes to the array.
   *
   * Note: no version numbers here.
   *
   * @param array $change_list
   *   List of changes. Format is:
   *   <action> => [ <action's data>]
   *   Where <action> could be 'unset' or 'move'. Example:
   *   $change_list = [
   *     'unset' => [
   *       'build_info',
   *       'temporary',
   *       'programmed',
   *     ],
   *   ].
   * @param mixed $array
   *   Array which must be changed.
   *
   * @return array
   *   Return modified array.
   */
  public static function applyChanges(array $change_list, $array) {
    foreach ($change_list as $action => $action_data) {
      // Note: $action_data is a list of all actions (not single action).
      $method = 'TppsArray::' . $action;
      $params = ['@function_name' => $method, '@action' => $action];
      if (!method_exists('TppsArray', $action) && !function_exists($function)) {
        $message = "Function @function_name wan't found.";
        tpps_message($message, $params, 'error');
        continue;
      }
      if (empty($action_data)) {
        $message = 'No data for array action "@action".'
          . '<br />Function name: @function_name.';
        tpps_message($message, $params, 'error');
        continue;
      }
      // Call an action-methods.
      // 1. TppsArray::unset();
      // 2. TppsArray::move().
      // 3. TppsArray::create().
      // Note: $action_data also couldn't be empty here.
      call_user_func_array($method, [&$array, $action_data]);
    }
    return $array;
  }

  /**
   * Array modification 'unset'.
   *
   * Given list of fields will be removed/unset in given array.
   *
   * See also Submission::purifySharedState() where 'unset' action used to
   * reduce size of the SharedState array.
   *
   * @param array $array
   *   Array to be modified.
   * @param array $action_data
   *   Data required for this action.
   *   See tpps_submission_form_get_change_list().
   *   Each item is path to the single field.
   *
   * @return bool
   *   Returns TRUE if action was successfully applied.
   *   Returns FALSE otherwise.
   */
  public static function unset(array &$array, array $action_data) {
    foreach ($action_data as $key => $item) {
      $params = [
        '@key' => ($item['field_name'] ?? $key),
        '@item' => print_r($item, 1),
      ];
      if (!isset($item['parents'])) {
        $message = 'Array modification "unset" #@key failed. Missing "parents" '
          . 'element. <pre>@item</pre>.';
        tpps_message($message, $params, 'error');
        continue;
      }
      $target = &TppsForm::getPath($array, $item['parents']);
      if (empty($target['parent_element']) || empty($target['field_name'])) {
        if (variable_get('tpps_debug_mode', FALSE)) {
          // It's not an error because field would be not used in this submission.
          $message = "Array modification 'unset' can't find element for #@key."
            . '<br />Value: <pre>@item</pre>.';
          watchdog('tpps', $message, $params, WATCHDOG_DEBUG);
        }
        continue;
      }
      // Note:
      // $target['parent_element'] is a reference to the element in $array.
      // Action!
      unset($target['parent_element'][$target['field_name']]);
    }
    return TRUE;
  }

  /**
   * Array modificaiton 'move'.
   *
   * Given list of fields will be moved/relocate to the new location.
   *
   * @param array $array
   *   Array to be modified.
   * @param array $action_data
   *   Data required for this action.
   *   See tpps_submission_form_get_change_list().
   *   Each item contains information for a single field relocation and
   *   represented by list of parents (including field name) with 2 keys:
   *   'source' and 'target'.
   *   'source' is a real element path. 'target' is a new (but actually it's
   *   location in verion 1).
   *
   * @return bool
   *   Returns TRUE if action was successfully applied.
   *   Returns FALSE otherwise.
   */
  public static function move(array &$array, array $action_data) {
    if ($debug_mode ?? FALSE) {
      dpm(print_r(debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 5), TRUE));
    }
    foreach ($action_data as $key => $item) {
      if (empty($item['source']) || empty($item['target'])) {
        continue;
      }
      // Note:
      // 'parent_element' is not a list of parents but a reference to element.
      // To get value use 'field_name' as key inside 'parent_element'.
      $source = &TppsForm::getPath($array, $item['source']);
      $target = &TppsForm::getPath($array, $item['target']);
      if (
        empty($source['parent_element']) || empty($source['field_name'])
        || empty($target['parent_element']) || empty($target['field_name'])
      ) {
        $message = 'Array modification "move" #@key failed. One of the required '
          . 'fields is empty.<br />Value: <pre>@item</pre>.';
        $params = ['@key' => $key, '@item' => print_r($item, 1)];
        tpps_message($message, $params, 'debug');
        continue;
      }
      // Note: $source['parent_element'] and $target['parent_element']
      // are a references to the elements inside $array.
      if (!array_key_exists($source['field_name'], $source['parent_element'])) {
        $message = 'Array modification "move" #@key failed. Source field not found.'
          . '<br />Value: <pre>@item</pre>.';
        $params = ['@key' => $key, '@item' => print_r($item, 1)];
        tpps_message($message, $params, 'debug');
        continue;
      }

      // @TODO Minor. Use TppsArray::get().
      $source_field_value = $source['parent_element'][$source['field_name']] ?? NULL;
      $target_field_value = $target['parent_element'][$target['field_name']] ?? NULL;

      if ($debug_mode ?? FALSE) {
        drupal_set_message(TppsArray::print([
          'action' => __METHOD__,
          'source' => [
            '[ ' . implode(', ', $item['source']) . ' ]'
            => var_export($source_field_value, 1)
          ],
          'target' => [
            '[ ' . implode(', ', $item['target']) . ' ]'
            => var_export($target_field_value, 1)
          ],
        ]));
      }

      if (empty($target_field_value)) {
        // Target has no value - do the move.
        $target['parent_element'][$target['field_name']] = $source_field_value;
        unset($source['parent_element'][$source['field_name']]);
      }
      else {
        if (variable_get('tpps_submission_version_switch_values', FALSE)) {
          // Switch values to store existing value for later.
          $temp = $target['parent_element'][$target['field_name']];
          $target['parent_element'][$target['field_name']] = $source_field_value;
          $source['parent_element'][$source['field_name']] = $temp;
        }
        else {
          // Target field is not empty.
          // This should never happen so it's a strange case which we should log.
          // Switch to backup existing value.
          $message = 'Array modification "move": target field is not empty.<br />'
            . 'Direction: @direction. <br />'
            . 'Form Version: @form_version. <br />'
            . "Target's field value: <pre>@field_value</pre>"
            . "Source's field value: <pre>@source_field_value</pre>"
            . "Action's meta-data: <pre>@meta</pre>";
          $params = [
            '@key' => $key,
            '@meta' => var_export($item, 1),
            '@field_value' => var_export($target_field_value, 1),
            '@source_field_value' => var_export($source_field_value, 1),
            '@direction' => (!empty($array['saved_values'][1]['doi']) ? 'upgrade' : 'downgrade'),
            '@form_version' => $array['saved_values'][SubmissionVersion::FORM_KEY],
          ];
          watchdog('tpps', $message, $params, WATCHDOG_DEBUG);

          $target['parent_element'][$target['field_name']] = $source_field_value;
          unset($source['parent_element'][$source['field_name']]);
        }
      }
    }
    return TRUE;
  }

  /**
   * Gets value of the nested element from the array.
   *
   * To get value by reference use:
   * $ref = &TppsArray::get($array, $parents);
   *
   * To update value:
   * $ref = &TppsArray::get($array, $parents);
   * $ref = 'new value';
   *
   * Shortcut for function drupal_array_get_nested_value() with default value.
   * See
   * https://api.drupal.org/api/drupal/includes%21common.inc/function/drupal_array_get_nested_value/7.x#.
   *
   * Note:
   * This function is very common but gunction tpps_get_ajax_value() is more
   * specific. It does much more and works with 'values' and 'complete form'
   * elements of given arrays.
   *
   * @param mixed $array
   *   Array to search within.
   *   When empty then there will be no error but NULL will be returned.
   * @param array $parents
   *   List of the element's parents (including element name).
   *   When empty array then original $array will be returned.
   * @param mixed $key_exists
   *   If given, an already defined variable that is altered by reference.
   *
   * @return mixed
   *   Returns the reference (!) to the value of the element of the array.
   *   Returns $array when $parents is an empty array.
   */
  public static function &get(&$array = NULL, array $parents = [], &$key_exists = NULL) {
    if (is_null($array)) {
      // $array could be a sub-element of the array (e.g., $form_state['input']
      // and be empty (doesn't exists) so needed element also doesn't exist.
      $key_exists = FALSE;
      $null = NULL;
      return $null;
    }
    $value = &drupal_array_get_nested_value($array, $parents, $key_exists);
    if ($key_exists) {
      return $value;
    }
    else {
      $key_exists = FALSE;
      $null = NULL;
      return $null;
    }
  }

  /**
   * Sets value of the nested element in the array.
   *
   * Shortcut for function drupal_array_set_nested_value().
   * See
   * https://api.drupal.org/api/drupal/includes%21common.inc/function/drupal_array_set_nested_value/7.x#.
   *
   * @param array $array
   *   Array to search.
   * @param array $parents
   *   List of the element's parents (including element name).
   *   When $parents is an empty array then $array = $value.
   * @param mixed $value
   *   The value.
   */
  public static function set(array &$array, array $parents, $value = NULL) {
    drupal_array_set_nested_value($array, $parents, $value);
  }

  /**
   * Dumps array using item list.
   *
   * WARNING: Be sure to use check_plain() for data in the $list before call.
   * Note: uses recursion.
   *
   * @param array $list
   *   Array with data.
   * @param string $title
   *   Optional title for the list.
   *   Note: HTML's H3 tag wil be used.
   *
   * @return string
   *   Returns rendered HTML list.
   */
  public static function print(array $list = [], $title = NULL) {
    $items = [];
    if (empty($list)) {
      return '';
    }
    foreach ($list as $key => $value) {
      if (is_array($value)) {
        $value = static::print($value);
      }
      else {
        // Using check_plain() blocks HTML Links in the lists.
        // $value = check_plain($value);
      }
      $items[] = '<strong>' . check_plain($key) . ':</strong> ' . $value;
    }
    return theme('item_list', [
      'items' => $items,
      'title' => check_plain($title),
    ]);
  }

  /**
   * Dumps array using item list.
   *
   * WARNING: Be sure to use check_plain() for data in the $list before call.
   * Note: uses recursion.
   *
   * @param array $list
   *   Array with data.
   * @param string $title
   *   Optional title for the list.
   *   Note: HTML's H3 tag wil be used.
   *
   * @return string
   *   Returns rendered HTML list.
   */
  public static function printTable(array $list = [], $title = NULL) {
    $rows = [];
    if (empty($list)) {
      return '';
    }
    foreach ($list as $key => $value) {
      if (is_array($value)) {
        $value = TppsArray::printTable($value);
      }
      else {
        // Using check_plain() blocks HTML Links in the lists.
        // $value = check_plain($value);
      }
      $rows[] = [
        '<strong>' . check_plain($key) . '</strong>',
        $value,
      ];
    }
    return theme('table', [
      'rows' => $rows,
      'title' => check_plain($title),
      'empty' => t('No data'),
    ]);
  }

  /**
   * Gets path to the array element as a list of parents.
   *
   * $array = [
   *   'a' => [
   *     'b' => [
   *       'c' => 1,
   *      ],
   *    ],
   *  ];
   *
   *  Result: ['a', 'b', 'c'];
   *
   * @param array $array
   *   Array to process.
   *
   * @return array
   *   Returns parents of the element.
   */
  public static function getPath(array $array) {
    foreach ($array as $key => $value) {
      return array_merge(
        [$key],
        (is_array($value) ? TppsArray::getPath($value) : [])
      );
    }
  }

  /**
   * Submission Version action 'create'.
   *
   * Given list of fields created in a new version.
   *
   * @param array $array
   *   Array to be modified.
   * @param array $action_data
   *   Data required for this action.
   *   See tpps_submission_form_get_change_list().
   *   Each item is path to the single field.
   *
   * @return bool
   *   Returns TRUE if action was successfully applied.
   *   Returns FALSE otherwise.
   */
  public static function create(array &$array, array $action_data) {

    // Version 2. Was added several helper fields.
    $marker_type_field_list = [
      'does_study_include_snp_data' => 'SNPs',
      'does_study_include_ssr_cpssr_data' => 'SSRs/cpSSRs',
      'does_study_include_other_genotypic_data' => 'Other',
    ];
    foreach ($action_data as $item) {
      $key = $marker_type_field_list[$item['field_name']] ?? NULL;
      if (empty($key)) {
        continue;
      }
      // We have a hidden field 'Marker type' which was left for
      // backward compatibility and it's value is set using JS on client side
      // when user select those new fields.
      $marker_type_parents = $item['parents'];
      array_pop($marker_type_parents);
      $marker_type_parents[] = 'marker-type';
      $marker_type_values = TppsArray::get($array, $marker_type_parents);
      if (!empty($marker_type_values[$key])) {
        TppsArray::set($array, $item['parents'], 'yes');
      }
    }

    // Note: v3. Values of the new fields couldn't be created based on existings
    // fields values because they are totally new.
    return TRUE;
  }

  /**
   * Gets index of array element by it's key.
   *
   * Problem:
   * If an array is an indexed array (where keys are sequential integers
   * starting from 0), the "index" of an element is its key. If it's an
   * associative array (where keys are strings or non-sequential integers),
   * there isn't a direct "index" in the same sense as an indexed array.
   *
   * Supports mixed arrays.
   * See https://stackoverflow.com/a/54704110
   *
   * @param array $array
   *   Array to be searched.
   * @param string $key
   *   Key to be found.
   *
   * @return int
   *   Returns index of the element.
   */
  public static function getIndex(array $array, $key) {
    return array_search($key, array_keys($array), $key);
  }

  /**
   * Checks if given value is the very last element of the array.
   *
   * To get public static function getIndex(array $array, $key) {
   *
   * @param array $array
   *   Array to be searched.
   * @param mixed $value
   *   Element's value.
   *
   * @return bool
   *   Returns TRUE when give value is the last element of the array.
   *   Returns FALSE otherwise.
   */
  public static function isLast(array $array, $value) {
    // getIndex() allows to search also mixed array (not indexed).
    $index = static::getIndex($array, array_search($value, $array));
    return (($index + 1) == count($array));
  }

  /**
   * Gets first element of the given array.
   *
   * @param array $array
   *   Array which will be updated.
   *
   * @return mixed
   *   Returns value of the first element.
   */
  public static function getFirst(array &$array) {
    return reset($array);
  }

  /**
   * Removes first element of the given array.
   *
   * WARNING: given array will be updated but not returned back.
   *
   * Idea: https://stackoverflow.com/a/18577136.
   *
   * @param array $array
   *   Array which will be updated.
   *
   * @return mixed
   *   Returns value of the removed first element.
   */
  public static function removeFirst(array &$array) {
    // Make sure to reset the array's current index.
    reset($array);
    $key = key($array);
    $first_element = $array[$key];
    unset($array[$key]);
    return $first_element;
  }

  /**
   * Removes and return back the last element of the given array.
   *
   * WARNING: given array WILL BE updated by reference.
   *
   * @param array $array
   *   Array which will be updated.
   *
   * @return mixed
   *   Returns value of the removed last element.
   */
  public static function removeLast(array &$array) {
    $last_element = array_pop($array);
    return $last_element;
  }

  /**
   * Gets last element of the given array.
   *
   * Note: original array WON'T be modified.
   *
   * @param array $array
   *   Array which will be updated.
   *
   * @return mixed
   *   Returns value of the removed last element.
   */
  public static function getLast(array $array) {
    return end($array);
  }

  /**
   * Gets duplicates.
   *
   * WARNING: Original element is not included. Only duplicates.
   *
   * @param array $array
   *   Array which will be processed.
   *
   * @return array
   *   Returns list of duplicate elements.
   *   Returns empty array if there is no duplicates.
   */
  public static function getDuplicates(array $array) {
    // Compare values (and keys) of 2 arrays and return the difference.
    return array_diff_assoc(
      $array,
      // Remove duplicate values.
      array_unique($array)
    );
  }

  /**
   * Finds the differences in values between two multidimensional arrays.
   *
   * To compare for exact equality (structure and values) use the strict
   *   comparison operator '==='.
   * To compare specific columns/values:
   *   1. Extract those parts using array_column(),
   *   2. Use functions like array_diff(), array_intersect(), or custom loops.
   *
   * @param array $array1
   *   First array to compare.
   * @param array $array2
   *   Second array to compare.
   *
   * @return array
   *   Returns element from 1st array which are missing or differs in 2nd array.
   */
  public static function compare(array $array1, array $array2) {
    $difference = [];
    foreach ($array1 as $key => $value) {
      if (array_key_exists($key, $array2)) {
        if (is_array($value) && is_array($array2[$key])) {
          $recursiveDiff = TppsArray::compare($value, $array2[$key]);
          if (count($recursiveDiff)) {
            $difference[$key] = $recursiveDiff;
          }
        }
        elseif ($value != $array2[$key]) {
          $difference[$key] = $value;
        }
      }
      else {
        $difference[$key] = $value;
      }
    }
    return $difference;
  }

  /**
   * Checks if array has NO elements with empty values.
   *
   * @param array $data
   *   One dimention array to check.
   *
   * @return bool
   *   Returns TRUE if each element has non-empty value and FALSE otherwise.
   */
  public static function isComplete(array $data) {
    return (empty($data) || count($data) === count(array_filter($data)));
  }

  /**
   * Gets key using insensitive search for the value in array.
   *
   * See
   * https://www.php.net/manual/en/function.array-search.php.
   *
   * @param string $needle
   *   String to be searched.
   * @param array $haystack
   *   Array to be searched in.
   *
   * @return mixed
   *   Returns key if value was found and NULL otherwise.
   */
  public static function searchInsensitive(string $needle, array $haystack) {
    return array_search(strtolower($needle), array_map('strtolower', $haystack));
  }

  /**
   * Insensitive check if value exists in array.
   *
   * @param string $needle
   *   String to be searched.
   * @param array $haystack
   *   Array to be searched in.
   *
   * @return bool
   *   Returns TRUE if value was found and FALSE otherwise.
   */
  public static function isExistsInsensitive(string $needle, array $haystack): bool {
    return !empty(static::searchInsensitive($needle, $haystack));
  }

}
