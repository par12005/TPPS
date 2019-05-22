<?php

function tpps_chado_insert_record($table, $records, $options = array()){
    //TODO: multi-record optimization
    if (!array_key_exists('multi', $options)){
        $options['multi'] = FALSE;
    }
    
    if (!$options['multi']){
        $records = array($records);
    }
    else {
        $has_fields = FALSE;
        $insert = db_insert("chado.$table");
        $fields_arr = array();
        foreach ($records[0] as $field => $val){
            $fields_arr[] = $field;
        }
        $insert->fields($fields_arr);
    }
    
    $table_desc = chado_get_schema($table);
    if (!$table_desc)
        return FALSE;
    $primary = !empty($table_desc['primary key'][0]) ? $table_desc['primary key'][0] : NULL;
    foreach($records as $values){
        // Populate insert_values array and check if a record with the same unique 
        // fields already exists. If it does, return the existing primary key.
        $insert_values = array();

        foreach ($values as $field => $value) {
            if (is_array($value)) {
                $results = chado_schema_get_foreign_key($table_desc, $field, $value);
                if (sizeof($results) != 1)
                    return FALSE;
                else 
                    $insert_values[$field] = $results[0];
            }
            else {
                $insert_values[$field] = $value;
            }
        }

        $unique = array();
        if (array_key_exists('unique keys', $table_desc))
            $unique = $table_desc['unique keys'];
        $u_cols = array($primary);
        $u_vals = array();

        $exists = FALSE;
        foreach ($unique as $name => $fields) {
            foreach ($fields as $index => $field) {
                $u_cols[] = $field;
                if (!array_key_exists($field, $insert_values)){
                    if (array_key_exists('default', $table_desc['fields'][$field]))
                        $u_vals[$field] = $table_desc['fields'][$field]['default'];
                }
                else 
                    $u_vals[$field] = $insert_values[$field];
            }
            
            if (($results = chado_select_record($table, $u_cols, $u_vals))) {
                $exists = TRUE;
                // If the new values match with exactly one record, update that one
                // and return the primary key.
                if (!$options['multi'] and sizeof($results == 1)){
                    chado_update_record($table, $u_vals, $insert_values);
                    return $results[0]->{$primary};
                }
            }
        }
        
        if (!$exists and $options['multi']){
            $insert->values($insert_values);
            $has_fields = TRUE;
        }
        
        if (!$options['multi']){
            if ($exists)
                return $results[0]->{$primary};
            $new_record = chado_insert_record($table, $insert_values, $options);
            return $new_record[$primary];
        }
    }
    if ($options['multi'] and $has_fields){
        $insert->execute();
    }
}
