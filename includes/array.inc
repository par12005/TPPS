<?php

/**
 * @file
 * Helper functions to manage arrays.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/array');
 */

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
function tpps_array_replace(array $array, $old, $new) {
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
function tpps_array_dump(array $array, $sticky = TRUE) {
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
function tpps_array_apply_changes(array $change_list, $array) {
  foreach ($change_list as $action => $action_data) {
    // Note: $action_data is a list of all actions (not single action).
    $function = 'tpps_array_' . $action;
    $params = ['@function_name' => $function, '@action' => $action];
    if (!function_exists($function)) {
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
    // Call an action-function.
    // 1. tpps_array_unset();
    // 2. tpps_array_move().
    // 3. tpps_array_create().
    // Note: $action_data also couldn't be empty here.
    call_user_func_array($function, [&$array, $action_data]);
  }
  return $array;
}

/**
 * Array modification 'unset'.
 *
 * Given list of fields will be removed/unset in given array.
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
function tpps_array_unset(array &$array, array $action_data) {
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
function tpps_array_move(array &$array, array $action_data) {
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

    // @TODO Minor. Use tpps_array_get_value().
    $target_field_value = $target['parent_element'][$target['field_name']] ?? NULL;
    $source_field_value = $source['parent_element'][$source['field_name']] ?? NULL;
    if (empty($target_field_value)) {
      // Target has no value - do the move.
      $target['parent_element'][$target['field_name']] = $source_field_value;
      unset($source['parent_element'][$source['field_name']]);
    }
    else {
      // Target has value. Switch to backup existing value.
      if (
        !empty($source_field_value)
        && ($target['field_name'] == 'doi' || $source['field_name'] == 'doi')
      ) {
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
          '@source_field_value' => var_export($source_field_value, 1),
          '@saved_values' => (
            $target['field_name'] == 'doi'
            ? var_export($array['saved_values'][1]['publication'] ?? NULL, 1)
            : ''
          ),
          '@direction' => (!empty($array['saved_values'][1]['doi']) ? 'upgrade' : 'downgrade'),
          '@form_version' => $array['saved_values'][SubmissionVersion::FORM_KEY],
        ];
        tpps_message($message, $params, 'debug');
        // Skip this because field already exists.
        // @TODO Minor. Check also value of the target field.
        continue;
      }
      if (variable_get('tpps_submission_version_switch_values', FALSE)) {
        // Switch values to store existing value for later.
        $temp = $target['parent_element'][$target['field_name']];
        $target['parent_element'][$target['field_name']] = $source_field_value;
        $source['parent_element'][$source['field_name']] = $temp;
      }
      else {
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
 * $ref = &tpps_array_get_value($array, $parents);
 *
 * To update value:
 * $ref = &tpps_array_get_value($array, $parents);
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
 *
 * @return mixed
 *   Returns the reference (!) to the value of the element of the array.
 *   Returns $default if element wasn't found in given array but not reference.
 *   Returns $array when $parents is an empty array.
 */
function &tpps_array_get_value(&$array = [], array $parents = []) {
  $key_exists = FALSE;
  if (!empty($array) && is_array($array)) {
    $value = &drupal_array_get_nested_value($array, $parents, $key_exists);
  }
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
function tpps_array_set_value(array &$array, array $parents, $value = NULL) {
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
 *
 * @return string
 *   Returns rendered HTML list.
 *
 *   @todo move to TppsArray class.
 */
function tpps_array_print_list(array $list = []) {
  $items = [];
  foreach ($list as $key => $value) {
    if (is_array($value)) {
      $value = tpps_array_print_list($value);
    }
    else {
      // Using check_plain() blocks HTML Links in the lists.
      // $value = check_plain($value);
    }
    $items[] = '<strong>' . check_plain($key) . ':</strong> ' . $value;
  }
  return theme('item_list', ['items' => $items]);
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
function tpps_array_get_path(array $array) {
  foreach ($array as $key => $value) {
    return array_merge(
      [$key],
      (is_array($value) ? tpps_array_get_path($value) : [])
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
function tpps_array_create(array &$array, array $action_data) {


// @TODO Add 'create' action to v2 for 'yes/no' new fields.


  // @TODO Some fields in v2 could be set into correct values.
  // Note: v3. New fields couldn't be created from v1 and v2.
  return TRUE;
}
