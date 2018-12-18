<?php

function tpps_accession_coordinates(){
    $params = drupal_get_query_parameters($_POST);
    
    if (!empty($params['fid'])){
        if (($file_name = file_load($params['fid'])->uri)){
            $id_col = $params['id_col'];
            $lat_col = $params['lat_col'];
            $long_col = $params['long_col'];
            $location = drupal_realpath($file_name);
            $content = gttn_tpps_parse_xlsx($location);

            if ($params['no-header']){
                gttn_tpps_content_no_header($content);
            }
            
            $coordinates = array();
            for ($i = 0; $i < count($content) - 1; $i++){
                // Only add coordinates to the standardized coordinates array if
                // we can successfully standardsize them.
                if (($coord = gttn_tpps_standard_coord("{$content[$i][$lat_col]},{$content[$i][$long_col]}"))){
                    $pair = explode(',', $coord);
                    // Add the standardized coordinates.
                    array_push($coordinates, array("{$content[$i][$id_col]}", $pair[0], $pair[1]));
                }
            }

            drupal_json_output($coordinates);
        }
    }
}