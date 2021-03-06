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
  $matches = array();

  if (preg_match('/(([0-9]+)° ?([0-9]+)\' ?([0-9|\.]+)" ?([N|S])),? ?(([0-9]+)° ?([0-9]+)\' ?([0-9|\.]+)" ?([E|W]))/', $raw_coordinate, $matches)) {
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
