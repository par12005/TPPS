<?php

/**
 * @file
 * Defines useful functions to be used with the TPPS form.
 */

/**
 * Creates a dynamic list fieldset complete with add/remove buttons and ajax.
 *
 * This function accepts a form object, form state object, an id, and a field
 * to repeat, and creates a fieldset with buttons to add/remove items from the
 * list. This function also supports a variety of additional options, which can
 * be specified in the $options array. The list will be inserted at $form[$id],
 * unless parents are specified in the options, in which case the list will be
 * inserted at $form[...$parents][$id].
 *
 * @param array $form
 *   The form array that the new list will be generated for.
 * @param array $form_state
 *   The form state array of the provided form.
 * @param string $id
 *   The id of the dynamic list.
 * @param array $repeat
 *   The field to repeat.
 * @param array $options
 *   Additional options.
 */
function tpps_dynamic_list(array &$form, array &$form_state, $id, array $repeat, array $options = array()) {
  $label = $options['label'] ?? $id;
  $parents = $options['parents'] ?? array();
  $number_parents = $parents;
  array_push($number_parents, $id, 'number');
  $up = $options['up'] ?? "Add $label";
  $down = $options['down'] ?? "Remove $label";
  $name_suffix = $options['name_suffix'] ?? "";
  $default = $options['default'] ?? 0;
  $minimum = $options['minimum'] ?? $default;
  $fieldset_title = $options['title'] ?? "$label information:";
  $button_callback = $options['callback'] ?? "tpps_{$id}_callback";
  $list_wrapper = $options['wrapper'] ?? "$id-wrapper";
  $sub_keys = $options['substitute_keys'] ?? array();
  $sub_fields = $options['substitute_fields'] ?? array();
  $replace_pattern = $options['replacement_pattern'] ?? '/!num/';
  
  $number = tpps_get_ajax_number($form_state, $number_parents, $up . $name_suffix, $down . $name_suffix, $default, $minimum);

  $element = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => "<div class=\"fieldset-title\">$fieldset_title</div>",
    '#collapsible' => $options['collapsible'] ?? TRUE,
    '#prefix' => "<div id=\"$list_wrapper\">",
    '#suffix' => '</div>',
    'add' => array(
      '#type' => 'button',
      '#button_type' => 'button',
      '#value' => $up,
      '#name' => $up . $name_suffix,
      '#ajax' => array(
        'wrapper' => $list_wrapper,
        'callback' => $button_callback,
      ),
    ),
    'remove' => array(
      '#type' => 'button',
      '#button_type' => 'button',
      '#value' => $down,
      '#name' => $down . $name_suffix,
      '#ajax' => array(
        'wrapper' => $list_wrapper,
        'callback' => $button_callback,
      ),
    ),
    'number' => array(
      '#type' => 'hidden',
      '#value' => $number,
    ),
  );

  for ($i = 1; $i <= $number; $i++) {
    $instance = $repeat;
    foreach ($sub_fields as $field) {
      if (!is_array($field)) {
        $instance[$field] = preg_replace($replace_pattern, $i, $repeat[$field]);
      }
      else {
        $new_value = preg_replace($replace_pattern, $i, drupal_array_get_nested_value($repeat, $field));
        drupal_array_set_nested_value($instance, $field, $new_value);
      }
    }
    foreach ($sub_keys as $key) {
      $value = drupal_array_get_nested_value($instance, $key);
      drupal_array_set_nested_value($instance, $key, NULL);
      $new_key = array();
      foreach ($key as $key_item) {
        $new_key[] = preg_replace($replace_pattern, $i, $key_item);
      }
      drupal_array_set_nested_value($instance, $new_key, $value);
    }
    $element[$i] = $instance;
  }

  $element_parents = $parents;
  array_push($element_parents, $id);
  drupal_array_set_nested_value($form, $element_parents, $element);
}

/**
 * Retrieves the value of the number of a tpps_dynamic_list fieldset.
 *
 * This function first checks the values of the form state, then the
 * saved_values, and finally returns the default if it cannot find the nubmer.
 *
 * @param array $state
 *   The state of the form.
 * @param array $parents
 *   The parents leading to the list number.
 * @param string $up
 *   The #name of the button to increase the list number.
 * @param string $down
 *   The #name of the button to decrease the list number.
 * @param int $default
 *   The default number if the number does not exist.
 * @param int $minimum
 *   The minimum number we are allowed to reach.
 *
 * @return int
 *   The ajax number of the dynamic list, or the default number.
 */
