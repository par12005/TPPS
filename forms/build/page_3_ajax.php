<?php

function page_3_multi_map($form, $form_state){
    
    if (!empty($form['tree-accession']['file']['#value']['fid'])){
        $file = $form_state['values']['tree-accession']['file']['fid'];
        $columns = $form_state['values']['tree-accession']['file']['columns'];
        
        foreach ($columns as $key => $val){
            if ($val == '1'){
                $id_col = $key;
            }
            if ($val == '4'){
                $lat_col = $key;
            }
            if ($val == '5'){
                $long_col = $key;
            }
        }
        
        if (!isset($lat_col) or !isset($long_col) or !isset($id_col)){
            $commands[] = ajax_command_invoke('#map_wrapper', 'hide');
            return array('#type' => 'ajax', '#commands' => $commands);
        }
        
        $standards = array();
        
        if (($file = file_load($file))){
            $file_name = $file->uri;

            $location = drupal_realpath("$file_name");
            $content = tpps_parse_xlsx($location);

            if (isset($form_state['values']['tree-accession']['no-header']) and $form_state['values']['tree-accession']['no-header'] == 1){
                tpps_content_no_header($content);
            }

            for ($i = 0; $i < count($content) - 1; $i++){
                if (($coord = tpps_standard_coord("{$content[$i][$lat_col]},{$content[$i][$long_col]}"))){
                    $pair = explode(',', $coord);
                    array_push($standards, array("{$content[$i][$id_col]}", $pair[0], $pair[1]));
                }
            }
            
        }
        
        $commands[] = ajax_command_invoke('#map_wrapper', 'updateMap', array($standards));
        
        return array('#type' => 'ajax', '#commands' => $commands);
    }
    else {
        $commands[] = ajax_command_invoke('#map_wrapper', 'hide');
        return array('#type' => 'ajax', '#commands' => $commands);
    }
}
