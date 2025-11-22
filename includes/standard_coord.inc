<?php

/**
 * @file
 * Defines function to standardize and validate coordinates.
 */

/**
 * This function returns a standardized set of coordinates from raw coordinates.
 *
 * If the raw coordinates provided are invalid, the function returns false.
 *
 * @param string $raw_coordinate
 *   The non-standard coordinates to be evaluated and standardized.
 *
 * @return mixed
 *   The coordinates in standardized form. False if coordinates were not valid.
 */
function tpps_standard_coord($raw_coordinate) {
  // Patch added on 3/22/2022 - Reported by Emily Strickland
  if(stripos($raw_coordinate,'‐') !== FALSE) {
    // this is a strange character, so replace it with standard minus
    $raw_coordinate = str_ireplace('‐','-', $raw_coordinate);
  }
  

  $matches = array();
  if (preg_match('/-?([0-9|\.]+),\s?-?([0-9|\.]+)/', $raw_coordinate, $matches)) {
    // echo "GPS needs interpretation: $raw_coordinate\n";
    $lat_with_periods = $matches[1];
    $long_with_periods = $matches[2];
    $lat_parts = explode('.', $lat_with_periods);
    $long_parts = explode('.', $long_with_periods);
    $lat_parts_count = count($lat_parts);
    $long_parts_count = count($long_parts);
    // Check latitude value
    if ($lat_parts_count == 3) {
      echo "Found a 3 part latitude value from raw coordinates $raw_coordinate\n";
      $lat = intval($lat_parts[0]) + (intval($lat_parts[1]) / 60) + (intval($lat_parts[2]) / 3600);
    }
    else if ($lat_parts_count <= 2 and $lat_parts_count > 0) {
      $lat = $lat_with_periods;
    }
    else {
      echo "There's a latitude error happening for $raw_coordinate";
      return FALSE;
    }
    // Check longitude value
    if ($long_parts_count == 3) {
      echo "Found a 3 part longitude value from raw coordinates $raw_coordinate\n";
      $long = intval($long_parts[0]) + (intval($long_parts[1]) / 60) + (intval($long_parts[2]) / 3600);
    }
    else if ($long_parts_count <= 2 and $long_parts_count > 0) {
      $long = $long_with_periods;
    }
    else {
      echo "There's a longitude error happening for $raw_coordinate";
      return FALSE;
    }
    // echo "GPS Interpreted $raw_coordinate to $lat,$long\n";
    // throw new Exception('DEBUG');
  }
  elseif (preg_match('/(([0-9]+)° ?([0-9]+)\' ?([0-9|\.]+)" ?([N|S])),? ?(([0-9]+)° ?([0-9]+)\' ?([0-9|\.]+)" ?([E|W]))/', $raw_coordinate, $matches)) {
    $lat = $matches[2] + ($matches[3] / 60) + ($matches[4] / 3600);
    $long = $matches[7] + ($matches[8] / 60) + ($matches[9] / 3600);
  }
  elseif (preg_match('/(([0-9]+)° ?([0-9|\.]+)\' ?([N|S])),? ?(([0-9]+)° ?([0-9|\.]+)\' ?([E|W]))/', $raw_coordinate, $matches)) {
    $lat = $matches[2] + ($matches[3] / 60);
    $long = $matches[6] + ($matches[7] / 60);
  }
  elseif (preg_match('/([0-9|\.]+), ?[\+|-]?([0-9|\.]+)/', $raw_coordinate, $matches)) {
    $lat = $matches[1];
    $long = $matches[2];
  }
  elseif (preg_match('/([0-9|\.]+)°? ?([N|S]),? ?([0-9|\.]+)°? ?([E|W])/', $raw_coordinate, $matches)) {
    $lat = $matches[1];
    $long = $matches[3];
  }
  else {
    return FALSE;
  }

  if ($lat > 180 or $long > 180) {
    return FALSE;
  }

  if (preg_match('/(-).+,/', $raw_coordinate, $matches)) {
    $lat = $lat * -1;
  }
  if (preg_match('/,.*(-)/', $raw_coordinate, $matches)) {
    $long = $long * -1;
  }

  if (preg_match('/([N|S]).+([E|W])/', $raw_coordinate, $matches)) {
    if ($matches[1] == 'S') {
      $lat = $lat * -1;
    }
    if ($matches[2] == 'W') {
      $long = $long * -1;
    }
  }

  return "$lat,$long";
}