function tpps_get_ajax_number(array &$state, array $parents, $up, $down, $default = 0, $minimum = 0) {
  // Get page number.
  $page = $state['stage'];

  // Check form_state values, update if necessary.
  $value_parents = $parents;
  // If the triggering element is an upload/remove file button, then the
  // form_state['values'] array will not be loaded correctly, so we will need
  // to use the 'complete form' array instead.
  if (preg_match('/^.*_(upload|remove)_button$/', $state['triggering_element']['#name'])) {
    array_unshift($value_parents, 'complete form');
    $val = drupal_array_get_nested_value($state, $value_parents);
    if (isset($val['#type']) and $val['#type'] == 'hidden') {
      array_push($value_parents, '#value');
    }
  }
  else {
    array_unshift($value_parents, 'values');
  }
  $val = drupal_array_get_nested_value($state, $value_parents);
  if (isset($val) and $state['triggering_element']['#name'] == $up) {
    drupal_array_set_nested_value($state, $value_parents, $val + 1);
  }
  elseif (isset($val) and $state['triggering_element']['#name'] == $down and $val > $minimum) {
    drupal_array_set_nested_value($state, $value_parents, $val - 1);
  }
  $val = drupal_array_get_nested_value($state, $value_parents);
  if (isset($val)) {
    return $val;
  }

  // Check saved values.
  $saved_value_parents = $parents;
  array_unshift($saved_value_parents, 'saved_values', $page);
  $saved_val = drupal_array_get_nested_value($state, $saved_value_parents);
  if (isset($saved_val)) {
    return $saved_val;
  }

  // Found nothing, return default.
  return $default;
}

/**
 * Retrieves the value of a field that may have been altered through ajax.
 *
 * This function first checks the 'complete form' of the form state, then the
 * saved_values, and finally returns the default if it cannot find the value.
 * If the value we are trying to reach is part of a TPPS managed_file, then we
 * can also pass the file name in order to find saved_values such as
 * 'file-columns', or 'file-no-header'.
 *
 * @param array $state
 *   The state of the form.
 * @param array $parents
 *   The parents leading to the list number.
 * @param mixed $default
 *   The default value to return.
 * @param string $file_name
 *   Optional - the name of the file.
 *
 * @return mixed
 *   The value of the specified field, or the default value.
 */
function tpps_get_ajax_value(array &$state, array $parents, $default = NULL, $file_name = "") {
  $page = $state['stage'];
  $value_parents = $parents;
  array_unshift($value_parents, 'values');
  $element = drupal_array_get_nested_value($state, $value_parents);
  if (isset($element['#type']) and $element['#type'] != 'fieldset') {
    array_push($value_parents, '#value');
  }
  $val = drupal_array_get_nested_value($state, $value_parents);
  if (isset($val)) {
    return $val;
  }

  $complete_parents = $parents;
  array_unshift($complete_parents, 'complete form');
  $element = drupal_array_get_nested_value($state, $complete_parents);
  if (isset($element['#type']) and $element['#type'] != 'fieldset') {
    array_push($complete_parents, '#value');
  }
  $val = drupal_array_get_nested_value($state, $complete_parents);
  if (isset($val)) {
    return $val;
  }

  $saved_value_parents = $parents;
  if (!empty($file_name)) {
    $saved_value_parents = array();
    $last = '';
    foreach ($parents as $item) {
      if ($last == $file_name) {
        $item = "$file_name-$item";
      }
      $last = $item;
      if ($item == $file_name) {
        continue;
      }
      $saved_value_parents[] = $item;
    }
  }
  array_unshift($saved_value_parents, 'saved_values', $page);
  $saved_val = drupal_array_get_nested_value($state, $saved_value_parents);
  return $saved_val ?? $default;
}

/**
 * Migrates necessary information from the old form state to the new one.
 *
 * This function is usually only called from tpps_main().
 *
 * @param array $new
 *   The new form state to be populated.
 * @param array $old
 *   The old form state with the old information.
 */
function tpps_form_state_info(&$new, &$old) {
  $new['saved_values'] = $old['saved_values'];
  $new['stage'] = $old['stage'];
  $new['accession'] = $old['accession'];
  $new['dbxref_id'] = $old['dbxref_id'];
  $new['stats'] = $old['stats'] ?? NULL;
  $new['ids'] = $old['ids'] ?? NULL;
  $new['tpps_type'] = $old['tpps_type'] ?? NULL;
  $new['file_info'] = $old['file_info'] ?? NULL;
  $new['status'] = $old['status'] ?? NULL;
  $new['updated'] = $old['updated'] ?? time();
  $new['created'] = $old['created'] ?? NULL;
  $new['approved'] = $old['approved'] ?? NULL;
  $new['completed'] = $old['completed'] ?? NULL;
  $new['loaded'] = $old['loaded'] ?? NULL;
  $new['submitting_uid'] = $old['submitting_uid'] ?? NULL;
  $new['job_id'] = $old['job_id'] ?? NULL;
  $new['revised_files'] = $old['revised_files'] ?? NULL;
  $new['admin_comments'] = $old['admin_comments'] ?? NULL;
}
