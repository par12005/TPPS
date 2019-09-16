<?php
/**
 * @file
 * Defines useful functions for form validation, submission, etc.
 */

/**
 * Checks fields returned by tpps_form_traverse to see if they are empty.
 *
 * By default, the $conditional function used for tpps_form_traverse() returns
 * fields which match certain types.
 *
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 * @param mixed $conditional
 *   Optional - the function returning the fields to be checked.
 */
function tpps_empty_validate(&$form, &$form_state, $func = NULL) {
  $default_types = array(
    'textfield',
    'textarea',
    'select',
  );

  if (empty($func)) {
    $func = function ($element) {
      return TRUE;
    };
  }

  $conditional = function ($element) use ($default_types, $func) {
    $type_match = array_search($element['#type'], $default_types) !== FALSE;
    return $type_match and $func($element);
  };

  $fields = tpps_form_traverse($form, $conditional);
  
  foreach ($fields as $field) {
    if (!empty($field['#title'])) {
      $val = drupal_array_get_nested_value($form_state['values'], $field['#parents']);
      if (tpps_empty_value($val, $field['#type'])) {
        dpm($field);
        $path = implode('][', $field['#parents']);
        $name = trim($field['#title'], " :*");
        form_set_error($path, "$name: field is required.");
      }
    }
  }
}

function tpps_empty_value($val, $type) {
  switch ($type) {
    case 'textfield':
    case 'textarea':
      return !((bool) strlen($val));
    case 'select':
    case 'checkbox':
      return !$val;
    default:
      return FALSE;
  }
}

/**
 * Returns the elements of a form which satisfy a function returning a bool.
 *
 * @param mixed $element
 *   The form or element currently being traversed.
 * @param Closure $conditional
 *   The function the element will be tested against.
 *
 * @return array
 *   An array of form elements which satisfied $conditional.
 */
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
