<?php

/**
 * @file
 */

/**
 * 
 */
function tpps_dynamic_list(&$form, &$form_state, $id, $repeat, $options = array()) {
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
 * 
 */
function tpps_get_ajax_number(&$state, $parents, $up, $down, $default = 0, $minimum = 0) {
  // Get page number.
  $page = $state['stage'];

  // Check form_state values, update if necessary.
  $value_parents = $parents;
  array_unshift($value_parents, 'values');
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
 * 
 */
function tpps_get_ajax_value(&$state, $parents, $default = NULL, $file_name = "") {
  $page = $state['stage'];
  $value_parents = $parents;
  array_unshift($value_parents, 'complete form');
  $element = drupal_array_get_nested_value($state, $value_parents);
  if ($element['#type'] != 'fieldset') {
    array_push($value_parents, '#value');
  }
  $val = drupal_array_get_nested_value($state, $value_parents);
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
