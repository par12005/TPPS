<?php

function tpps_file_validate_columns(&$form_state, $required_groups, $file_element){
    $cols = $file_element['#value']['columns'];
    
    $parents = $file_element['#parents'];
    $new_end_columns = end($parents) . "-columns";
    $new_end_no_header = end($parents) . "-no-header";
    $new_end_empty = end($parents) . "-empty";
    $group_path = array_pop($parents) . "-groups";
    $values = &$form_state['values'];
    foreach ($parents as $item){
        $values = &$values[$item];
    }
    //initialize form column values in form state
    $values[$new_end_columns] = array();
    //hold onto the location of the columns in form state
    $state_column_values = &$values[$new_end_columns];
    $values[$new_end_no_header] = isset($file_element['#value']['no-header']) ? $file_element['#value']['no-header'] : NULL;
    $values[$new_end_empty] = isset($file_element['#value']['empty']) ? $file_element['#value']['empty'] : NULL;
    
    $title_parts = explode(':', $file_element['#title']);
    $error_prompt = $title_parts[0];
    
    $groups = array();
    $required_groups_flat = array();
    foreach($required_groups as $group => $combinations){
        $groups[$group] = array();
        $required_groups_flat[$group] = array();
        foreach ($combinations as $name => $combination){
            $required_groups_flat[$group] = array_merge($required_groups_flat[$group], $combination);
        }
    }
    
    foreach ($cols as $name => $type){
        $state_column_values[$name] = $type;
        foreach ($required_groups_flat as $group => $types){
            if (in_array($type, $types)){
                if (!isset($groups[$group][$type])){
                    $groups[$group][$type] = array($name);
                }
                else {
                    $groups[$group][$type][] = $name;
                }
                break;
            }
        }
    }
    
    foreach($required_groups as $group => $combinations){
        $group_valid = FALSE;
        foreach ($combinations as $name => $combination){
            $combination_valid = TRUE;
            foreach ($combination as $type){
                if (!isset($groups[$group][$type])){
                    $combination_valid = FALSE;
                    break;
                }
            }
            if ($combination_valid){
                $groups[$group]['#type'] = $name;
                $group_valid = TRUE;
                break;
            }
        }
        
        if (!$group_valid){
            form_set_error($file_element['#name'] . "[columns][$group", "$error_prompt: Please specify a column or columns that hold $group.");
        }
    }
    
    foreach ($groups as $key => $group){
        foreach ($group as $opt_num => $col_names){
            if ($opt_num != '#type' and count($col_names) == 1){
                $groups[$key][$opt_num] = $col_names[0];
            }
        }
    }
    
    $values[$group_path] = $groups;
    
    return $groups;
}
