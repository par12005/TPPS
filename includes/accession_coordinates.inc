<?php

/**
 * @file
 * Defines Ajax callback for plant accession coordinates.
 */

/**
 * Returns an array of standardized coordinates based on a file id number.
 */
function tpps_accession_coordinates() {
  $params = drupal_get_query_parameters($_POST);

  if (!empty($params['fid']) and file_load($params['fid'])) {
    $coordinates = array();
    $columns = array(
      'id_col' => $params['id_col'],
      'lat_col' => $params['lat_col'],
      'long_col' => $params['long_col'],
    );

    $options = array(
      'no_header' => $params['no-header'],
      'columns' => $columns,
      'coords' => &$coordinates,
    );
    tpps_file_iterator($params['fid'], 'tpps_get_coordinates', $options);

    drupal_json_output($coordinates);
  }
}

/**
 * This function processes a single row of a plant accession file.
 *
 * This function populates the coords attribute of the options array with the
 * standardized coordinates of each plant in the file. This function is meant to
 * be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_get_coordinates($row, &$options) {
  $id_col = $options['columns']['id_col'];
  $lat_col = $options['columns']['lat_col'];
  $long_col = $options['columns']['long_col'];
  $coordinates = &$options['coords'];

  // Only add coordinates to the standardized coordinates array if
  // we can successfully standardsize them.
  if (($coord = tpps_standard_coord("{$row[$lat_col]},{$row[$long_col]}"))) {
    $pair = explode(',', $coord);
    // Add the standardized coordinates.
    array_push($coordinates, array(
      "{$row[$id_col]}",
      $pair[0],
      $pair[1],
    ));
  }
}
