<?php

/**
 * @file
 * Defines a function to validate the columns of a managed_file element.
 */

/**
 * This function validates that the file contains all of the required groups.
 *
 * A "required group" is a column type that must be present within the file for
 * the file to be considered valid. For instance, a required group for a tree
 * accession file would be the "Location" group. The required groups array
 * should satisfy the structure:
 *  array(
 *    '<group name>' => array(
 *      '<type>' => array(<column option index>[, <column option index>...])[,
 *      '<type>' => array(<column option index>[, <column option index>...])...]
 *    )[,
 *    '<group name>' => array(
 *      '<type>' => array(<column option index>[, <column option index>...])[,
 *      '<type>' => array(<column option index>[, <column option index>...])...]
 *    )...]
 *  )
 * where <group name> is the name of the required group, <type> is the name of
 * the set of options that could satisfy the required group, and each of the
 * <column option index>s is a key within the file element['column-options']
 * field.
 *
 * @param array $form_state
 *   The state of the form that the file is part of.
 * @param array $required_groups
 *   The required column option groups for the file.
 * @param array $file_element
 *   The managed_file element of the file within the form.
 *
 * @return array
 *   The groups and the column option types that the file was valid for.
 */
function tpps_file_validate_columns(array &$form_state, array $required_groups, array $file_element) {
  $cols = $file_element['#value']['columns'];

  $parents = $file_element['#parents'];
  $new_end_columns = end($parents) . "-columns";
  $new_end_no_header = end($parents) . "-no-header";
  $new_end_empty = end($parents) . "-empty";
  $group_path = array_pop($parents) . "-groups";
  $values = &$form_state['values'];
  foreach ($parents as $item) {
    $values = &$values[$item];
  }
  // Initialize form column values in form state.
  $values[$new_end_columns] = array();
  // Hold onto the location of the columns in form state.
  $state_column_values = &$values[$new_end_columns];
  $values[$new_end_no_header] = isset($file_element['#value']['no-header']) ? $file_element['#value']['no-header'] : NULL;
  $values[$new_end_empty] = isset($file_element['#value']['empty']) ? $file_element['#value']['empty'] : NULL;

  $title_parts = explode(':', $file_element['#title']);
  $error_prompt = $title_parts[0];

  $groups = array();
  $required_groups_flat = array();
  foreach ($required_groups as $group => $combinations) {
    $groups[$group] = array();
    $required_groups_flat[$group] = array();
    foreach ($combinations as $name => $combination) {
      $required_groups_flat[$group] = array_merge($required_groups_flat[$group], $combination);
    }
  }

  //dpm($required_groups_flat);
  //dpm($cols);
  //dpm($state_column_values);
  foreach ($cols as $name => $type) {
    $state_column_values[$name] = $type;
    foreach ($required_groups_flat as $group => $types) {
      if (in_array($type, $types)) {
        if (!isset($groups[$group][$type])) {
          $groups[$group][$type] = array($name);
        }
        else {
          $groups[$group][$type][] = $name;
        }
        break;
      }
    }
  }
  dpm($groups);

  foreach ($required_groups as $group => $combinations) {
    $group_valid = FALSE;
    foreach ($combinations as $name => $combination) {
      $combination_valid = TRUE;
      foreach ($combination as $type) {
        if (!isset($groups[$group][$type])) {
          $combination_valid = FALSE;
          break;
        }
      }
      if ($combination_valid) {
        $groups[$group]['#type'] = $name;
        $group_valid = TRUE;
        break;
      }
    }

    if (!$group_valid) {
      form_set_error($file_element['#name'] . "[columns][$group", "$error_prompt: Please specify a column or columns that hold $group.");
    }
  }

  foreach ($groups as $key => $group) {
    foreach ($group as $opt_num => $col_names) {
      if ($opt_num != '#type' and count($col_names) == 1) {
        $groups[$key][$opt_num] = $col_names[0];
      }
    }
  }

  $values[$group_path] = $groups;

  return $groups;
}
