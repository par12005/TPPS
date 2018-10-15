<?php

function tpps_create_record($table, array $values){
    if (isset($values[0]) and gettype($values[0]) === "array"){
        $is_multi = TRUE;
    }
    else {
        $is_multi = FALSE;
    }
    
    $default_fields = array();
    $insert = NULL;
    
    if ($is_multi){
        $unique_columns = tpps_get_unique_columns($table);
        foreach ($values as $key => $record){
            if (($existing_key = tpps_record_exists($table, $record, $unique_columns))){
                unset($values[$key]);
            }
        }
        if ($values == array()){
            return;
        }
    }
    else{
        //check record exists
        if (($existing_key = tpps_record_exists($table, $values))){
            //if it does, do not attempt to re-write it.
            //drupal_set_message(t('Error: Record already exists.'), 'error');
            return $existing_key;
        }
    }
    
    $fields = db_query("select column_name from INFORMATION_SCHEMA.columns where table_name='$table' and table_schema='chado'");
    $set_fields = array();
    
    if ($is_multi){
        while (($field = $fields->fetchAssoc())){
            $key = $field['column_name'];
            $top = reset($values);
            if (!isset($top[$key])){
                $default_fields[] = $key;
                foreach ($values as $index => $record){
                    unset($values[$index][$key]);
                }
            }
            else {
                $set_fields[] = $key;
            }
        }
    }
    else {
        //get default values
        while (($field = $fields->fetchAssoc())){
            $key = $field['column_name'];
            if (!isset($values[$key])){
                $default_fields[] = $key;
                unset($values[$key]);
            }
            else {
                $set_fields[] = $key;
            }
        }
    }
    
    //print_r($default_fields);
//    dpm($table);
//    dpm($values);
//    dpm($default_fields);
    if ($is_multi){
        $insert = db_insert("chado.$table")
            ->useDefaults($default_fields)
            ->fields($set_fields);
        foreach ($values as $record){
            $insert->values($record);
        }
        $insert->execute();
        return;
    }
    else {
        //Insert the record
        $insert = db_insert("chado.$table")
            ->useDefaults($default_fields)
            ->fields($set_fields)
            ->values($values)
            ->execute();

        return $insert;
    }
}

function tpps_record_exists($table, array $values, $unique_columns = NULL){
    
    if (!isset($unique_columns)){
        $unique_columns = tpps_get_unique_columns($table);
        if (!isset($unique_columns)){
            return FALSE;
        }
    }
    
    $results = db_select("chado.$table", 't')
        ->fields('t');
    
    $and = db_and();
    foreach ($unique_columns as $field){
        if (isset($values[$field]) and $values[$field] != ''){
            $and->condition($field, "$values[$field]");
        }
    }
    
    $results = $results->condition($and)
        ->execute()
        ->fetchAssoc();
    
    if ($results){
        $primary = tpps_get_primary_key($table);
        return $results[$primary];
    }
    
    return FALSE;
}

function tpps_get_unique_columns($table){
    
    $result = db_query("select indexdef from pg_indexes where tablename = '$table' and schemaname = 'chado' and indexdef LIKE '%UNIQUE%' and indexname NOT LIKE '%_pkey';")
        ->fetchAssoc();
    
    if ($result){
        $result = preg_split('/(\(|\))/',$result['indexdef']);
        $result = explode(', ', $result[1]);
        return $result;
    }
    else {
        return NULL;
    }
}

function tpps_get_primary_key($table){
    
    $result = db_query("select indexdef from pg_indexes where tablename = '$table' and schemaname = 'chado' and indexdef LIKE '%UNIQUE%' and indexname LIKE '%_pkey';")
    ->fetchAssoc();
    
    if ($result){
        $result = preg_split('/(\(|\))/',$result['indexdef']);
        return $result[1];
    }
    else {
        return NULL;
    }
}
