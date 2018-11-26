<?php

function tpps_get_environmental_layer_data($layer_id, $lat, $long){
    
    $response = tpps_get_env_response($layer_id, $lat, $long);
    
    if (($response = explode("\n", $response))){
        if (($data = explode("=", $response[2]))){
            return trim($data[1]);
        }
    }
    
    return NULL;
}

function tpps_get_env_response($layer_id, $lat, $long){
    $query = db_select('cartogratree_layers', 'l')
        ->fields('l', array('name'))
        ->condition('layer_id', $layer_id)
        ->execute();
    
    $result = $query->fetchObject();
    $layers = $result->name;
    
    $url = "http://treegenesdev.cam.uchc.edu:8080/geoserver/ct/wms?";
    $serv = "WMS";
    $ver = "1.3.0";
    $req = "GetFeatureInfo";
    $srs = "EPSG:4326";
    $format = "application/json";
    $bigger_lat = $lat + 0.0000001;
    $bigger_long = $long + 0.0000001;
    $bbox = "$lat,$long,$bigger_lat,$bigger_long";
    $pixels = "width=1&height=1&X=0&Y=0";
    
    $url .= "service=$serv&"
        . "version=$ver&"
        . "request=$req&"
        . "layers=$layers&"
        . "srs=$srs&"
        . "format=$format&"
        . "query_layers=$layers&"
        . "bbox=$bbox&"
        . "$pixels";
    
    return file_get_contents($url);
}