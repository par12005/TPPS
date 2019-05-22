<?php

/**
 * @file
 */

/**
 *
 */
function tpps_content_no_header(&$content) {
  $top_row = $content['headers'];
  $headers = array();
  for ($i = 0; $i < count($content['headers']); $i++) {
    $headers[$i] = $i;
  }
  $other_rows = array_slice($content, 1);

  foreach ($other_rows as $row_num => $row) {
    $new_row = array();
    for ($i = 0; $i < count($row); $i++) {
      $new_row[$i] = $row[$top_row[$i]];
    }
    $other_rows[$row_num] = $new_row;
  }
  array_unshift($other_rows, $top_row);

  $content = $other_rows;
  $content['headers'] = $headers;
}
