<?php

/**
 * @file
 * Defines functions to load environmental data from CartograTree layers.
 */

/**
 * This function parses and returns a data point from a CartograTree layer.
 *
 * The data point for the layer at the specified location is obtained by calling
 * tpps_get_env_response, and the resulting response string is parsed to return
 * the specified parameter.
 *
 * @param int $layer_id
 *   The identifier of the CartograTree environmental layer.
 * @param float $lat
 *   The latitude coordinate being queried.
 * @param float $long
 *   The longitude coordinate being queried.
 * @param string $param
 *   The name of the parameter type.
 *
 * @return mixed
 *   The parsed environmental data. If no valid data was found, return NULL.
 */
function tpps_get_environmental_layer_data($layer_id, $lat, $long, $param) {

  $response = tpps_get_env_response($layer_id, $lat, $long);
  if (($response = explode("\n", $response))) {
    $response = array_slice($response, 2, -2);
    foreach ($response as $line) {
      if (($item = explode("=", $line)) and trim($item[0]) == $param) {
        return trim($item[1]);
      }
    }
  }
  return NULL;
}

/**
 * This function loads data for a CartograTree layer at a lat/long coordinate.
 *
 * @param int $layer_id
 *   The identifier of the CartograTree environmental layer.
 * @param float $lat
 *   The latitude coordinate being queried.
 * @param float $long
 *   The longitude coordinate being queried.
 *
 * @return string
 *   The environmental data for that layer at that lat/long coordinate.
 */
function tpps_get_env_response($layer_id, $lat, $long) {
  if (db_table_exists('cartogratree_layers')){
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

    $url .= "service=$serv&version=$ver&request=$req&layers=$layers&srs=$srs&format=$format&query_layers=$layers&bbox=$bbox&$pixels";

    return file_get_contents($url);
  }
}
