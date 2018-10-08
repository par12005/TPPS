<?php

function tpps_compare_files($fid_1, $fid_2, $file_1_id_name, $file_2_id_name){
    
    $file_1 = file_load($fid_1);
    $file_2 = file_load($fid_2);
    
    $file_1_name = $file_1->uri;
    $file_2_name = $file_2->uri;
    
    $file_1_location = drupal_realpath("$file_1_name");
    $file_2_location = drupal_realpath("$file_2_name");
    $file_1_content = tpps_parse_xlsx($file_1_location);
    $file_2_content = tpps_parse_xlsx($file_2_location);
    
    if (gettype($file_1_id_name) == 'integer'){
        $file_1_id_name = $file_1_content['headers'][$file_1_id_name];
        $last_row = array();
        foreach($file_1_content['headers'] as $num => $val){
            $last_row[$val] = $val;
        }
        $file_1_content[] = $last_row;
    }
    if (gettype($file_2_id_name) == 'integer'){
        $file_2_id_name = $file_2_content['headers'][$file_2_id_name];
        $last_row = array();
        foreach($file_2_content['headers'] as $num => $val){
            $last_row[$val] = $val;
        }
        $file_2_content[] = $last_row;
    }
    
    $missing_ids = array();
    
    for ($i = 0; $i < count($file_1_content) - 1; $i++){
        $row = $file_1_content[$i];
        
        $needle_id = $row[$file_1_id_name];
        $found = FALSE;
        
        for ($j = 0; $j < count($file_2_content) - 1; $j++){
            if ($needle_id === $file_2_content[$j][$file_2_id_name]){
                $found = TRUE;
                break;
            }
        }
        
        if (!$found){
            array_push($missing_ids, $needle_id);
        }
    }
    
    return $missing_ids;
}
