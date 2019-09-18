<?php

/**
 * @file
 * Defines Ajax callback for tree accession coordinates.
 */

/**
 * Returns an array of standardized coordinates based on a file id number.
 */
function tpps_accession_coordinates() {
  $params = drupal_get_query_parameters($_POST);

  if (!empty($params['fid'])) {
    if (file_load($params['fid'])) {
      $id_col = $params['id_col'];
      $lat_col = $params['lat_col'];
      $long_col = $params['long_col'];
      $content = tpps_parse_file($params['fid'], 0, $params['no-header']);

      $coordinates = array();
      for ($i = 0; $i < count($content) - 1; $i++) {
        // Only add coordinates to the standardized coordinates array if
        // we can successfully standardsize them.
        if (($coord = tpps_standard_coord("{$content[$i][$lat_col]},{$content[$i][$long_col]}"))) {
          $pair = explode(',', $coord);
          // Add the standardized coordinates.
          array_push($coordinates, array("{$content[$i][$id_col]}",
            $pair[0],
            $pair[1],
          ));
        }
      }

      drupal_json_output($coordinates);
    }
  }
}
