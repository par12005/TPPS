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
    return (bool) (count(array_diff($array1, $array2)) == 0);
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
    return static::isEqual($intersection, $array1);
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
   * Applys given list of changes to the array.
   *
   * See 'TppsArray' class.
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
   *   ];
   *   Note: no version numbers here.
   * @param mixed $result
   *   Array which must be changed.
   *
   * @TODO Minor. Return TRUE if all actions applied with success.
   */
  public static function applyChanges(array $change_list, &$result) {
    foreach ($change_list as $action => $action_data) {
      // Note: $action_data is a list of all actions (not single action).
      $function = 'TppsArray::' . $action;
      $params = ['@function_name' => $function, '@action' => $action];
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
      // Calls an action-functions: 'TppsArray::unset', 'TppsArray::move'.
      call_user_func_array($function, [&$result, $action_data]);
    }
  }

  /**
   * Array modificaiton 'unset'.
   *
   * Given list of fields will be removed/unset in given array.
   *
   * @param array $array
   *   Array to be modified.
   * @param array $action_data
   *   Data which required for perform an action.
   *   Each item is path to the single field.
   *   See tpps_submission_form_get_change_list().
   *
   * @return bool
   *   Returns TRUE if action was successfully applied.
   *   Returns FALSE otherwise.
   */
  public static function unset(array &$array, array $action_data) {
    module_load_include('inc', 'tpps', 'includes/form');
    foreach ($action_data as $key => $item) {
      if (empty($item)) {
        $message = 'Array modification "unset" #@key failed. One of the required '
          . 'fields is empty.<br />Value: <pre>@item</pre>.';
        $params = ['@key' => $key, '@item' => print_r($item, 1)];
        tpps_message($message, $params, 'debug');
        continue;
      }
      $target = &tpps_form_get_path($array, $item);
      if (empty($target['parent_element']) || empty($target['field_name'])) {
        $message = "Array modification 'unset' can't find element for #@key."
          . '<br />Value: <pre>@item</pre>. ';
        $params = ['@key' => $key, '@item' => print_r($item, 1)];
        tpps_message($message, $params, 'debug');
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
   *   Data which is required to perform this action.
   *   Each item contains information for a single field relocation and
   *   represented by list of parents (including field name) with 2 keys:
   *   'source' and 'target'.
   *   'source' is a real element path. 'target' is a new (but actually it's
   *   location in verion 1).
   *   See tpps_submission_form_get_change_list().
   *
   * @return bool
   *   Returns TRUE if action was successfully applied.
   *   Returns FALSE otherwise.
   */
  public static function move(array &$array, array $action_data) {
    module_load_include('inc', 'tpps', 'includes/form');
    foreach ($action_data as $key => $item) {
      if (empty($item['source']) || empty($item['target'])) {
        continue;
      }
      // Note:
      // 'parent_element' is not a list of parents but a reference to element.
      // To get value use 'field_name' as key inside 'parent_element'.
      $source = &tpps_form_get_path($array, $item['source']);
      $target = &tpps_form_get_path($array, $item['target']);
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

      $target_field_value = $target['parent_element'][$target['field_name']] ?? NULL;
      if (!empty($target_field_value)) {
        if ($target['field_name'] == 'doi' || $source['field_name'] == 'doi') {
          $message = 'Array modification "move": '
            . 'target field is not empty and will be overwritten.<br />'
            . 'Direction: @direction. <br />'
            . 'Form Version: @form_version. <br />'
            . "Target's field value: <pre>@field_value</pre>"
            . "Source's field value: <pre>@source_field_value</pre>"
            . "Action's meta-data: <pre>@meta</pre>"
            . "Saved Values: <pre>@saved_values</pre>";
          $params = [
            '@key' => $key,
            '@meta' => var_export($item, 1),
            '@field_value' => var_export($target_field_value, 1),
            '@source_field_value' => var_export(
              $source['parent_element'][$source['field_name']] ?? NULL, 1
            ),
            '@saved_values' => (
              $target['field_name'] == 'doi'
              ? var_export($array['saved_values'][1]['publication'] ?? NULL, 1)
              : ''
            ),
            '@direction' => (!empty($array['saved_values'][1]['doi']) ? 'upgrade' : 'downgrade'),
            '@form_version' => $array['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY],
          ];
          tpps_message($message, $params, 'debug');
        }
        // Skip this because field already exists.
        // @TODO Minor. Check also value of the target field.
        continue;
      }
      // Action!
      $target['parent_element'][$target['field_name']]
        = $source['parent_element'][$source['field_name']];
      unset($source['parent_element'][$source['field_name']]);
    }
    return TRUE;
  }

  /**
   * Gets value of the nested element from the array.
   *
   * To get value by reference use:
   * $ref = &TppsArray::getValue($array, $parents);
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
   * @param array $array
   *   Array to search.
   * @param array $parents
   *   List of the element's parents (including element name).
   *   When empty array then original $array will be returned.
   * @param mixed $key_exists
   *   If given, an already defined variable that is altered by reference.
   *
   * @return mixed
   *   Returns the reference (!) to the value of the element of the array.
   *   Returns $default if element wasn't found in given array but not reference.
   *   Returns $array when $parents is an empty array.
   */
  public static function &getValue(array &$array, array $parents = [], &$key_exists = NULL) {
    $value = &drupal_array_get_nested_value($array, $parents, $key_exists);
    if ($key_exists) {
      return $value;
    }
    else {
      $null = NULL;
      return $null;
    }
  }

  /**
   * Sets value of the nested element in the array.
   *
   * Shortcut for 'drupal_array_set_nested_value()'.
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
  public static function setValue(array &$array, array $parents, $value = NULL) {
    drupal_array_set_nested_value($array, $parents, $value);
  }

  /**
   * Prints array using theme_item_list().
   *
   * @param array $array
   *   Array to be printed.
   *
   * @return string
   *   Returns rendered HTML-list.
   */
  public static function printList(array $array) {
    $output = [];
    foreach ($array as $key => $value) {
      $output[check_plain($key)] = static::preparePrintListItem($key, $value);
    }
    return theme('item_list', ['items' => $output]);
  }

  /**
   * Helper to build items for 'print' method.
   *
   * Using separate method allows to use recursion.
   *
   * @param mixed $key
   *   Array item key.
   * @param mixed $value
   *   Array item value.
   *
   * @return array
   *   Returns array with items ready for theme_item_list().
   */
  public static function preparePrintListItem($key, $value) {
    if (is_array($value)) {
      $output = [];
      foreach ($value as $sub_key => $sub_value) {
        $sub_key = check_plain($sub_key);
        if (is_array($sub_value)) {
          $output[$sub_key] = static::buildItem($sub_key, $sub_value);
        }
        else {
          $output[$sub_key] = check_plain($sub_value);
        }
      }
      return ['data' => check_plain($key), 'children' => $output];
    }
    return check_plain($value);
  }

}
