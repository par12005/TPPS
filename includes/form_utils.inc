<?php
/**
 * 
 */

function tpps_textfield_validate(&$form, &$form_state, $conditional = NULL) {
  if (empty($conditional)) {
    $conditional = function ($element) {
      return $element['#type'] == 'textfield';
    };
  }

  $fields = tpps_form_traverse($form, $conditional);
  
  foreach ($fields as $field) {
    if (!empty($field['#title']) and substr($field['#title'], -1) == '*') {
      $val = drupal_array_get_nested_value($form_state['values'], $field['#parents']);
      if (!$val) {
        $path = implode('][', $field['#parents']);
        $name = substr($field['#title'], 0, -3);
        form_set_error($path, "$name: field is required.");
      }
    }
  }
}

function tpps_form_traverse($element, $conditional) {
  $return = array();
  if (!is_array($element) or !array_key_exists('#type', $element)) {
    return $return;
  }

  if ($conditional($element)) {
    $return[] = $element;
  }

  foreach ($element as $key => $val) {
    $return = array_merge($return, tpps_form_traverse($val, $conditional));
  }

  return $return;
}
